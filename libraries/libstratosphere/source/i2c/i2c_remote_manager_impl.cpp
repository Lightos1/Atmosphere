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

    #if defined(ATMOSPHERE_OS_HORIZON)
    namespace {

        struct I2cRemoteManagerTag;
        using RemoteAllocator     = ams::sf::ExpHeapStaticAllocator<1_KB, I2cRemoteManagerTag>;
        using RemoteObjectFactory = ams::sf::ObjectFactory<typename RemoteAllocator::Policy>;

        class StaticAllocatorInitializer {
            public:
                StaticAllocatorInitializer() {
                    RemoteAllocator::Initialize(lmem::CreateOption_None);
                }
        } g_static_allocator_initializer;

    }

    Result RemoteManagerImpl::OpenSession2(ams::sf::Out<ams::sf::SharedPointer<i2c::sf::ISession>> out, DeviceCode device_code) {
        const u32 raw_device_code = device_code.GetInternalValue();

        ::Service session_srv;
        R_TRY(serviceDispatchIn(m_srv, 4, raw_device_code,
            .out_num_objects = 1,
            .out_objects     = std::addressof(session_srv),
        ));

        out.SetValue(RemoteObjectFactory::CreateSharedEmplaced<i2c::sf::ISession, RemoteSessionImpl>(session_srv));
        R_SUCCEED();
    }
    #endif

}
