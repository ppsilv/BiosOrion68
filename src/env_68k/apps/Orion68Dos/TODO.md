# TODO:

# 1 - Um meio de enviar arquivo para o OrionDOS.
    22-07-2026: O arquivo já está indo para 0x82000 falta agora
                gravar o programa no disco.------------------------------> OK
    18-07-2026: quase lá já consigo enviar um arquivo do pc para
                o picow via tcp-ip. Agora falta testar a interface
                entre o pico e o Orion68DOS.-----------------------------> OK
# 2 - Desenhar a interface
    TRAP #XX------------------------------------------>trap12 disco-> OK trap1 console->OK
    libc----------------------------------------------> OK
    libfileIO fopen,fread,fwrite...-------------------> OK
    libSerial----------------------------------------->
    libParallel--------------------------------------->
    libRtc-------------------------------------------->
    libVideo acesso as funções da interface de video-->
# 3 - Validar interruções.
    systick ------------------------------------------> OK
    teclado ps2---------------------------------------> OK
    teclado USB---------------------------------------> OK
    SuperMultiIO--------------------------------------> OK
# 4 - Implementar wiznet--------------------------------> Aguardando implementação hardware
# 5 - Implementar pi picoW:
    interface com o m68k------------------------------> OK
    leitura do keyboard USB---------------------------> OK
    leitura do keyboard PS2---------------------------> OK
    leitura sdcard------------------------------------> OK
# 5.1-Inteface IDE
    Implementar---------------------------------------> OK
    Implementar fatfs---------------------------------> OK
# 6 - Validar a verificação de quantidade de memoria.
    total de memoria inicial 0x100000 ----------------> OK
    Inserido mais 0x100000 total 2Mb------------------> OK
# 7 - RTC
    Implementar hardware testar acesso----------------> OK
    leitura data/hora--------------------------------->
    escrita data/hora--------------------------------->
# 8 - TinyBasic
    Implementar---------------------------------------> OK
    Verificar as função save e load do tinybasic------>
# 9 - Duart
    Implementar o hardware----------------------------> OK
    Led piscante para sinalizar cpu ON----------------> OK
    Testar serial escrita----------------------------->
    Testar serial leitura----------------------------->
# 10 -PIO - Parallel Input/Output
    Implementar o hardware --------------------------->
    Testar escrita nas portas------------------------->
    Testar leitura nas portas------------------------->
# 11 -SDCARD
    Implementar fasfs para o sdcard------------------->
# 12 -PicoVGA
    Refatoração com melhoria na interface com m68k----> OK
    Implementar uma biblioteca grafica---------------->


# LOG ERROS:
