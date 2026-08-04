

#include "hardware/watchdog.h"

void reboot_pico() {
    // 0 = reset imediato, o segundo parâmetro é se deve entrar em modo de bootloader (false = normal)
    watchdog_reboot(0, 0, 0);
}
