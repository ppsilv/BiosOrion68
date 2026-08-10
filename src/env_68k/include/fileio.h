#include "fatfs/ff.h"

FRESULT fopen(FIL* fp, const TCHAR* path, BYTE mode);
FRESULT fclose(FIL* fp);
FRESULT fread(FIL* fp, void* buff, UINT btr, UINT* br);
FRESULT fwrite(FIL* fp, const void* buff, UINT btw, UINT* bw);
FRESULT fsync(FIL* fp);


FRESULT flseek(FIL* fp, FSIZE_t offset);
FSIZE_t ftell(FIL* fp);
FSIZE_t fsize(FIL* fp);
int feof(FIL* fp);


FRESULT funlink(const TCHAR* path);
FRESULT frename(const TCHAR* path_old, const TCHAR* path_new);
FRESULT fstat(const TCHAR* path, FILINFO* fno);

FRESULT fopendir(DIR* dp, const TCHAR* path);
FRESULT freaddir(DIR* dp, FILINFO* fno);
FRESULT fclosedir(DIR* dp);
FRESULT fmkdir(const TCHAR* path);

FRESULT fmount(FATFS* fs, const TCHAR* path, BYTE opt);
FRESULT funmount(const TCHAR* path):
