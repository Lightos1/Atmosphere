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

#pragma once
#include <stratosphere.hpp>

namespace ams::pcv::bpc {

    class IrqDevice;

    class PowerManager {
        private:
            enum Source : uintptr_t {
                Source_Irq,
                Source_AcOk,
                Source_PowerButton,
                Source_SleepTimer,
                Source_ResumeRecheck,
            };
        private:
            IrqDevice *m_irq_device = nullptr;

            os::EventType           m_ac_ok_event{};
            os::SystemEventType     m_ac_ok_system_event{};

            os::EventType           m_power_button_event{};
            os::SystemEventType     m_power_button_system_event{};

            os::TimerEventType      m_sleep_timer_event{};
            os::MultiWaitHolderType m_sleep_timer_holder{};

            os::EventType           m_resume_recheck_event{};
            os::MultiWaitHolderType m_resume_recheck_holder{};

            os::InterruptEventType  m_irq_event{};

            os::MultiWaitType       m_multi_wait{};
            os::MultiWaitHolderType m_irq_holder{}, m_ac_ok_holder{}, m_power_button_holder{};
        private:
            os::ThreadType          m_thread{};
            alignas(os::ThreadStackAlignment) u8 m_thread_stack[0x1000]{};

            os::SdkMutex            m_irq_mutex;
            bool                    m_irq_gated                   = false;
            bool                    m_irq_pending_while_gated     = false;
            bool                    m_prev_ac_ok                  = false;
            bool                    m_prev_pressed                = false;
            bool                    m_long_press_alarm_set        = false;
            bool                    m_wake_alarm_enabled          = false;
            bool                    m_setup_rtc_reset_on_shutdown = false;

            u8                      m_saved_irq_top_mask      = 0;
            u8                      m_saved_onoff_irq_mask    = 0;
            u8                      m_wakeup_reason           = 0;
            u8                      m_shutdown_reason         = 0;

            void                  (*m_saved_on_off_callback)(u32) = nullptr;
        private:
            ALWAYS_INLINE bool HasPowerButtonChanged(bool pressed) const { return pressed != m_prev_pressed; }
        private:
            static void ThreadEntry(void *arg) { static_cast<PowerManager *>(arg)->ThreadFunc(); }
            void ThreadFunc();

            Result SetWakeAlarm(bool enable, bool force);
            void StartSleepTimer();
            void StopSleepTimerAndDisarmWakeAlarm();

            void OnIrq();
            void OnAcOk();
            void OnPowerButton();
            void OnSleepTimer();
            void OnResumeRecheck();
            void ReadWakeupReason();
        public:
            void Initialize(IrqDevice *irq_device);
            void Start();

            void PrepareSleep();
            void Resume();
            void OnOffCallback(u32 status);
        public:
            ALWAYS_INLINE void SetShutdownReason(u8 reason) { m_shutdown_reason = reason; }
        private:
            NORETURN void ShutdownOrRebootImpl(bool reboot);
            NORETURN void ShutdownOrReboot(bool reboot);
            void UpdateFuelGaugeForShutdown();
        public:
            NORETURN void ShutdownSystem();
            NORETURN void RebootSystem();
            void GetWakeupReason(u32 *out_wakeup_reason);
            void GetShutdownReason(u32 *out_shutdown_reason);
            Result GetAcOk(bool *out_ok);
            Result GetPowerEvent(os::SystemEventType **out_event, u32 target);
            Result GetPowerButtonEvent(os::SystemEventType **out_event, u32 target);
            Result GetPowerButton(bool *out_is_pushed);
            void SetUpRtcResetOnShutdown();
    };

}
