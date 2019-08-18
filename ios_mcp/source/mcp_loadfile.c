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
#include "fsa.h"
#include "svc.h"
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
static bool skipPPCSetup = false;
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

    /*if (request->type == LOAD_FILE_CAFE_OS &&
        request->name[0] == '*') {
        char path[0x40];

        // Translate request->name to a path by replacing * with / 
        for (int i = 0; i < 0x40; ++i) {
            if (request->name[i] == '*') {
                path[i] = '/';
            } else {
                path[i] = request->name[i];
            }
        }

        int result = MCP_LoadCustomFile(path, msg, request);
        if (result >= 0) return result;
    }*/
    /*  RPX replacement!
      
        The goal here is only to replace an rpx once. Reading at pos = 0 signifies a
        new rpx load - these conditions detect that. */
        
    if (request->name[0] == 'm' &&
        request->name[1] == 'e' &&
        request->name[2] == 'n' &&
        request->name[3] == '.' &&
        request->name[4] == 'r' &&
        request->name[5] == 'p' &&
        request->name[6] == 'x'
        && rpxpath[0] != 'd'
        && !skipPPCSetup){
            int fsa_h = svcOpen("/dev/fsa", 0);
            FSA_Unmount(fsa_h, "/vol/storage_iosu_homebrew", 2);
            FSA_Mount(fsa_h, "/dev/sdcard01", "/vol/storage_iosu_homebrew", 2, NULL, 0);
            svcClose(fsa_h);
            
            char * f_path = "/vol/storage_iosu_homebrew/wiiu/root.rpx";;
            
            int result = MCP_LoadCustomFile(f_path, msg, request);
            
            if (result >= 0) {                
                return result;
            }            
        }else if (request->name[0] == 's' &&
        request->name[1] == 'a' &&
        request->name[2] == 'f' &&
        request->name[3] == 'e' &&
        request->name[4] == '.' &&
        request->name[5] == 'r' &&
        request->name[6] == 'p' &&
        request->name[7] == 'x'){
            
        char * final_path = rpxpath;
            
        if(rpxpath[0] == '\0') {
            final_path = "/vol/storage_iosu_homebrew/wiiu/apps/homebrew_launcher/homebrew_launcher.rpx";
            if (request->pos == 0){
                didrpxfirstchunk = false;
            }
        }

        char* extension = request->name + strlen(request->name) - 4;
        if( extension[0] == '.' &&
            extension[1] == 'r' &&
            extension[2] == 'p' &&
            extension[3] == 'x') {
             if(final_path != NULL){
                if (!didrpxfirstchunk || request->pos > 0) {
                    int fsa_h = svcOpen("/dev/fsa", 0);
                    FSA_Unmount(fsa_h, "/vol/storage_iosu_homebrew", 2);
                    FSA_Mount(fsa_h, "/dev/sdcard01", "/vol/storage_iosu_homebrew", 2, NULL, 0);
                    svcClose(fsa_h);
                    
                    int result = MCP_LoadCustomFile(final_path, msg, request);
                    
                    rpxpath[0] = '\0';
                    if (result >= 0) {
                        if (request->pos == 0) didrpxfirstchunk = true;
                        return result;
                    }
                }
            }
        }
    }
    if (rpxpath[0] == 'd' &&
        rpxpath[1] == 'o' &&
        rpxpath[2] == 'n' &&
        rpxpath[3] == 'e'){
        skipPPCSetup = true;
        rpxpath[0] = '\0';
    }
    
    
    return real_MCP_LoadFile(msg);
}

static int MCP_LoadCustomFile(char* path, ipcmessage* msg, MCPLoadFileRequest* request) {
    log("Load custom path \"%s\"\n", path);
    
    int filesize = 0;
    int fileoffset = 0;
        
    if(filesize > 0 && (request->pos + fileoffset >  filesize)){
        return 0;
    }
    
    /*  TODO: If this fails, try last argument as 1 */
    int bytesRead = 0;
    int result = MCP_DoLoadFile(path, NULL, msg->ioctl.buffer_io, msg->ioctl.length_io, request->pos + fileoffset, &bytesRead, 0);
    log("MCP_DoLoadFile returned %d, bytesRead = %d pos %d \n", result, bytesRead, request->pos + fileoffset);
    

    if (result >= 0) {
        if (!bytesRead) {
            return 0;
        }

    /*  TODO: If this fails, try last argument as 1 */
        result = MCP_UnknownStuff(path, request->pos + fileoffset, msg->ioctl.buffer_io, msg->ioctl.length_io, msg->ioctl.length_io, 0);
        log("MCP_UnknownStuff returned %d\n", result);

        if (result >= 0) {
            if(filesize > 0 && (bytesRead + request->pos > filesize)){
                return filesize - request->pos;
            }
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
    
    memset(rpxpath,0,sizeof(rpxpath));
    strncpy(rpxpath, (const char*)msg->ioctl.buffer_in, msg->ioctl.length_in);
    //rpxpath[strlen(rpxpath)] = '\0';

    didrpxfirstchunk = false;

    log("Will load %s for next title\n", rpxpath);

/*  Signal that all went well */
    if (msg->ioctl.buffer_io && msg->ioctl.length_io >= sizeof(u32)) {
        *(u32*)msg->ioctl.buffer_io = 2;
    }
    return 1;
}
