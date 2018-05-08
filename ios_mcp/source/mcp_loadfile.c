int _MCP_LoadFile_patch(int arg1, int arg2, int arg3, int arg4, char arg5) {
    int (*real_func)(int arg1, int arg2, int arg3, int arg4, char arg5) = (void*)0x0501CAA8 + 1; //thumb
    return real_func(arg1, arg2, arg3, arg4, arg5);
}
