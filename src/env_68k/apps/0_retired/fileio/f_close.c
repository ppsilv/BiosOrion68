#include "fatfs/ff.h"

FRESULT fclose(FIL* fp){
    return f_close(  fp);
}
