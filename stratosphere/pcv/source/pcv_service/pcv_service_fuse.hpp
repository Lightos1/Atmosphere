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

namespace ams::pcv::detail {

    struct FuseInfo {
        u32 cpu_speedo;
        u32 gpu_speedo;
        u32 soc_speedo;

        u32 cpu_iddq;
        u32 gpu_iddq;
        u32 soc_iddq;
    };
    static_assert(sizeof(FuseInfo) == 6 * sizeof(u32));

    void InitializeFuseInfo();

    namespace impl {

        void GetFuseInfo(FuseInfo *out_info, s32 max_count, s32 *out_count);
        void GetDramId(u32 *out_id);

    }

}
