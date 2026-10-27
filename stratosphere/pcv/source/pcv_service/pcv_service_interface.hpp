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

#define AMS_PCV_I_MANAGER_INTERFACE_INFO(C, H)                                                                                                                      \
    AMS_SF_METHOD_INFO(C, H, 26, Result, GetFuseInfo, (const sf::OutArray<u32> &out_info, s32 max_count, sf::Out<s32> out_count), (out_info, max_count, out_count)) \
    AMS_SF_METHOD_INFO(C, H, 27, Result, GetDramId,   (sf::Out<u32> out_id),                                                      (out_id))

AMS_SF_DEFINE_INTERFACE(ams::pcv::detail, IPcvManager, AMS_PCV_I_MANAGER_INTERFACE_INFO, 0x49506376)
