#include "fatfs/ff.h"

FRESULT fclosedir(DIR* dp){
    return f_closedir( dp);
}
