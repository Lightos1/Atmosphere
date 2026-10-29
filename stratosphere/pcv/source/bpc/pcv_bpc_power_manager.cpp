/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stratosphere.hpp>
#include "pcv_bpc_power_manager.hpp"
#include "pcv_bpc_irq_device.hpp"
#include "pcv_bpc_pmic_registers.hpp"
#include "../rtc/pcv_rtc.hpp"

namespace ams::pcv::bpc {

    namespace {

        constexpr os::InterruptName PmicInterruptName = 118;

    }

    void PowerManager::Initialize(IrqDevice *irq_device) {
        m_irq_device = irq_device;

        /* Initialize the events. */
        os::InitializeEvent(std::addressof(m_ac_ok_event),          false,           os::EventClearMode_ManualClear);
        os::InitializeEvent(std::addressof(m_power_button_event),   false,           os::EventClearMode_ManualClear);
        os::InitializeEvent(std::addressof(m_resume_recheck_event), false,           os::EventClearMode_ManualClear);
        os::InitializeInterruptEvent(std::addressof(m_irq_event), PmicInterruptName, os::EventClearMode_ManualClear);
        os::InitializeTimerEvent(std::addressof(m_sleep_timer_event),                os::EventClearMode_ManualClear);

        /* Create system events. */
        R_ABORT_UNLESS(os::CreateSystemEvent(std::addressof(m_ac_ok_system_event),        os::EventClearMode_ManualClear, true));
        R_ABORT_UNLESS(os::CreateSystemEvent(std::addressof(m_power_button_system_event), os::EventClearMode_ManualClear, true));

        /* Initialize the multi wait. */
        os::InitializeMultiWait(std::addressof(m_multi_wait));

        /* Initialize the multi wait holders. */
        os::InitializeMultiWaitHolder(std::addressof(m_irq_holder),            std::addressof(m_irq_event));
        os::InitializeMultiWaitHolder(std::addressof(m_ac_ok_holder),          std::addressof(m_ac_ok_event));
        os::InitializeMultiWaitHolder(std::addressof(m_power_button_holder),   std::addressof(m_power_button_event));
        os::InitializeMultiWaitHolder(std::addressof(m_sleep_timer_holder),    std::addressof(m_sleep_timer_event));
        os::InitializeMultiWaitHolder(std::addressof(m_resume_recheck_holder), std::addressof(m_resume_recheck_event));

        /* Set the multi wait holder user data. */
        os::SetMultiWaitHolderUserData(std::addressof(m_irq_holder),            Source_Irq);
        os::SetMultiWaitHolderUserData(std::addressof(m_ac_ok_holder),          Source_AcOk);
        os::SetMultiWaitHolderUserData(std::addressof(m_power_button_holder),   Source_PowerButton);
        os::SetMultiWaitHolderUserData(std::addressof(m_sleep_timer_holder),    Source_SleepTimer);
        os::SetMultiWaitHolderUserData(std::addressof(m_resume_recheck_holder), Source_ResumeRecheck);

        /* Link the wait holders. */
        os::LinkMultiWaitHolder(std::addressof(m_multi_wait), std::addressof(m_irq_holder));
        os::LinkMultiWaitHolder(std::addressof(m_multi_wait), std::addressof(m_ac_ok_holder));
        os::LinkMultiWaitHolder(std::addressof(m_multi_wait), std::addressof(m_power_button_holder));
        os::LinkMultiWaitHolder(std::addressof(m_multi_wait), std::addressof(m_sleep_timer_holder));
        os::LinkMultiWaitHolder(std::addressof(m_multi_wait), std::addressof(m_resume_recheck_holder));
    }

    void PowerManager::Start() {
        /* Start the thread. */
        R_ABORT_UNLESS(os::CreateThread(std::addressof(m_thread), ThreadEntry, this, m_thread_stack, sizeof(m_thread_stack), AMS_GET_SYSTEM_THREAD_PRIORITY(pcv, BpcPowerManager)));
        os::SetThreadNamePointer(std::addressof(m_thread), AMS_GET_SYSTEM_THREAD_NAME(pcv, BpcPowerManager));
        os::StartThread(std::addressof(m_thread));
    }

    void PowerManager::ThreadFunc() {
        while (true) {
            os::MultiWaitHolderType *holder = os::WaitAny(std::addressof(m_multi_wait));

            switch (os::GetMultiWaitHolderUserData(holder)) {
                case Source_Irq:           OnIrq();           break;
                case Source_AcOk:          OnAcOk();          break;
                case Source_PowerButton:   OnPowerButton();   break;
                case Source_SleepTimer:    OnSleepTimer();    break;
                case Source_ResumeRecheck: OnResumeRecheck(); break;
                AMS_UNREACHABLE_DEFAULT_CASE();
            }
        }
    }

    void PowerManager::StartSleepTimer() {
        os::StartOneShotTimerEvent(std::addressof(m_sleep_timer_event), TimeSpan::FromSeconds(5));
    }

