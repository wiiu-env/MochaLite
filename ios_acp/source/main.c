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
#include "imports.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "../../common/ipc_defs.h"
#include "svc.h"

#define DEBUG_FUNCTION_LINE(FMT, ARGS...)do { \
    log_printf("[%23s]%30s@L%04d: " FMT "",__FILE__,__FUNCTION__, __LINE__, ## ARGS); \
    } while (0)


static void* allocIobuf(int size) {
    void* ptr = svcAlloc(0xCAFF, size);

    memset(ptr, 0x00, size);

    return ptr;
}

static void freeIobuf(void* ptr) {
    svcFree(0xCAFF, ptr);
}


// probably painfully inefficient, but enough for debugging.
void log_printf(const char * format, ...) {
    int fd = svcOpen("/dev/mcp", 0);
    if(fd >= 0) {
        u8* iobuf = allocIobuf(0x100 + 0x04 + 0x04); // str + command + out
        if(iobuf != NULL) {
            u32* inbuf = (u32*)iobuf;
            u32* outbuf = (u32*)&iobuf[(0x100 + 0x04)]; // str + command

            va_list args;
            va_start(args, format);

            inbuf[0] = IPC_CUSTOM_LOG_STRING; // set command

            int len = vsnprintf((char *) &inbuf[0x04 / 0x04], 0x100, format, args);
            if(len > 0) {
                inbuf[len] = 0; // null terminator
                svcIoctl(fd, 0x64, inbuf, 0x100 + 0x04, outbuf, 4);
            }

            va_end(args);

            freeIobuf(iobuf);
        }
        svcClose(fd);
    }
    return;
}

bool isSwapRequired() {
    int fd = svcOpen("/dev/mcp", 0);
    int res = false;
    if(fd >= 0) {
        u8* iobuf = allocIobuf(0x08);
        if(iobuf != NULL) {
            u32* inbuf = (u32*)iobuf;
            u32* outbuf = (u32*)&iobuf[0x04]; // command

            inbuf[0] = IPC_CUSTOM_META_XML_SWAP_REQUIRED; // set command

            svcIoctl(fd, 0x64, inbuf, 0x04, outbuf, 4);

            if(outbuf[0] == 10) {
                res = true;
            }

            freeIobuf(iobuf);
        }

        svcClose(fd);
    }
    return res;
}

bool getMetaXML(ACPMetaXml * ptr) {
    int fd = svcOpen("/dev/mcp", 0);
    int res = false;
    if(fd >= 0) {
        u8* iobuf = allocIobuf(0x04 + sizeof(ACPMetaXml));
        if(iobuf != NULL) {
            u32* inbuf = (u32*)iobuf;
            u32* outbuf = (u32*)&iobuf[0x04]; // command

            inbuf[0] = IPC_CUSTOM_META_XML_READ; // set command

            memcpy(outbuf, ptr, sizeof(ACPMetaXml));

            svcIoctl(fd, 0x64, inbuf, 0x04, outbuf, sizeof(ACPMetaXml));

            memcpy(ptr,outbuf, sizeof(ACPMetaXml));

            freeIobuf(iobuf);
        }

        svcClose(fd);
    }
    return res;
}

void dumpHex(const void* data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    DEBUG_FUNCTION_LINE("0x%08X (0x0000): ", data);
    for (i = 0; i < size; ++i) {
        log_printf("%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            log_printf(" ");
            if ((i+1) % 16 == 0) {
                log_printf("|  %s \n", ascii);
                if(i + 1 < size) {
                    DEBUG_FUNCTION_LINE("0x%08X (0x%04X); ", data + i + 1,i+1);
                }
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    log_printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    log_printf("   ");
                }
                log_printf("|  %s \n", ascii);
            }
        }
    }
}



int ACP_ParseMetaXML(ACPMetaXml *param_1,char *data,int filesize) {
    int (*real_ACP_ParseMetaXML)(ACPMetaXml *param_1,char *data,int filesize) = (void*)0xe0012e38;
    int res = real_ACP_ParseMetaXML(param_1,data,filesize);

    return res;
}

int ACP_LoadMetaXmlFromPath(char * param_1,uint32_t param_2) {
    int (*real_ACP_LoadMetaXmlFromPath)(char * param_1,uint32_t param_2) = (void*)0xe0029f1c;
    int res = real_ACP_LoadMetaXmlFromPath(param_1,param_2);
    ACPMetaXml *xml_data = (ACPMetaXml *) ((uint32_t)param_1 + 0xe8);
    //DEBUG_FUNCTION_LINE("%08X %08X \n", param_1, param_2);
    if(param_1 != NULL){
        if(isSwapRequired()) {
            getMetaXML(xml_data);
            xml_data->e_manual = 0;
            //DEBUG_FUNCTION_LINE("Do swap %s \n", xml_data->longname_en);
        } else {
            DEBUG_FUNCTION_LINE("We want no swap! %s\n", xml_data->longname_en);
        }
    }

    return res;
}

