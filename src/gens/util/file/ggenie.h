/* Header file for decode functions */
#ifndef GENS_GGENIE_H
#define GENS_GGENIE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "emulator/gens.h"

struct patch { unsigned int addr, data; };

struct GG_Code
{
	char code[16];
	char name[240];
	unsigned int active;
	unsigned int restore;
	unsigned int addr;
	unsigned short data;
};

extern struct GG_Code Liste_GG[256];
extern char Patch_Dir[GENS_PATH_MAX];

void Init_GameGenie(void);
int Load_Patch_File(void);
int Save_Patch_File(void);
void decode(const char* code, struct patch *result);
void Patch_Codes(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_GGENIE_H */
