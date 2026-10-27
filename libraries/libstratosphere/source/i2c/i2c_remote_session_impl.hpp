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

namespace ams::i2c {

    #if defined(ATMOSPHERE_OS_HORIZON)
    class RemoteSessionImpl {
        private:
            ::Service m_srv;
        public:
            RemoteSessionImpl(::Service &s) : m_srv(s) { /* ... */ }

            ~RemoteSessionImpl() { ::serviceClose(std::addressof(m_srv)); }
        public:
            /* Commands for firmwares before 6.0.0. */
            Result SendOld(const ams::sf::InBuffer &in_data, i2c::TransactionOption option) {
                /* TODO: libnx bindings */
                AMS_UNUSED(in_data, option);
                AMS_ABORT();
            }

            Result ReceiveOld(const ams::sf::OutBuffer &out_data, i2c::TransactionOption option) {
                /* TODO: libnx bindings */
                AMS_UNUSED(out_data, option);
                AMS_ABORT();
            }

            Result ExecuteCommandListOld(const ams::sf::OutBuffer &rcv_buf, const ams::sf::InPointerArray<i2c::I2cCommand> &command_list) {
                /* TODO: libnx bindings */
                AMS_UNUSED(rcv_buf, command_list);
                AMS_ABORT();
            }

            /* Actual commands. */
            Result Send(const ams::sf::InAutoSelectBuffer &in_data, i2c::TransactionOption option) {
                const u32 raw_option = static_cast<u32>(option);

                R_RETURN(serviceDispatchIn(std::addressof(m_srv), 10, raw_option,
                    .buffer_attrs = { SfBufferAttr_In | SfBufferAttr_HipcAutoSelect },
                    .buffers      = { { in_data.GetPointer(), in_data.GetSize() } },
                ));
            }

            Result Receive(const ams::sf::OutAutoSelectBuffer &out_data, i2c::TransactionOption option) {
                const u32 raw_option = static_cast<u32>(option);

                R_RETURN(serviceDispatchIn(std::addressof(m_srv), 11, raw_option,
                    .buffer_attrs = { SfBufferAttr_Out | SfBufferAttr_HipcAutoSelect },
                    .buffers      = { { out_data.GetPointer(), out_data.GetSize() } },
                ));
            }

            Result ExecuteCommandList(const ams::sf::OutAutoSelectBuffer &rcv_buf, const ams::sf::InPointerArray<i2c::I2cCommand> &command_list) {
                R_RETURN(serviceDispatch(std::addressof(m_srv), 12,
                    .buffer_attrs = {
                        SfBufferAttr_Out | SfBufferAttr_HipcAutoSelect,
                        SfBufferAttr_In  | SfBufferAttr_HipcPointer,
                    },
                    .buffers = {
                        { rcv_buf.GetPointer(),      rcv_buf.GetSize()      },
                        { command_list.GetPointer(), command_list.GetSize() },
                    },
                ));
            }

            Result SetRetryPolicy(s32 max_retry_count, s32 retry_interval_us) {
                const struct {
                    s32 max_retry_count;
                    s32 retry_interval_us;
                } in = { max_retry_count, retry_interval_us };

                R_RETURN(serviceDispatchIn(std::addressof(m_srv), 13, in));
            }
    };
    static_assert(i2c::sf::IsISession<RemoteSessionImpl>);
    #endif

}
