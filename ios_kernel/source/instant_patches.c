/***************************************************************************
 * Copyright (C) 2016
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 ***************************************************************************/
#include "utils.h"
#include "types.h"
#include "elf_patcher.h"
#include "kernel_patches.h"
#include "ios_mcp_patches.h"
#include "../../ios_mcp/ios_mcp_syms.h"

typedef struct
{
    u32 paddr;
    u32 vaddr;
    u32 size;
    u32 domain;
    u32 type;
    u32 cached;
} ios_map_shared_info_t;

void instant_patches_setup(void)
{
    // apply IOS ELF launch hook
	*(volatile u32*)0x0812A120 = ARM_BL(0x0812A120, kernel_launch_ios);

    // patch FSA raw access
	*(volatile u32*)0x1070FAE8 = 0x05812070;
	*(volatile u32*)0x1070FAEC = 0xEAFFFFF9;
}
