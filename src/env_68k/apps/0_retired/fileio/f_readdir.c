#include "fatfs/ff.h"

FRESULT freaddir(DIR* dp, FILINFO* fno){
    return f_readdir( dp, fno);
}