    Result PowerManager::SetWakeAlarm(bool enable, bool force) {
        /* Is the wake alarm already enabled? */
        if (!force && m_wake_alarm_enabled == enable) {
            return ResultSuccess();
        }

        if (!enable) {
            /* Disable alarm 1. */
            R_TRY(rtc::GetRtcDevice().DisableAlarm(1));

            m_wake_alarm_enabled = enable;
            return ResultSuccess();
        }

        /* We're enabling the alarm. */
        /* Get the current time. */
        u64 current_time_s = 0;
        R_TRY(rtc::GetRtcDevice().GetTimeInSeconds(std::addressof(current_time_s)));

        /* Enable the alarm to go off in 15 seconds. */
        u64 alarm_time = current_time_s + 15;
        R_TRY(rtc::GetRtcDevice().EnableAlarm(1, alarm_time));

        m_wake_alarm_enabled = enable;
        return ResultSuccess();
    }

    void PowerManager::StopSleepTimerAndDisarmWakeAlarm() {
        os::StopTimerEvent(std::addressof(m_sleep_timer_event));
        os::ClearTimerEvent(std::addressof(m_sleep_timer_event));

        m_long_press_alarm_set = false;

        /* Stop the alarm. */
        R_ABORT_UNLESS(SetWakeAlarm(false, false));
    }

    void PowerManager::OnSleepTimer() {
        os::ClearTimerEvent(std::addressof(m_sleep_timer_event));

        bool pressed = false;
        R_ABORT_UNLESS(GetPowerButton(std::addressof(pressed)));

        /* Still held? */
        if (pressed && !os::TryWaitEvent(std::addressof(m_power_button_event)) && !m_long_press_alarm_set) {
            m_long_press_alarm_set = true;
            /* Set the alarm. */
            R_ABORT_UNLESS(SetWakeAlarm(true, false));
        }
    }

    void PowerManager::OnIrq() {
        std::scoped_lock lk(m_irq_mutex);

        /* Clear the interrupt event. */
        os::ClearInterruptEvent(std::addressof(m_irq_event));

        /* Handle the interrupt, unless we're asleep. */
        /* Nintendo doesn't this check, they unlink the irq waiter in PrepareSleep and relink it in Resume. */
        /* But this is too much work :P */
        /* Note: Nintendo ignores the return value. */
        if (!m_irq_gated) {
            R_DISCARD(m_irq_device->HandleInterrupt());
        } else {
            m_irq_pending_while_gated = true;
        }
    }

    void PowerManager::OnAcOk() {
        /* CLear the event. */
        os::ClearEvent(std::addressof(m_ac_ok_event));

        bool ac_ok = false;
        R_ABORT_UNLESS(GetAcOk(std::addressof(ac_ok)));

        /* Signal the system event */
        if (ac_ok && !m_prev_ac_ok) {
            os::SignalSystemEvent(std::addressof(m_ac_ok_system_event));
        }

        m_prev_ac_ok = ac_ok;
    }

    void PowerManager::OnPowerButton() {
        /* CLear the event. */
        os::ClearEvent(std::addressof(m_power_button_event));

        /* Get the current state. */
        bool pressed = false;
        R_ABORT_UNLESS(GetPowerButton(std::addressof(pressed)));

        /* Signal the system event if the state changed (press or release). */
        if (HasPowerButtonChanged(pressed)) {
            os::SignalSystemEvent(std::addressof(m_power_button_system_event));
        }

        m_prev_pressed = pressed;

        /* The button was pressed, start the 5s timer. */
        if (m_prev_pressed) {
            StartSleepTimer();
        } else {
            /* The button was released, stop the timer. */
            StopSleepTimerAndDisarmWakeAlarm();
        }
    }

    void PowerManager::OnResumeRecheck() {
        /* Clear the event. */
        os::ClearEvent(std::addressof(m_resume_recheck_event));

        /* Resync the power button state after sleep. */
        OnPowerButton();
    }

    void PowerManager::PrepareSleep() {
        {
            std::scoped_lock lk(m_irq_mutex);
            m_irq_gated = true;
        }

        /* Save and invalidate the on off callback. */
        m_saved_on_off_callback = m_irq_device->GetOnOffCallback();
        R_ABORT_UNLESS(m_irq_device->SetOnOffCallback(nullptr));

        i2c::I2cSession &pmic_session = m_irq_device->GetPmicSession();

        /* Save the irq top mask. */
        u8 irq_top_mask = 0;
        R_ABORT_UNLESS(I2cRead(pmic_session, Max77620PmicIrqTopMReg, irq_top_mask));
        m_saved_irq_top_mask = irq_top_mask;

        /* Set the sleep masks. */
        u8 sleep_mask = Max77620PmicIrqTopSleepMask;
        R_ABORT_UNLESS(I2cWrite(pmic_session, Max77620PmicIrqTopMReg, sleep_mask));

        /* Read the irq top register. */
        {
            u8 irq_top_register = 0;
            R_ABORT_UNLESS(I2cRead(pmic_session, Max77620PmicIrqTopReg, irq_top_register));
        }

        /* Save the on off irq mask. */
        u8 on_off_irq_mask = 0;
        R_ABORT_UNLESS(I2cRead(pmic_session, Max77620PmicOnOffIrqMReg, on_off_irq_mask));
        m_saved_onoff_irq_mask = on_off_irq_mask;

        /* Set the on off irq sleep mask. */
        u8 on_off_irq_sleep_mask = Max77620PmicOnOffIrqSleepMask;
        R_ABORT_UNLESS(I2cWrite(pmic_session, Max77620PmicOnOffIrqMReg, on_off_irq_sleep_mask));

        /* Set the wakeup reason to zero since we're going to sleep. */
        m_wakeup_reason = 0;
    }

