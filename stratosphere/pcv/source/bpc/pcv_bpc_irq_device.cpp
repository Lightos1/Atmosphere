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
#include "pcv_bpc_irq_device.hpp"
#include "pcv_bpc_pmic_registers.hpp"

#include "../rtc/pcv_rtc.hpp"
#include "../rtc/pcv_rtc_registers.hpp"

namespace ams::pcv::bpc {

    void IrqDevice::UnmaskTopInterrupt(u8 mask) {
        u8 irq_top_register_mask = 0;
        if (R_SUCCEEDED(I2cRead(m_pmic_session, Max77620PmicIrqTopMReg, irq_top_register_mask))) {
            irq_top_register_mask &= ~mask;

            /* Note: Nintendo ignores the return value. */
            R_DISCARD(I2cWrite(m_pmic_session, Max77620PmicIrqTopMReg, irq_top_register_mask));
        }
    }

    void IrqDevice::SetLbtInterruptsEnabled(u8 mask, bool enable) {
        /* Read the low battery/thermal interrupt enable register. */
        u8 lbt_reg = 0;
        if (R_FAILED(I2cRead(m_pmic_session, Max77620PmicIntEnLbtReg, lbt_reg))) {
            return;
        }

        if (enable) {
            /* Unmask the interrupts. */
            lbt_reg &= ~mask;
            if (R_FAILED(I2cWrite(m_pmic_session, Max77620PmicIntEnLbtReg, lbt_reg))) {
                return;
            }

            /* Unmask the top level interrupt. */
            UnmaskTopInterrupt(Max77620PmicIrqTopGlblMask);
        } else {
            /* Mask the interrupts. */
            lbt_reg |= mask;

            /* Note: Nintendo ignores the return value. */
            R_DISCARD(I2cWrite(m_pmic_session, Max77620PmicIntEnLbtReg, lbt_reg));
        }
    }

    void IrqDevice::Initialize() {
        /* Initialize pmic and rtc i2c sessions. */
        R_ABORT_UNLESS(i2c::OpenSession(std::addressof(m_pmic_session), i2c::DeviceCode_Max77620Pmic));
        R_ABORT_UNLESS(i2c::OpenSession(std::addressof(m_rtc_session),  i2c::DeviceCode_Max77620Rtc));

        /* Enable the interrupts. */
        EnableRtcInterrupts();
        EnableOnOffInterrupts();

        /* Enable the thermal alarm. */
        SetThermalAlarmEnabled(true);

        /* Enable the low battery interrupt. */
        SetLowBatteryEnabled(true);

        m_is_ready = true;
    }

    Result IrqDevice::SetRtcCallback(IrqCallbackFn callback) {
        R_UNLESS(m_is_ready, ResultDeviceNotReady());

        m_rtc_callback = callback;

        return ResultSuccess();
    }

    Result IrqDevice::SetOnOffCallback(IrqCallbackFn callback) {
        R_UNLESS(m_is_ready, ResultDeviceNotReady());

        m_on_off_callback = callback;

        return ResultSuccess();
    }

    void IrqDevice::CleanupInterruptGlblm() {
        /* Clear the global interrupt mask and write the register. */
        m_saved_int_en_lbt &= ~Max77620PmicGlblMMask;

        /* NOTE: Nintendo ignores the return value. */
        R_DISCARD(I2cWrite(m_pmic_session, Max77620PmicIntEnLbtReg, m_saved_int_en_lbt));
    }

    /* What even is glblm? */
    Result IrqDevice::EnableInterruptGlblm() {
        u8 int_en_lbt = 0;
        R_TRY(I2cRead(m_pmic_session, Max77620PmicIntEnLbtReg, int_en_lbt));

        int_en_lbt |= Max77620PmicGlblMMask;
        R_TRY(I2cWrite(m_pmic_session, Max77620PmicIntEnLbtReg, int_en_lbt));

        m_saved_int_en_lbt = int_en_lbt;

        return ResultSuccess();
    }

