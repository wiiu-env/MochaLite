//Main.h
#ifndef _MAIN_H_
#define _MAIN_H_


/* Main */
#ifdef __cplusplus
extern "C" {
#endif


#define OSDynLoad_Acquire ((void (*)(char* rpl, unsigned int *handle))0x0102A3B4)
#define OSDynLoad_FindExport ((void (*)(unsigned int handle, int isdata, char *symbol, void *address))0x0102B828)

//! C wrapper for our C++ functions
int Menu_Main(void);

#ifdef __cplusplus
}
#endif

#endif
