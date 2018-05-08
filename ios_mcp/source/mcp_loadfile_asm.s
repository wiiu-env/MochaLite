.extern _MCP_LoadFile_patch

.global MCP_LoadFile_patch
MCP_LoadFile_patch:
    .thumb
    bx pc
    nop
    .arm
    ldr r12, =_MCP_LoadFile_patch
    bx r12
