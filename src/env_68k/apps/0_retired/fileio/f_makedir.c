#include "fatfs/ff.h"

FRESULT fmkdir(const TCHAR* path){
    return f_mkdir( path);
}
