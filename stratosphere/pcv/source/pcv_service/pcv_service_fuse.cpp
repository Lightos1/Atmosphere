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
#include "pcv_service_fuse.hpp"

namespace ams::pcv::detail {

    namespace {

        constinit u32 g_dram_id = 0;

        constinit FuseInfo g_fuse_info = {};

        constexpr u32 FuseCpuSpeedoCalib = 0x114;
        constexpr u32 FuseGpuSpeedoCalib = 0x130;
        constexpr u32 FuseSocSpeedoCalib = 0x134;

        constexpr u32 FuseCpuIddqCalib = 0x118;
        constexpr u32 FuseGpuIddqCalib = 0x228;
        constexpr u32 FuseSocIddqCalib = 0x140;

        constexpr u32 MaxFuseInfoCount = sizeof(FuseInfo) / sizeof(u32);

        void GetFuseInfo() {
            constexpr dd::PhysicalAddress FuseBase = 0x7000F800;

            uintptr_t virt_address = dd::QueryIoMapping(FuseBase, FuseBase);

            g_fuse_info.cpu_speedo = *reinterpret_cast<u32 *>(virt_address + FuseCpuSpeedoCalib);
            g_fuse_info.gpu_speedo = *reinterpret_cast<u32 *>(virt_address + FuseGpuSpeedoCalib);
            g_fuse_info.soc_speedo = *reinterpret_cast<u32 *>(virt_address + FuseSocSpeedoCalib);

            g_fuse_info.cpu_iddq   = *reinterpret_cast<u32 *>(virt_address + FuseCpuIddqCalib) * 4;
            g_fuse_info.gpu_iddq   = *reinterpret_cast<u32 *>(virt_address + FuseGpuIddqCalib) * 5;
            g_fuse_info.soc_iddq   = *reinterpret_cast<u32 *>(virt_address + FuseSocIddqCalib) * 4;
        }
    }

    namespace impl {

        void GetFuseInfo(FuseInfo *out_info, s32 max_count, s32 *out_count) {
            const s32 count = std::clamp<s32>(max_count, 0, MaxFuseInfoCount);
            std::memcpy(out_info, std::addressof(g_fuse_info), count * sizeof(u32));
            *out_count = count;
        }

        void GetDramId(u32 *out_id) {
            *out_id = g_dram_id;
        }

    }

    void InitializeFuseInfo() {
        splInitialize();

        /* Get the dram id. */
        u64 dram_id_config = 0;
        R_ABORT_UNLESS(splGetConfig(SplConfigItem_DramId, std::addressof(dram_id_config)));
        splExit();

        g_dram_id = dram_id_config;

        /* Copy fuse info. (speedos and iddq) */
        GetFuseInfo();
    }

}
