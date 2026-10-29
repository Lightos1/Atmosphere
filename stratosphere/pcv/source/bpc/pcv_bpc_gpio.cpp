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
#include "pcv_bpc_gpio.hpp"
#include "pcv_bpc_pmic_registers.hpp"
#include "../pcv_i2c.hpp"

namespace ams::pcv::bpc::gpio {

    namespace {

        constexpr u8  FpsRegNone     = 0xFF;
        constexpr u32 FpsSrc0        = 0;
        constexpr u32 FpsSrcNone     = 3;
        constexpr u32 FpsPeriodUnset = std::numeric_limits<u32>::max();

        constexpr size_t GpioCount = 8;

        constinit Max77620GpioEntry g_gpio_devices[GpioCount]{};

        /* Used when the PMICs CID4 register is 0x35. */
        constexpr Max77620GpioEntry GpioDevicesCid35[GpioCount] = {
            { .index = 0, .name = "max77620_gpio0", .gpio_index = 0, .gpio_cfg_reg = Max77620PmicGpio0Reg, .bit_mask = BIT(0), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 1, .name = "max77620_gpio1", .gpio_index = 1, .gpio_cfg_reg = Max77620PmicGpio1Reg, .bit_mask = BIT(1), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = Max77620PmicFpsGpio1Reg, .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 2, .name = "max77620_gpio2", .gpio_index = 2, .gpio_cfg_reg = Max77620PmicGpio2Reg, .bit_mask = BIT(2), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 1, .fps_reg = Max77620PmicFpsGpio2Reg, .fps_pu_period = 0,              .fps_pd_period = 7,              .fps_src = FpsSrc0    },
            { .index = 3, .name = "max77620_gpio3", .gpio_index = 3, .gpio_cfg_reg = Max77620PmicGpio3Reg, .bit_mask = BIT(3), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 1, .fps_reg = Max77620PmicFpsGpio3Reg, .fps_pu_period = 4,              .fps_pd_period = 2,              .fps_src = FpsSrc0    },
            { .index = 4, .name = "max77620_gpio4", .gpio_index = 4, .gpio_cfg_reg = Max77620PmicGpio4Reg, .bit_mask = BIT(4), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 5, .name = "max77620_gpio5", .gpio_index = 5, .gpio_cfg_reg = Max77620PmicGpio5Reg, .bit_mask = BIT(5), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 6, .name = "max77620_gpio6", .gpio_index = 6, .gpio_cfg_reg = Max77620PmicGpio6Reg, .bit_mask = BIT(6), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 7, .name = "max77620_gpio7", .gpio_index = 7, .gpio_cfg_reg = Max77620PmicGpio7Reg, .bit_mask = BIT(7), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
        };

        /* Used when the PMICs CID4 register is 0x53. */
        /* Note: max77620_gpio1 to max77620_gpio3 have FPS values set, but no fps register will be written since fps_configured is not set. */
        constexpr Max77620GpioEntry GpioDevicesCid53[GpioCount] = {
            { .index = 0, .name = "max77620_gpio0", .gpio_index = 0, .gpio_cfg_reg = Max77620PmicGpio0Reg, .bit_mask = BIT(0), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 1, .name = "max77620_gpio1", .gpio_index = 1, .gpio_cfg_reg = Max77620PmicGpio1Reg, .bit_mask = BIT(1), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = Max77620PmicFpsGpio1Reg, .fps_pu_period = 5,              .fps_pd_period = 0,              .fps_src = FpsSrc0    },
            { .index = 2, .name = "max77620_gpio2", .gpio_index = 2, .gpio_cfg_reg = Max77620PmicGpio2Reg, .bit_mask = BIT(2), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = Max77620PmicFpsGpio2Reg, .fps_pu_period = 6,              .fps_pd_period = 0,              .fps_src = FpsSrc0    },
            { .index = 3, .name = "max77620_gpio3", .gpio_index = 3, .gpio_cfg_reg = Max77620PmicGpio3Reg, .bit_mask = BIT(3), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = Max77620PmicFpsGpio3Reg, .fps_pu_period = 3,              .fps_pd_period = 0,              .fps_src = FpsSrc0    },
            { .index = 4, .name = "max77620_gpio4", .gpio_index = 4, .gpio_cfg_reg = Max77620PmicGpio4Reg, .bit_mask = BIT(4), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 5, .name = "max77620_gpio5", .gpio_index = 5, .gpio_cfg_reg = Max77620PmicGpio5Reg, .bit_mask = BIT(5), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 6, .name = "max77620_gpio6", .gpio_index = 6, .gpio_cfg_reg = Max77620PmicGpio6Reg, .bit_mask = BIT(6), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
            { .index = 7, .name = "max77620_gpio7", .gpio_index = 7, .gpio_cfg_reg = Max77620PmicGpio7Reg, .bit_mask = BIT(7), .unk13 = 0x40, .unk14 = 0x10, .unk15 = 0x10, .fps_configured = 0, .fps_reg = FpsRegNone,              .fps_pu_period = FpsPeriodUnset, .fps_pd_period = FpsPeriodUnset, .fps_src = FpsSrcNone },
        };

        i2c::I2cSession g_gpio_session{};

        u32 g_open_count = 0;

        void ConfigureFps(Max77620GpioEntry &entry) {
            constexpr u8  FpsSrcShift      = 6;
            constexpr u8  FpsSrcMask       = 0xC0;
            constexpr u8  FpsPuPeriodShift = 3;
            constexpr u8  FpsPuPeriodMask  = 0x38;
            constexpr u8  FpsPdPeriodMask  = 0x07;
            constexpr u32 FpsSrc2          = 2;
            constexpr u32 FpsPeriodCount   = 8;

            /* Check if we have to configure fps. */
            if (entry.fps_src > FpsSrc2 || !entry.fps_configured) {
                return;
            }

            /* Read the current FPS register */
            u8 reg_value = 0;
            if (R_FAILED(I2cRead(*entry.session, entry.fps_reg, reg_value))) {
                return;
            }

            u8 mask  = FpsSrcMask;
            u8 value = static_cast<u8>(entry.fps_src << FpsSrcShift);

            /* Write to Power-up and down perios if they're valid. */
            if (entry.fps_pu_period < FpsPeriodCount) {
                mask  |= FpsPuPeriodMask;
                value |= static_cast<u8>(entry.fps_pu_period << FpsPuPeriodShift);
            }

            if (entry.fps_pd_period < FpsPeriodCount) {
                mask  |= FpsPdPeriodMask;
                value |= static_cast<u8>(entry.fps_pd_period);
            }

            /* Write to the register. */
            reg_value = (reg_value & ~mask) | (value & mask);

            /* Note: Nintendo ignores the return value. */
            R_DISCARD(I2cWrite(*entry.session, entry.fps_reg, reg_value));
        }

        void InitializeEntry(u32 index) {
            if (g_open_count++ == 0) {
                /* Open the session */
                R_ABORT_UNLESS(i2c::OpenSession(std::addressof(g_gpio_session), i2c::DeviceCode_Max77620Pmic));

                /* NOTE: Nintendo ignores this. */
                u8 cid4 = 0;
                R_DISCARD(I2cRead(g_gpio_session, Max77620PmicCid4Reg, cid4));

                /* Copy correct gpio entries depending on the register. */
                if (cid4 == 0x35) {
                    std::memcpy(std::addressof(g_gpio_devices), std::addressof(GpioDevicesCid35), sizeof(GpioDevicesCid35));
                } else {
                    /* cid value 0x53 is the only other valid value. */
                    AMS_ABORT_UNLESS(cid4 == 0x53);
                    std::memcpy(std::addressof(g_gpio_devices), std::addressof(GpioDevicesCid53), sizeof(GpioDevicesCid53));
                }
            }

            /* Set the session. */
            g_gpio_devices[index].session = std::addressof(g_gpio_session);

            /* Connection test? */
            {
                u8 dummy = 0;
                R_ABORT_UNLESS(I2cRead(g_gpio_session, Max77620PmicGpio0Reg, dummy));
            }

            /* Configure fps. */
            ConfigureFps(g_gpio_devices[index]);

            /* Mark the device as opened. */
            g_gpio_devices[index].opened = true;
        }

        void FinalizeEntry(u32 index) {
            if (!g_gpio_devices[index].opened) {
                return;
            }

            /* Close the session once the last device is closed. */
            if (--g_open_count == 0) {
                i2c::CloseSession(g_gpio_session);
            }

            /* Mark the device as closed. */
            g_gpio_devices[index].opened = false;
        }

        bool g_is_initialized = false;

    }

    void InitializeDevices() {
        /* Don't run this if it has already been initialized. */
        if (g_is_initialized) {
            return;
        }
        g_is_initialized = true;

        /* Initialize all devices. */
        for (u32 i = 0; i < GpioCount; ++i) {
            InitializeEntry(i);
            FinalizeEntry(i);
        }
    }

}
