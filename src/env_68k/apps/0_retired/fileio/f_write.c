#include "fatfs/ff.h"

FRESULT fwrite(FIL* fp, const void* buff, UINT btw, UINT* bw){
    return f_write(fp, buff, btw, bw);
}
