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

#include "pcv_bpc_pmic_registers.hpp"
#include "../pcv_i2c.hpp"

namespace ams::pcv::bpc {

    class IrqDevice {
        public:
            using IrqCallbackFn = void(*)(u32 flags);
        private:
            bool m_is_ready = false;
            u8 m_saved_int_en_lbt      = 0;

            i2c::I2cSession m_pmic_session{};
            i2c::I2cSession m_rtc_session{};

            /* Note: Nintendo has no thermal or low battery callbacks, and registers a nop rtc callback. */
            IrqCallbackFn m_rtc_callback         = nullptr;
            IrqCallbackFn m_on_off_callback      = nullptr;
        private:
            void UnmaskTopInterrupt(u8 mask);
            void SetLbtInterruptsEnabled(u8 mask, bool enable);
        private:
            Result EnableInterruptGlblm();
            Result HandleGlobalPendingInterrupts(u8 irq_top);
            Result HandleRtcPendingInterrupts(u8 irq_top, u32 &rtc_flags);
            Result HandleOnOffPendingInterrupts(u8 irq_top, u32 &on_off_flags);
            void CleanupInterruptGlblm();
        private:

            void EnableRtcInterrupts() {
                UnmaskTopInterrupt(Max77620PmicIrqTopRtcMask);
            }

            void EnableOnOffInterrupts() {
                UnmaskTopInterrupt(Max77620PmicIrqTopOnOffMask);
            }

            void SetThermalAlarmEnabled(bool enable) {
                SetLbtInterruptsEnabled(Max77620PmicIrqTjAlrm1Mask | Max77620PmicIrqTjAlrm2Mask, enable);
            }

            void SetLowBatteryEnabled(bool enable) {
                SetLbtInterruptsEnabled(Max77620PmicIrqLbmMask, enable);
            }
        public:
            IrqDevice() = default;

            void Initialize();
        public:
            Result SetOnOffCallback(IrqCallbackFn callback);
            Result SetRtcCallback(IrqCallbackFn callback);

            Result HandleInterrupt();
        public:
            ALWAYS_INLINE i2c::I2cSession &GetPmicSession()  { return m_pmic_session; }
            ALWAYS_INLINE i2c::I2cSession &GetRtcSession()   { return m_rtc_session; }
            ALWAYS_INLINE IrqCallbackFn GetOnOffCallback() const { return m_on_off_callback; }
    };

}
