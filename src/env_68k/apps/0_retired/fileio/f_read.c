#include "fatfs/ff.h"

FRESULT fread(FIL* fp, void* buff, UINT btr, UINT* br){
    return f_read( fp, buff, btr,  br);
}

