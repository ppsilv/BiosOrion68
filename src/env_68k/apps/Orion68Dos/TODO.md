# TODO:

# 1 - Um meio de enviar arquivo para o OrionDOS.------> OK
    22-07-2026: O arquivo já está indo para 0x82000 falta agora
                gravar o programa no disco.------------------------------> OK
    18-07-2026: quase lá já consigo enviar um arquivo do pc para
                o picow via tcp-ip. Agora falta testar a interface
                entre o pico e o Orion68DOS.-----------------------------> OK
# 2 - Desenhar a interface----------------------------> 
    TRAP #XX------------------------------------------>trap12 disco-> OK trap1 console->OK
    libc----------------------------------------------> OK
    libfileIO fopen,fread,fwrite...-------------------> OK
    libSerial-----------------------------------------> trap12 --> testar fortemente
    libParallel--------------------------------------->
    libRtc--------------------------------------------> trap4 --> testar fortemente
    libVideo acesso as funções da interface de video--> trap2 --> testar fortemente
# 3 - Validar interruções.----------------------------> OK
    systick ------------------------------------------> OK
    teclado ps2---------------------------------------> OK
    teclado USB---------------------------------------> OK
    SuperMultiIO--------------------------------------> OK
# 4 - Implementar wiznet--------------------------------> Aguardando implementação hardware
# 5 - Implementar pi picoW:---------------------------> OK
    interface com o m68k------------------------------> OK
    leitura do keyboard USB---------------------------> OK
    leitura do keyboard PS2---------------------------> OK
    leitura sdcard------------------------------------> OK
# 5.1-Inteface IDE------------------------------------> 
    Implementar---------------------------------------> OK
    Implementar fatfs---------------------------------> OK
    Implementar get_fattime para fatfs---------------->
# 6 - Validar a verificação de quantidade de memoria.-> OK
    total de memoria inicial 0x100000 ----------------> OK
    Inserido mais 0x100000 total 2Mb------------------> OK
# 7 - RTC --------------------------------------------> OK
    Implementar hardware testar acesso----------------> OK
    leitura data/hora---------------------------------> OK
    escrita data/hora---------------------------------> OK
# 8 - TinyBasic---------------------------------------> OK
    Implementar---------------------------------------> OK
    Verificar as função save e load do tinybasic------> OK
# 9 - Duart-------------------------------------------> OK
    Implementar o hardware----------------------------> OK
    Led piscante para sinalizar cpu ON----------------> OK
    Testar serial escrita-----------------------------> OK
    Testar serial leitura-----------------------------> OK
# 10 -PIO - Parallel Input/Output--------------------->
    Implementar o hardware --------------------------->
    Testar escrita nas portas------------------------->
    Testar leitura nas portas------------------------->
# 11 -SDCARD
    Implementar fasfs para o sdcard------------------->
# 12 -PicoVGA ----------------------------------------> 
    Refatoração com melhoria na interface com m68k----> OK
    Implementar uma biblioteca grafica---------------->
# 13 - AT24C32 --------------------------------------->
    Implementar uma forma de ler essa memoria
    Uma vez que o no arduino li a memoria mas no m68k
    com bitbang não funciona, sem motivo aparente,
    já testei retestei verifiquei codigo hardware e
    de forma alguma consegui ler ou escrever na memoria
    com sucesso.
# 14 - Implementar interrupção vetorada -------------->  
    Usando um GAL22V10 implementar uma interrupção---->
    vetorada para ampliar o leque de opções de 
    interrupção.

# LOG ERROS:


