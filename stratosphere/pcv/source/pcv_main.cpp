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

#include "rtc/pcv_rtc.hpp"
#include "rtc/pcv_rtc_service.hpp"

#include "bpc/pcv_bpc.hpp"
#include "bpc/pcv_bpc_service.hpp"

#include "pcv_service/pcv_service_fuse.hpp"
#include "pcv_service/pcv_service.hpp"

namespace ams {

    namespace pcv {

        namespace rtc {

            namespace {

                constexpr sm::ServiceName RtcServiceName = sm::ServiceName::Encode("rtc");
                constexpr size_t          RtcMaxSessions = 3;

                sf::hipc::ServerManager<1, sf::hipc::DefaultServerManagerOptions, RtcMaxSessions> g_server_manager;
                sf::UnmanagedServiceObject<IRtcManager, RtcService> g_rtc_service;

                alignas(os::ThreadStackAlignment) constinit u8 g_thread_stack[0x1000];
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

        }

        namespace bpc {

            namespace {
                constexpr sm::ServiceName BpcServiceName = sm::ServiceName::Encode("bpc");
                constexpr size_t          BpcMaxSessions = 13;

                sf::hipc::ServerManager<1, sf::hipc::DefaultServerManagerOptions, BpcMaxSessions> g_server_manager;
                sf::UnmanagedServiceObject<IBpcManager, BpcService> g_bpc_service;

                alignas(os::ThreadStackAlignment) constinit u8 g_thread_stack[0x1000];
                constinit os::ThreadType g_thread;

                void ThreadFunction(void *) {
                    g_server_manager.LoopProcess();
                }

            }

            void InitializeServer() {
                R_ABORT_UNLESS(g_server_manager.RegisterObjectForServer(g_bpc_service.GetShared(), BpcServiceName, BpcMaxSessions));
            }

            void StartServer() {
                R_ABORT_UNLESS(os::CreateThread(std::addressof(g_thread), ThreadFunction, nullptr, g_thread_stack, sizeof(g_thread_stack), AMS_GET_SYSTEM_THREAD_PRIORITY(pcv, BpcIpcServer)));
                os::SetThreadNamePointer(std::addressof(g_thread), AMS_GET_SYSTEM_THREAD_NAME(pcv, BpcIpcServer));
                os::StartThread(std::addressof(g_thread));
            }

        }

        namespace detail {

            namespace {

                constexpr sm::ServiceName PcvServiceName = sm::ServiceName::Encode("pcv");
                constexpr size_t          PcvMaxSessions = 30;

                sf::hipc::ServerManager<1, sf::hipc::DefaultServerManagerOptions, PcvMaxSessions> g_server_manager;
                sf::UnmanagedServiceObject<IPcvManager, PcvService> g_pcv_service;

                alignas(os::ThreadStackAlignment) constinit u8 g_thread_stack[0x1000];
                constinit os::ThreadType g_thread;

                void ThreadFunction(void *) {
                    g_server_manager.LoopProcess();
                }
            }

            void InitializeServer() {
                R_ABORT_UNLESS(g_server_manager.RegisterObjectForServer(g_pcv_service.GetShared(), PcvServiceName, PcvMaxSessions));
            }

            void StartServer() {
                R_ABORT_UNLESS(os::CreateThread(std::addressof(g_thread), ThreadFunction, nullptr, g_thread_stack, sizeof(g_thread_stack), AMS_GET_SYSTEM_THREAD_PRIORITY(pcv, PcvIpcServer)));
                os::SetThreadNamePointer(std::addressof(g_thread), AMS_GET_SYSTEM_THREAD_NAME(pcv, PcvIpcServer));
                os::StartThread(std::addressof(g_thread));
            }

            void WaitServer() {
                os::WaitThread(std::addressof(g_thread));
            }

        }

    }

    namespace init {

        void InitializeSystemModule() {
            /* Initialize our connection to sm. */
            R_ABORT_UNLESS(sm::Initialize());

            /* Verify that we can sanely execute. */
            ams::CheckApiVersion();

            i2c::Initialize();

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

        pcv::bpc::Initialize();
        pcv::bpc::StartPowerManager();
        pcv::bpc::InitializeServer();
        pcv::bpc::StartServer();

        pcv::detail::InitializeFuseInfo();
        pcv::detail::InitializeServer();
        pcv::detail::StartServer();

        AMS_ABORT();

        pcv::detail::WaitServer();
    }

}