    void PowerManager::ReadWakeupReason() {
        i2c::I2cSession &pmic_session = m_irq_device->GetPmicSession();

        /* Read top irq register. */
        u8 irq_top = 0;
        R_ABORT_UNLESS(I2cRead(pmic_session, Max77620PmicIrqTopReg, irq_top));

        /* Restore its saved mask. */
        u8 irq_top_mask = 0;
        R_ABORT_UNLESS(I2cRead(pmic_session,  Max77620PmicIrqTopMReg, irq_top_mask));
        R_ABORT_UNLESS(I2cWrite(pmic_session, Max77620PmicIrqTopMReg, m_saved_irq_top_mask));
        irq_top &= ~irq_top_mask;

        /* Read the on off irq register. */
        u8 on_off_irq = 0;
        R_ABORT_UNLESS(I2cRead(pmic_session, Max77620PmicOnOffIrqReg, on_off_irq));

        /* Restore its saved mask.. */
        u8 on_off_irq_mask = 0;
        R_ABORT_UNLESS(I2cRead(pmic_session,  Max77620PmicOnOffIrqMReg, on_off_irq_mask));
        R_ABORT_UNLESS(I2cWrite(pmic_session, Max77620PmicOnOffIrqMReg, m_saved_onoff_irq_mask));

        const u8 on_off_pending = on_off_irq & ~on_off_irq_mask;

        /* Set the wakeup reason. */
        u8 wakeup_reason = 0;

        if (on_off_pending & Max77620PmicOnOffIrqEn0RiseMask) {
            wakeup_reason |= Max77620PmicWakeupReasonEn0RiseMask;
        }

        if (on_off_pending & Max77620PmicOnOffIrqAcOkFallMask) {
            wakeup_reason |= Max77620PmicWakeupReasonAcOkFallMask;
        }

        if (on_off_pending & Max77620PmicOnOffIrqAcOkRiseMask) {
            wakeup_reason |= Max77620PmicWakeupReasonAcOkRiseMask;
        }

        if (irq_top & Max77620PmicIrqTopRtcMask) {
            wakeup_reason |= Max77620PmicWakeupReasonRtcMask;
        }

        m_wakeup_reason = wakeup_reason;
    }

    void PowerManager::Resume() {
        std::scoped_lock lk(m_irq_mutex);

        /* Read the wakeup reason. */
        /* NOTE: Nintendo checks the result even though this function always aborts on failure. */
        ReadWakeupReason();

        /* Restore the callback. */
        R_ABORT_UNLESS(m_irq_device->SetOnOffCallback(m_saved_on_off_callback));

        /* We're no longer gated. */
        m_irq_gated = false;

        /* Handle any pending requests that occured while being gated. */
        if (m_irq_pending_while_gated) {
            m_irq_pending_while_gated = false;
            R_DISCARD(m_irq_device->HandleInterrupt());
        }

        /* Ask the power thread to recheck the power button. */
        os::SignalEvent(std::addressof(m_resume_recheck_event));
    }

    void PowerManager::OnOffCallback(u32 status) {
        if (status & Max77620PmicOnOffIrqAcOkRiseMask) {
            os::SignalEvent(std::addressof(m_ac_ok_event));
        }

        if (status & Max77620PmicOnOffIrqPowerButtonMask) {
            os::SignalEvent(std::addressof(m_power_button_event));
        }
    }

    Result PowerManager::GetAcOk(bool *out_ok) {
        R_UNLESS(out_ok != nullptr, ResultInvalidArgument());

        u8 on_off_state = 0;
        R_ABORT_UNLESS(I2cRead(m_irq_device->GetPmicSession(), Max77620PmicOnOffStatReg, on_off_state));

        *out_ok = (on_off_state & Max77620PmicOnOffStatAcOkMask) != 0;

        return ResultSuccess();
    }

    Result PowerManager::GetPowerButton(bool *out_is_pushed) {
        R_UNLESS(out_is_pushed != nullptr, ResultInvalidArgument());

        u8 on_off_state = 0;
        R_ABORT_UNLESS(I2cRead(m_irq_device->GetPmicSession(), Max77620PmicOnOffStatReg, on_off_state));

        *out_is_pushed = (on_off_state & Max77620PmicOnOffStatEn0Mask) != 0;

        return ResultSuccess();
    }

}
