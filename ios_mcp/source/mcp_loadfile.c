/* Patch for MCP_LoadFile (ioctl 0x53).
 *
 * Reference for most of the types and whatever:
 * https://github.com/exjam/decaf-emu/tree/ios/src/libdecaf/src/ios/mcp
 *
 * This is a proof of concept, and shouldn't be used until it's cleaned up a bit.
 * Co-authored by QuarkTheAwesome, exjam and Maschell
 *
 * Flow of calls:
 * - kernel loads system libraries, app rpx or user calls OSDynLoad
 * - goes to loader.elf, which will call ioctl 0x53
 *   - with fileType = LOAD_FILE_CAFE_OS
 *   - on failure, again with LOAD_FILE_PROCESS_CODE
 *   - on failure, again with LOAD_FILE_SYS_DATA_CODE
 * - each request routes here where we can do whatever
 */

#include "ipc_types.h"

typedef enum {
    //Load from the process's code directory (process title id)/code/%s
    LOAD_FILE_PROCESS_CODE = 0,
    //Load from the CafeOS directory (00050010-1000400A)/code/%s
    LOAD_FILE_CAFE_OS = 1,
    //Load from a system data title's content directory (0005001B-x)/content/%s
    LOAD_FILE_SYS_DATA_CONTENT = 2,
    //Load from a system data title's code directory (0005001B-x)/content/%s
    LOAD_FILE_SYS_DATA_CODE = 3,
} MCPFileType;

typedef struct {
    unsigned char unk[0x10];

    unsigned int pos;
    MCPFileType type;
    unsigned int cafe_pid;

    unsigned char unk2[0xC];

    char name[0x40];

    unsigned char unk3[0x12D8 - 0x68];
} __attribute__((packed)) MCPLoadFileRequest;
//sizeof(MCPLoadFileRequest) = 0x12D8

int (*const real_MCP_LoadFile)(ipcmessage* msg) = (void*)0x0501CAA8 + 1; //+1 for thumb

int _MCP_LoadFile_patch(ipcmessage* msg) {
    if (msg->ioctl.length_in > 0 /*TODO log this and verify it's 0x12D8*/) {
        MCPLoadFileRequest* request = (MCPLoadFileRequest*)msg->ioctl.buffer_in;
    /*  loader.elf tries this value first */
        if (request->type == LOAD_FILE_CAFE_OS) {
        /*  Your code here */
        }
    }
    
    return real_MCP_LoadFile(msg);
}
