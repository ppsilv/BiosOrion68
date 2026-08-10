#include "fatfs/ff.h"

FRESULT fmount(FATFS* fs, const TCHAR* path, BYTE opt){
    return f_mount(fs,  path, opt);
}

