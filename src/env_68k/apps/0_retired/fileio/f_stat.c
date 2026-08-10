#include "fatfs/ff.h"

FRESULT fstat(const TCHAR* path, FILINFO* fno){
    return f_stat(path, fno);
}

