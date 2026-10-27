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
#include "pcv_service.hpp"
#include "pcv_service_fuse.hpp"

namespace ams::pcv::detail {

    Result PcvService::GetFuseInfo(const sf::OutArray<u32> &out_info, s32 max_count, sf::Out<s32> out_count) {
        impl::GetFuseInfo(reinterpret_cast<FuseInfo *>(out_info.GetPointer()), max_count, out_count.GetPointer());

        return ResultSuccess();
    }

    Result PcvService::GetDramId(sf::Out<u32> out_id) {
        impl::GetDramId(out_id.GetPointer());

        return ResultSuccess();
    }

}
