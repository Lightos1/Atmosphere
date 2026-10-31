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
#include "pcv_bpc_power_manager.hpp"
#include "pcv_bpc_gpio.hpp"
#include "../pcv_i2c.hpp"

namespace ams::pcv::bpc {

    namespace {

        void ConfigurePmicRtcClockRegisters() {
            constexpr dd::PhysicalAddress PmcBase = 0x7000E400ul; /* APBDEV_PMC_CNTRL. */
            constexpr u32 IntrPolarity            = 0x20000;

            u32 apb_dev_pmc_cntrl = 0;
            R_ABORT_UNLESS(svc::ReadWriteRegister(std::addressof(apb_dev_pmc_cntrl), PmcBase, 0, 0));
            apb_dev_pmc_cntrl |= IntrPolarity;

            R_ABORT_UNLESS(svc::ReadWriteRegister(std::addressof(apb_dev_pmc_cntrl), PmcBase, 0xFFFFFFFF, apb_dev_pmc_cntrl));
        }

        constinit IrqDevice g_irq_device;
        constinit PowerManager g_power_manager;

        void OnOffCallback(u32 status) {
            g_power_manager.OnOffCallback(status);
        }

        void RtcCallBack(u32 status) {
            AMS_UNUSED(status);
        }

        void ReadShutdownReason() {
            u8 nverc = 0;

            /* Get the shutdown reason. */
            R_ABORT_UNLESS(I2cRead(g_irq_device.GetPmicSession(), Max77620PmicNvercReg, nverc));

            u8 reason = nverc & NvercEnvironmentShutdownMask;

            /* Check if the shutdown was forced through the user? */
            if (nverc & NvercUserShutdownMask) {
                reason |= Max77620PmicNvercShutdownPinMask;
            }

            /* Set the shutdown reason. */
            g_power_manager.SetShutdownReason(reason);
        }

    }

    void Initialize() {
        /* Configure the rtc clock registers. */
        ConfigurePmicRtcClockRegisters();

        /* Initialize the IRQ device. */
        g_irq_device.Initialize();

        /* Initialize the power manager. */
        g_power_manager.Initialize(std::addressof(g_irq_device));

        /* Set the rtc callback. */
        /* NOTE: Nintendo uses a stubbed function for this. */
        R_ABORT_UNLESS(g_irq_device.SetRtcCallback(RtcCallBack));

        /* Set the on/off callback. */
        R_ABORT_UNLESS(g_irq_device.SetOnOffCallback(OnOffCallback));

        /* Read the shutdown reason. */
        ReadShutdownReason();

        /* Initialize gpio devices. */
        gpio::InitializeDevices();
    }

    void StartPowerManager() {
        g_power_manager.Start();
    }

    PowerManager &GetPowerManager() {
        return g_power_manager;
    }

}
