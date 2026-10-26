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

#include "rtc/pcv_rtc_driver.hpp"
#include "rtc/pcv_rtc_service.hpp"

namespace ams {

    namespace pcv::rtc {

        namespace {

            constexpr sm::ServiceName RtcServiceName = sm::ServiceName::Encode("rtc");
            constexpr size_t          RtcMaxSessions = 8;

            sf::hipc::ServerManager<1, sf::hipc::DefaultServerManagerOptions, RtcMaxSessions> g_server_manager;
            sf::UnmanagedServiceObject<IRtcManager, RtcService> g_rtc_service;

            alignas(os::ThreadStackAlignment) constinit u8 g_thread_stack[0x4000];
            constinit os::ThreadType g_thread;

            void ThreadFunction(void *) {
                g_server_manager.LoopProcess();
            }

        }

        void InitializeServer() {
            R_ABORT_UNLESS(g_server_manager.RegisterObjectForServer(g_rtc_service.GetShared(), RtcServiceName, RtcMaxSessions));
        }

        void StartServer() {
            R_ABORT_UNLESS(os::CreateThread(std::addressof(g_thread), ThreadFunction, nullptr, g_thread_stack, sizeof(g_thread_stack), AMS_GET_SYSTEM_THREAD_PRIORITY(pcv, RtcIpcServer)));
            os::SetThreadNamePointer(std::addressof(g_thread), AMS_GET_SYSTEM_THREAD_NAME(pcv, RtcIpcServer));
            os::StartThread(std::addressof(g_thread));
        }

        void WaitServer() {
            os::WaitThread(std::addressof(g_thread));
        }

    }

    namespace init {

        void InitializeSystemModule() {
            /* Initialize our connection to sm. */
            R_ABORT_UNLESS(sm::Initialize());

            /* Verify that we can sanely execute. */
            ams::CheckApiVersion();

            R_ABORT_UNLESS(::setsysInitialize());
        }

        void FinalizeSystemModule() { /* ... */ }

        void Startup() { /* ... */ }

    }

    void Main() {
        /* Set thread name. */
        os::SetThreadNamePointer(os::GetCurrentThread(), AMS_GET_SYSTEM_THREAD_NAME(pcv, Main));
        AMS_ASSERT(os::GetThreadPriority(os::GetCurrentThread()) == AMS_GET_SYSTEM_THREAD_PRIORITY(pcv, Main));

        R_ABORT_UNLESS(pcv::rtc::Initialize());
        pcv::rtc::InitializeServer();
        pcv::rtc::StartServer();

        AMS_INFINITE_LOOP();
    }

}
