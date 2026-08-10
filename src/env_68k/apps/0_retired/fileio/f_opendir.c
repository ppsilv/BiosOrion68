#include "fatfs/ff.h"

FRESULT fopendir(DIR* dp, const TCHAR* path){
    return f_opendir( dp, path);
}
