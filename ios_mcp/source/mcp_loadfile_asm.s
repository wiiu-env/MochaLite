.extern _MCP_LoadFile_patch
.global MCP_LoadFile_patch
MCP_LoadFile_patch:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, =_MCP_LoadFile_patch
    bx r12

.extern _MCP_0x51_mcpSwitchTitle
.global MCP_0x51_mcpSwitchTitle
MCP_0x51_mcpSwitchTitle:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, =_MCP_0x51_mcpSwitchTitle
    bx r12

.extern _MCP_ReadCOSXml_patch
.global MCP_ReadCOSXml_patch
MCP_ReadCOSXml_patch:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, =_MCP_ReadCOSXml_patch
    bx r12

.extern _MCP_ReadAPPXml_patch
.global MCP_ReadAPPXml_patch
MCP_ReadAPPXml_patch:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, =_MCP_ReadAPPXml_patch
    bx r12

.extern _MCP_ioctl_proccess
.global MCP_ioctl_proccess
MCP_ioctl_proccess:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, =_MCP_ioctl_proccess
    bx r12

.extern _MCP_0x49_0x52_PrepareTitle
.global MCP_0x49_0x52_PrepareTitle
MCP_0x49_0x52_PrepareTitle:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, = _MCP_0x49_0x52_PrepareTitle
    bx r12

.extern _MCP_ioctl100_patch
.global MCP_ioctl100_patch
MCP_ioctl100_patch:
    .thumb
    ldr r0, [r7,#0xC]
    bx pc
    nop
    .arm
    ldr r12, =_MCP_ioctl100_patch
    bx r12
