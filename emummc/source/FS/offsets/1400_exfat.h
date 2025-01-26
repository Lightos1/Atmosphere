/*
 * Copyright (c) 2019 m4xw <m4x@m4xw.net>
 * Copyright (c) 2019 Atmosphere-NX
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
#ifndef __FS_1400_EXFAT_H__
#define __FS_1400_EXFAT_H__

// Accessor vtable getters
#define FS_OFFSET_1400_EXFAT_SDMMC_ACCESSOR_GC   0x1952D0
#define FS_OFFSET_1400_EXFAT_SDMMC_ACCESSOR_SD   0x1970E0
#define FS_OFFSET_1400_EXFAT_SDMMC_ACCESSOR_NAND 0x195800

// Hooks
#define FS_OFFSET_1400_EXFAT_SDMMC_WRAPPER_READ  0x190E70
#define FS_OFFSET_1400_EXFAT_SDMMC_WRAPPER_WRITE 0x190ED0
#define FS_OFFSET_1400_EXFAT_RTLD                0x282B8
#define FS_OFFSET_1400_EXFAT_RTLD_DESTINATION    ((uintptr_t)(INT64_C(-0x3C)))

#define FS_OFFSET_1400_EXFAT_CLKRST_SET_MIN_V_CLK_RATE 0x1B1110

// Misc funcs
#define FS_OFFSET_1400_EXFAT_LOCK_MUTEX          0x189D70
#define FS_OFFSET_1400_EXFAT_UNLOCK_MUTEX        0x189DC0

#define FS_OFFSET_1400_EXFAT_SDMMC_WRAPPER_CONTROLLER_OPEN  0x190E20
#define FS_OFFSET_1400_EXFAT_SDMMC_WRAPPER_CONTROLLER_CLOSE 0x190E50

// Misc Data
#define FS_OFFSET_1400_EXFAT_SD_MUTEX            0x10123F0
#define FS_OFFSET_1400_EXFAT_NAND_MUTEX          0x100D2F8
#define FS_OFFSET_1400_EXFAT_ACTIVE_PARTITION    0x100D338
#define FS_OFFSET_1400_EXFAT_SDMMC_DAS_HANDLE    0xE0B9C8

// NOPs
#define FS_OFFSET_1400_EXFAT_SD_DAS_INIT         0x27004

// Nintendo Paths
#define FS_OFFSET_1400_EXFAT_NINTENDO_PATHS \
{ \
	{.opcode_reg = 3, .adrp_offset = 0x0006D9C0, .add_rel_offset = 0x00000004}, \
	{.opcode_reg = 3, .adrp_offset = 0x0007AC24, .add_rel_offset = 0x00000004}, \
	{.opcode_reg = 4, .adrp_offset = 0x000813E8, .add_rel_offset = 0x00000004}, \
	{.opcode_reg = 4, .adrp_offset = 0x0009387C, .add_rel_offset = 0x00000004}, \
	{.opcode_reg = 0, .adrp_offset = 0, .add_rel_offset = 0}, \
}

#endif // __FS_1400_EXFAT_H__
