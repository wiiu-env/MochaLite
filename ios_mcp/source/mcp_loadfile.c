/* Patch for MCP_LoadFile (ioctl 0x53).
 * Also adds a sibling ioctl, 0x64, that allows setting a custom path to a main RPX
 *
 * Reference for most of the types and whatever:
 * https://github.com/exjam/decaf-emu/tree/ios/src/libdecaf/src/ios/mcp
 *
 * This is a proof of concept, and shouldn't be used until it's cleaned up a bit.
 * Co-authored by exjam, Maschell and QuarkTheAwesome
 *
 * Flow of calls:
 * - kernel loads system libraries, app rpx or user calls OSDynLoad
 * - goes to loader.elf, which will call ioctl 0x53
 *   - with fileType = LOAD_FILE_CAFE_OS
 *   - on failure, again with LOAD_FILE_PROCESS_CODE
 *   - on failure, again with LOAD_FILE_SYS_DATA_CODE
 * - each request routes here where we can do whatever
 */

#include "logger.h"
#include "ipc_types.h"
#include <string.h>

typedef enum {
    //Load from the process's code directory (process title id)/code/%s
    LOAD_FILE_PROCESS_CODE = 0,
    //Load from the CafeOS directory (00050010-1000400A)/code/%s
    LOAD_FILE_CAFE_OS = 1,
    //Load from a system data title's content directory (0005001B-x)/content/%s
    LOAD_FILE_SYS_DATA_CONTENT = 2,
    //Load from a system data title's code directory (0005001B-x)/content/%s
    LOAD_FILE_SYS_DATA_CODE = 3,

    LOAD_FILE_FORCE_SIZE = 0xFFFFFFFF,
} MCPFileType;

typedef struct {
    unsigned char unk[0x10];

    unsigned int pos;
    MCPFileType type;
    unsigned int cafe_pid;

    unsigned char unk2[0xC];

    char name[0x40];

    unsigned char unk3[0x12D8 - 0x68];
} MCPLoadFileRequest;
//sizeof(MCPLoadFileRequest) = 0x12D8

int (*const real_MCP_LoadFile)(ipcmessage* msg) = (void*)0x0501CAA8 + 1; //+1 for thumb
int (*const MCP_DoLoadFile)(const char* path, const char* path2, void* outputBuffer, u32 outLength, u32 pos, int* bytesRead, u32 unk) = (void*)0x05017248 + 1;
int (*const MCP_UnknownStuff)(const char* path, u32 pos, void* outputBuffer, u32 outLength, u32 outLength2, u32 unk) = (void*)0x05014CAC + 1;

static int MCP_LoadCustomFile(char* path, ipcmessage* msg, MCPLoadFileRequest* request);
static bool replacerpx = false;
static bool didrpxfirstchunk = false;
static char rpxpath[0x280];

#define log(fmt, ...) log_printf("%s: " fmt, __FUNCTION__, __VA_ARGS__)
#define FAIL_ON(cond, val) \
    if (cond) { \
        log(#cond " (%08X)", val); \
        return -29; \
    }

int _MCP_LoadFile_patch(ipcmessage* msg) {
    FAIL_ON(!msg->ioctl.buffer_in, 0);
    FAIL_ON(msg->ioctl.length_in != 0x12D8, msg->ioctl.length_in);
    FAIL_ON(!msg->ioctl.buffer_io, 0);
    FAIL_ON(!msg->ioctl.length_io, 0);

    MCPLoadFileRequest* request = (MCPLoadFileRequest*)msg->ioctl.buffer_in;
    log("msg->ioctl.buffer_io = %p, msg->ioctl.length_io = 0x%X\n", msg->ioctl.buffer_io, msg->ioctl.length_io);
    log("request->type = %d, request->pos = %d, request->name = \"%s\"\n", request->type, request->pos, request->name);

    if (request->type == LOAD_FILE_CAFE_OS &&
        request->name[0] == '*') {
        char path[0x40];

    /*  Translate request->name to a path by replacing * with / */
        for (int i = 0; i < 0x40; ++i) {
            if (request->name[i] == '*') {
                path[i] = '/';
            } else {
                path[i] = request->name[i];
            }
        }

        int result = MCP_LoadCustomFile(path, msg, request);
        if (result >= 0) return result;
    } else if (replacerpx) {
    /*  RPX replacement!
        Only replace this chunk if:
        - replacerpx is true (replace the next rpx to be loaded)
        - this file is an rpx
        and either of the following:
        - we haven't read the first chunk yet
        - this is not the first chunk

        The goal here is only to replace an rpx once. Reading at pos = 0 signifies a
        new rpx load - these conditions detect that. */
        char* extension = request->name + strlen(request->name) - 3;
        if (extension[0] == 'r' &&
            extension[1] == 'p' &&
            extension[2] == 'x') {

            if (!didrpxfirstchunk || request->pos > 0) {
                int result = MCP_LoadCustomFile(rpxpath, msg, request);
                if (result >= 0) {
                    if (request->pos == 0) didrpxfirstchunk = true;
                    return result;
                }
            } else {
            /*  This is the second time reading the first chunk of an rpx.
                Therefore we have already replaced the rpx we were asked to. */
                replacerpx = false;
            }
        }
    }

    return real_MCP_LoadFile(msg);
}

static int MCP_LoadCustomFile(char* path, ipcmessage* msg, MCPLoadFileRequest* request) {
    log("Load custom path \"%s\"\n", path);

/*  TODO: If this fails, try last argument as 1 */
    int bytesRead = 0;
    int result = MCP_DoLoadFile(path, NULL, msg->ioctl.buffer_io, msg->ioctl.length_io, request->pos, &bytesRead, 0);
    log("MCP_DoLoadFile returned %d, bytesRead = %d\n", result, bytesRead);

    if (result >= 0) {
        if (!bytesRead) {
            return 0;
        }

    /*  TODO: If this fails, try last argument as 1 */
        result = MCP_UnknownStuff(path, request->pos, msg->ioctl.buffer_io, msg->ioctl.length_io, msg->ioctl.length_io, 0);
        log("MCP_UnknownStuff returned %d\n", result);

        if (result >= 0) {
            return bytesRead;
        }
    }
    return result;
}

/*  RPX replacement! Call this ioctl to replace the next loaded RPX with an arbitrary path.
    DO NOT RETURN 0, this affects the codepaths back in the IOSU code */
int _MCP_ioctl100_patch(ipcmessage* msg) {
/*  Give some method to detect this ioctl's prescence, even if the other args are bad */
    if (msg->ioctl.buffer_io && msg->ioctl.length_io >= sizeof(u32)) {
        *(u32*)msg->ioctl.buffer_io = 1;
    }

    FAIL_ON(!msg->ioctl.buffer_in, 0);
    FAIL_ON(!msg->ioctl.length_in, 0);
    FAIL_ON(msg->ioctl.length_in > sizeof(rpxpath) - 1, msg->ioctl.length_in);

    strncpy(rpxpath, (const char*)msg->ioctl.buffer_in, sizeof(rpxpath) - 1);
    rpxpath[sizeof(rpxpath) - 1] = '\0';

    replacerpx = true;
    didrpxfirstchunk = false;

    log("Will load %s for next title\n", rpxpath);

/*  Signal that all went well */
    if (msg->ioctl.buffer_io && msg->ioctl.length_io >= sizeof(u32)) {
        *(u32*)msg->ioctl.buffer_io = 2;
    }
    return 1;
}
