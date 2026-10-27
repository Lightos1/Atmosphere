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
#include "ldr_embedded_code.hpp"

/* Generated header */
#include "pcv_bin.h"

namespace ams::ldr {

    void RegisterEmbeddedCodePackages() {
        /* pcv is launched before the sd card is available. */
        /* so it can't be launched from stratosphere.romfs. */
        /* Instead, we do the warcrime of embedding it into loader. */
        fs::RegisterEmbeddedCodePackage(ncm::SystemProgramId::Pcv, pcv_bin, pcv_bin_size);
    }

}
