#include "fatfs/ff.h"

FRESULT funlink(const TCHAR* path){
    return f_unlink(path);
}
