#include "fatfs/ff.h"

FRESULT flseek(FIL* fp, FSIZE_t offset){
    return f_lseek( fp, offset);
}
