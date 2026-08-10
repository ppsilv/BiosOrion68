#include "fatfs/ff.h"

FRESULT fsync(FIL* fp){
    return f_sync(fp);
}


