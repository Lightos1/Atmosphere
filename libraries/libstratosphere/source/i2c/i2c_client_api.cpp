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
#include "i2c_remote_manager_impl.hpp"

namespace ams::i2c {

    namespace {

        constinit os::SdkMutex g_init_mutex;
        constinit int g_initialize_count = 0;

        constinit os::SdkMutex g_i2c_mutex;
        ams::sf::SharedPointer<sf::IManager> g_i2c_manager;
        constinit int g_i2c_count = 0;

        constinit os::SdkMutex g_i2c_pcv_mutex;
        ams::sf::SharedPointer<sf::IManager> g_i2c_pcv_manager;
        constinit int g_i2c_pcv_count = 0;

        #if defined(ATMOSPHERE_OS_HORIZON)
        constinit bool g_remote = false;
        constinit ::Service g_i2c_srv     = {};
        constinit ::Service g_i2c_pcv_srv = {};
        constinit ams::sf::UnmanagedServiceObject<sf::IManager, RemoteManagerImpl> g_remote_i2c_manager_impl(std::addressof(g_i2c_srv));
        constinit ams::sf::UnmanagedServiceObject<sf::IManager, RemoteManagerImpl> g_remote_i2c_pcv_manager_impl(std::addressof(g_i2c_pcv_srv));
        #endif

        i2c::sf::ISession *GetInterface(const I2cSession &session) {
            AMS_ASSERT(session._session != nullptr);
            return static_cast<i2c::sf::ISession *>(session._session);
        }

        ams::sf::SharedPointer<sf::IManager> GetManager(DeviceCode device_code) {
            if (IsPowerBusDeviceCode(device_code)) {
                return g_i2c_pcv_manager;
            } else {
                return g_i2c_manager;
            }
        }

    }

    void Initialize() {
        #if defined(ATMOSPHERE_OS_HORIZON)
        std::scoped_lock lk(g_init_mutex);

        if ((g_initialize_count++) == 0) {
            R_ABORT_UNLESS(::smGetService(std::addressof(g_i2c_srv),     "i2c"));
            R_ABORT_UNLESS(::smGetService(std::addressof(g_i2c_pcv_srv), "i2c:pcv"));
            g_remote = true;

            {
                std::scoped_lock lk(g_i2c_mutex);
                g_i2c_manager = g_remote_i2c_manager_impl.GetShared();
                AMS_ABORT_UNLESS(g_i2c_count == 0);
                g_i2c_count = 1;
            }

            {
                std::scoped_lock lk(g_i2c_pcv_mutex);
                g_i2c_pcv_manager = g_remote_i2c_pcv_manager_impl.GetShared();
                AMS_ABORT_UNLESS(g_i2c_pcv_count == 0);
                g_i2c_pcv_count = 1;
            }
        }
        #else
        AMS_ABORT("TODO");
        #endif
    }

    void InitializeWith(ams::sf::SharedPointer<i2c::sf::IManager> sp, ams::sf::SharedPointer<i2c::sf::IManager> sp_pcv) {
        std::scoped_lock lk(g_init_mutex);

        AMS_ABORT_UNLESS(g_initialize_count == 0);

        {
            std::scoped_lock lk(g_i2c_mutex);
            g_i2c_manager = std::move(sp);
            AMS_ABORT_UNLESS(g_i2c_count == 0);
            g_i2c_count = 1;
        }

        {
            std::scoped_lock lk(g_i2c_pcv_mutex);
            g_i2c_pcv_manager = std::move(sp_pcv);
            AMS_ABORT_UNLESS(g_i2c_pcv_count == 0);
            g_i2c_pcv_count = 1;
        }

        g_initialize_count = 1;
    }

    void InitializeEmpty() {
        std::scoped_lock lk(g_init_mutex);

        ++g_initialize_count;
    }

    void Finalize() {
        std::scoped_lock lk(g_init_mutex);

        AMS_ASSERT(g_initialize_count > 0);

        if ((--g_initialize_count) == 0) {
            {
                std::scoped_lock lk(g_i2c_mutex);
                AMS_ASSERT(g_i2c_count > 0);
                if (g_i2c_count > 0) {
                    if ((--g_i2c_count) == 0) {
                        g_i2c_manager = nullptr;
                    }
                }
            }
            {
                std::scoped_lock lk(g_i2c_pcv_mutex);
                AMS_ASSERT(g_i2c_pcv_count > 0);
                if (g_i2c_pcv_count > 0) {
                    if ((--g_i2c_pcv_count) == 0) {
                        g_i2c_manager = nullptr;
                    }
                }
            }

            #if defined(ATMOSPHERE_OS_HORIZON)
            if (g_remote) {
                ::serviceClose(std::addressof(g_i2c_pcv_srv));
                ::serviceClose(std::addressof(g_i2c_srv));
                g_remote = false;
            }
            #endif
        }
    }

    Result OpenSession(I2cSession *out, DeviceCode device_code) {
        /* Get manager for the device. */
        auto manager = GetManager(device_code);

        /* Get the session. */
        ams::sf::SharedPointer<i2c::sf::ISession> session;
        {
            if (hos::GetVersion() >= hos::Version_6_0_0) {
                R_TRY(manager->OpenSession2(std::addressof(session), device_code));
            } else {
                R_TRY(manager->OpenSession(std::addressof(session), ConvertToI2cDevice(device_code)));
            }
        }

        /* Set output. */
        out->_session = session.Detach();

        /* We succeeded. */
        R_SUCCEED();
    }

    void CloseSession(I2cSession &session) {
        /* Close the session. */
        ams::sf::ReleaseSharedObject(GetInterface(session));
        session._session = nullptr;
    }

    Result Send(const I2cSession &session, const void *src, size_t src_size, TransactionOption option) {
        const ams::sf::InAutoSelectBuffer buf(src, src_size);

        R_RETURN(GetInterface(session)->Send(buf, option));
    }

    Result Receive(void *dst, size_t dst_size, const I2cSession &session, TransactionOption option) {
        const ams::sf::OutAutoSelectBuffer buf(dst, dst_size);

        R_RETURN(GetInterface(session)->Receive(buf, option));
    }

    Result ExecuteCommandList(void *dst, size_t dst_size, const I2cSession &session, const void *src, size_t src_size) {
        const ams::sf::OutAutoSelectBuffer buf(dst, dst_size);
        const ams::sf::InPointerArray<i2c::I2cCommand> arr(static_cast<const i2c::I2cCommand *>(src), src_size);

        R_RETURN(GetInterface(session)->ExecuteCommandList(buf, arr));
    }

    void SetRetryPolicy(const I2cSession &session, int max_retry_count, int retry_interval_us) {
        AMS_ASSERT(max_retry_count >= 0);
        AMS_ASSERT(retry_interval_us >= 0);

        R_ABORT_UNLESS(GetInterface(session)->SetRetryPolicy(max_retry_count, retry_interval_us));
    }

}
