#include "fatfs/ff.h"

FRESULT fopen(FIL* fp, const TCHAR* path, BYTE mode){
    return f_open( fp,   path, mode);
}
