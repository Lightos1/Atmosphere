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
#include "i2c_remote_session_impl.hpp"

namespace ams::i2c {

    #if defined(ATMOSPHERE_OS_HORIZON)
    class RemoteManagerImpl {
        private:
            ::Service *m_srv;
        public:
            constexpr explicit RemoteManagerImpl(::Service *srv) : m_srv(srv) { /* ... */ }

            ~RemoteManagerImpl() { /* ... */ }
        public:
            /* Commands for firmwares before 6.0.0. */
            Result OpenSessionForDev(ams::sf::Out<ams::sf::SharedPointer<i2c::sf::ISession>> out, s32 bus_idx, u16 slave_address, i2c::AddressingMode addressing_mode, i2c::SpeedMode speed_mode) {
                /* TODO: libnx bindings */
                AMS_UNUSED(out, bus_idx, slave_address, addressing_mode, speed_mode);
                AMS_ABORT();
            }

            Result OpenSession(ams::sf::Out<ams::sf::SharedPointer<i2c::sf::ISession>> out, i2c::I2cDevice device) {
                /* TODO: libnx bindings */
                AMS_UNUSED(out, device);
                AMS_ABORT();
            }

            Result HasDevice(ams::sf::Out<bool> out, i2c::I2cDevice device) {
                /* TODO: libnx bindings */
                AMS_UNUSED(out, device);
                AMS_ABORT();
            }

            Result HasDeviceForDev(ams::sf::Out<bool> out, i2c::I2cDevice device) {
                /* TODO: libnx bindings */
                AMS_UNUSED(out, device);
                AMS_ABORT();
            }

            /* Actual commands. */
            Result OpenSession2(ams::sf::Out<ams::sf::SharedPointer<i2c::sf::ISession>> out, DeviceCode device_code);
    };
    static_assert(i2c::sf::IsIManager<RemoteManagerImpl>);
    #endif

}