    Result IrqDevice::HandleGlobalPendingInterrupts(u8 irq_top) {
        /* Check if there's a global pending interrupt. */
        if (irq_top & Max77620PmicIrqTopGlblMask) {
            /* Read the low battery/thermal interrupt register. */
            u8 int_lbt = 0;
            R_TRY(I2cRead(m_pmic_session, Max77620PmicIntLbtReg, int_lbt));
        }

        return ResultSuccess();
    }

    Result IrqDevice::HandleRtcPendingInterrupts(u8 irq_top, u32 &rtc_flags) {
        /* Check for pending rtc interrupts. */
        if (irq_top & Max77620PmicIrqTopRtcMask) {

            /* Read rtc irq status. */
            u8 rtc_irq = 0;
            R_TRY(I2cRead(m_rtc_session, rtc::Max77620RtcIntReg, rtc_irq));

            /* Read the rtc mask. */
            u8 rtc_mask = 0;
            R_TRY(rtc::GetRtcDevice().ReadInterruptMask(std::addressof(rtc_mask)));

            /* Set pending interrupt flags. */
            /* Unsure what these flags actually are. */
            const u32 pending = rtc_irq & ~rtc_mask;
            rtc_flags = 0;
            if (pending & rtc::Max77620RtcIrqBit1Mask) { rtc_flags |= rtc::Max77620RtcFlagFromBit1; }
            if (pending & rtc::Max77620RtcIrqBit4Mask) { rtc_flags |= rtc::Max77620RtcFlagFromBit4; }
            if (pending & rtc::Max77620RtcIrqBit0Mask) { rtc_flags |= rtc::Max77620RtcFlagFromBit0; }
            if (pending & rtc::Max77620RtcIrqBit2Mask) { rtc_flags |= rtc::Max77620RtcFlagFromBit2; }
        }

        return ResultSuccess();
    }

    Result IrqDevice::HandleOnOffPendingInterrupts(u8 irq_top, u32 &on_off_flags) {
        /* Check for pending on off interrupts. */
        if (irq_top & Max77620PmicIrqTopOnOffMask) {
            u8 on_off_irq = 0, on_off_mask = 0;
            R_TRY(I2cRead(m_pmic_session, Max77620PmicOnOffIrqReg,  on_off_irq));
            R_TRY(I2cRead(m_pmic_session, Max77620PmicOnOffIrqMReg, on_off_mask));

            on_off_flags = on_off_irq & ~on_off_mask;
        }

        return ResultSuccess();
    }

    Result IrqDevice::HandleInterrupt() {
        R_UNLESS(m_is_ready, ResultDeviceNotReady());

        /* Enable interrupt glblm. */
        R_TRY(EnableInterruptGlblm());
        ON_SCOPE_EXIT { CleanupInterruptGlblm(); };

        /* Read top level interrupt status. */
        u8 irq_top = 0;
        R_TRY(I2cRead(m_pmic_session, Max77620PmicIrqTopReg, irq_top));

        /* Handle global pending interrupts. */
        R_TRY(HandleGlobalPendingInterrupts(irq_top));

        /* Handle rtc pending interrupts. */
        u32 rtc_flags     = 0;
        R_TRY(HandleRtcPendingInterrupts(irq_top, rtc_flags));

        /* Handle pending on off interrupts. */
        u32 on_off_flags  = 0;
        R_TRY(HandleOnOffPendingInterrupts(irq_top, on_off_flags));

        /* Handle the callbacks. */
        /* NOTE: Nintendo calls the thermal and low battery callbacks, but these are never actually set. */
        if (rtc_flags    && m_rtc_callback)    { m_rtc_callback(rtc_flags); }
        if (on_off_flags && m_on_off_callback) { m_on_off_callback(on_off_flags); }

        return ResultSuccess();
    }

}
