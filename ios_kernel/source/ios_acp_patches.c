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
#include "types.h"
#include "elf_patcher.h"
#include "ios_acp_patches.h"
#include "../../ios_acp/ios_acp.bin.h"
#include "../../ios_acp/ios_acp_syms.h"

#define ACP_CODE_BASE_PHYS_ADDR     (-0xE0000000 + 0x12900000)

extern const patch_table_t acp_patches_table[];
extern const patch_table_t acp_patches_table_end[];

u32 acp_get_phys_code_base(void) {
    return _text_start + ACP_CODE_BASE_PHYS_ADDR;
}

void acp_run_patches(u32 ios_elf_start) {
    section_write_bss(ios_elf_start, _bss_start, _bss_end - _bss_start);
    section_write(ios_elf_start, _text_start, (void*)acp_get_phys_code_base(), _text_end - _text_start);

    // hook main entry
    //section_write_word(ios_elf_start, 0xe000fb98, ARM_BL(0xe000fb98, _mainHook));

    // hook acp parse metafunction
    //section_write_word(ios_elf_start, 0xe0013dec, ARM_BL(0xe0013dec, ACP_ParseMetaXML));
    //section_write_word(ios_elf_start, 0xe0020234, ARM_BL(0xe0020234, ACP_ParseMetaXML));

    //section_write_word(ios_elf_start, 0xe0015864, ARM_BL(0xe0015864, ACP_LoadMetaXmlFromPath));
    //section_write_word(ios_elf_start, 0xe00159f8, ARM_BL(0xe00159f8, ACP_LoadMetaXmlFromPath));
    //section_write_word(ios_elf_start, 0xe001a100, ARM_BL(0xe001a100, ACP_LoadMetaXmlFromPath));
    //section_write_word(ios_elf_start, 0xe001cc68, ARM_BL(0xe001cc68, ACP_LoadMetaXmlFromPath));
    //section_write_word(ios_elf_start, 0xe002a658, ARM_BL(0xe002a658, ACP_LoadMetaXmlFromPath));
    //section_write_word(ios_elf_start, 0xe0032680, ARM_BL(0xe0032680, ACP_LoadMetaXmlFromPath));

    //u32 patch_count = (u32)(((u8*)acp_patches_table_end) - ((u8*)acp_patches_table)) / sizeof(patch_table_t);
    //patch_table_entries(ios_elf_start, acp_patches_table, patch_count);
}
