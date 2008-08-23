/**
 * GENS: Configuration file handler.
 */

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#ifdef __cplusplus
extern "C" {
#endif


int Save_Config(const char *File_Name);
int Save_As_Config(void);
int Load_Config(const char *File_Name, void *Game_Active);
int Load_As_Config(void *Game_Active);


#ifdef __cplusplus
}
#endif

#endif
