.extern _MCP_LoadFile_patch

.global MCP_LoadFile_patch
MCP_LoadFile_patch:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, =_MCP_LoadFile_patch
    bx r12

.extern _MCP_ioctl64_patch

.global MCP_ioctl64_patch
MCP_ioctl64_patch:
    .thumb
    ldr r0, [r7,#0xC]
    bx pc
    nop
    .arm
    ldr r12, =_MCP_ioctl64_patch
    bx r12
