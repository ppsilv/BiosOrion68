# 2026-08-11
        O modulo grafico vga_graphics.c está com alguns problemas.
        Problema 1: a lib não chega até o pico tem algum erro o unico debug que aparece é:
                        printf("VGA_Video: x0[%d] y0[%d] x1[%d] x1[%d] col[%d]\n",x0,y0,x1,y1,color);
                     tinha varios erros
                     1 - d1 não estava sendo populado.
                     2 - d1 não estava sendo enviado.
                     3 - um printf de debug colocado depois da população dos registradores 
                         estava matando o conteudo dos registradores.   
        Problema 2: a função drawLine no pico não está desenhando nada, testei com o poke do basic 
                        enviando cada valor individualmente e o print de debug do comando   CMD_DRAW_LINE dentro
                        do pico imprimiu  PICO: x0[20] y0[10] x1[40] x1[10] col[5]\n"           
                     1 - nesse caso não era ero esses valores mostram uma linha misnuscula na tela
                         que eu não estava vendo
        AVISO:  RETIRAR OS DEBUGS NO VGA_VIDEO e no PICO--> Feito-->OK                               
# 2026-08-10
        A transferencia de arquivos entre o pico2w e o m68k estava demorando 1ms por byte
        quando em minhas contas deveria ser 6ms por byte
        Consultando a IA claude, ela lembrou do código:
        for (uint16_t i = 0; i < tamanho_arquivo; i++) {
                destino_ram[i] = PICO_DATA_REG;
                //_delay_ms();
        }
        porque ela fez o calculo ao contrário um arquivo de 11101bytes em 11s que eu cronometrei
        e não usei o DADO, mas a ia usou e lembrou que eu dava justamente um delai e 1ms entre
        cada leitura, esse delay é remanescente de antes de eu mudar a forma de gerar o /DTACK
        para a cpu, depois que acertei o systema /DTACK o pico é quem responde esse sinal e não o
        sistema automático de /DTACK.
# 2026-08-08
        As vezes o computador não entra no ar e dá bus error, led verde fica aceso
        precionando o chip scn68681 tem resolvido.
        TODO: PESQUISAR ESSE DEFEITO PARA ACHAR O CAUSADOR.
              Depois de ver o problema abaixo o 001, acho que e esse problema parece
              ser igual, pois no problema 001 se resolvia quando pressionava o pico.
              estou pensado em trocar o soquete do 68681 para um soquete estampado.

        Problema 001: O pico parou de ler o teclado e enviar para o m68k
                      o pino 21 do soquete TORNEADO do pico2W apresentou problemas quando o
                      tirei verifiquei que ele parecia um tunel e o pino do pico2w
                      um fusquinha passando nesse pino, toquei esse pino e tudo voltou
                      ao normal. SOQUETE TORNEADO NUNCAMAIS

# 2026-08-07
        Quando adicionei o scn68681 à placa MultiIO, cometi os seguinte erros.
        1 - O endereço base não foi atualizado, atualizei mas ainda não funcionou
        2 - Endereços de offset dos registradores,estavam pares, trazidos do rosco, eu uso IO em D0-D7
# 2026-08-04
        Acredito que esse problema intermitente veio mudando de causa ao longo do projeto, como nunca trabalhei
        com o m68k que parece ser muito mais chato que um 8088,8086 e z80, eu tive problemas que fui resolvendo
        e por isso o travamento sumia mas depois reaparecia porque eu o criava de outra forma essa última vez e há
        muito tempo acredito eu que o problema era o mesmo um codigo deixado na gal do picoVGA, que sempre atacava
        a serial somente a serial, e acredito que seja porque o sinal que estava sendo usado na gal de forma errada
        só afetava a serial. LDS no pino 12 que é uma saida, sei que pode ser entrada porém para ser entrada tem que
        ser usado como entrada em uma equação, e nesse caso eu não usei ele ficou como saida e zoando o barramento.

# 2026-08-02
        Aquele problema atacou novamente, de uma hora para outra a placa simplesmente para de responder.
        Eu fixei o SP stack pointer em 0xFFFF0 e aparentemente resolveu, falo aparentemente porque estou sem vídeo e aquele velho problema da serial continua agindo de vez em quando ela imprime a mensagem do bootstrap mas do programa C imprime caractere lixo. Estou para trocar essa 16C554 pela 16C550 mas esse é outro erro.
        Então dessa vez parece que a memoria ram superior não está bom o aceso porque os primeiros 512 respondeu na hora.
        TODO: Implementar teste de memoria boa ou ruim.

# 2026-07-21
Está tudo funcionando agora, o dado pedido é o dado recebido. SIMPLES ASSIM. o problema
todo era a dupla leitura antiga e obsoleta do m68k uma vez que eu não ficava olhando para
o codigo dele, resolveu isso foi fácil achar o problema de sincronismo.
Agora finalmente desmascarado o problema, lembra que ontem falei 2 fantasmas e que 1 a
gente achou mas o outro estava escondido?
O fantasma escondido era a segunda leitura feita pelo m68k que no passado fazia sentido
e o sentido se perdeu e ela ficou la perturbando, no futuro observar o código como um todo
não só no lugar onde achamos que está o problema.
Pontuando:
1 - uma leitura de estatus que não mais era usada, ela não deveria causar erro mas escodia
o verdadeiro erro pois eu achava que existia uma leitura escondida
2 - o dtack não estava no momento correto, quando ele ocorria o dado nem estava lá ainda ou
seja o m68k estava mais rápido que o pico2w.
Lembrando uma leitura de I/O pelo m68k
/DTACK -> 1
/AS -> 0
/CS -> 0
/wr -> 0 ou 1 conforme a leitura ou escrita
pico trabalha
pico fica aguardando /CS=1
/DTACK -> 0   m68k lê
/AS -> 1
/CS -> 1
pico tira os dados põe bus em 3state
pico aguarda novo comando.

# 2026-07-06
Hoje eu decobri que muitos problemas iniciais com o endiam na leitura do MIDE era porque
estava tratando os dados como lidos em 16bits, mas na verdade tanto a função
ata_read_sector e ata_write_sector estavam configurando como leitura de 8bits, até que
eu achei que o MIDE vem de fábrica assim em 8bitse ai tudo começou a funcionar,
agora criei uma função para por em 8 ou 16bits.

# 2026-07-05
Problema: A fatfs cria o diretorio mas ele não aparece na lista dir.

levei o disco para um PC e ele leu criou diretorios normalmente e eu criei diretórios com os mesmos nomes que estava tentando criar com o m68k, quando voltei o disco para o m68k leu a arvore de diretorios normalmente com os novos diretorios aparecendo, e na primeira vez que li o setor 66 só tinha 3 entradas de diretorio, quando tentei criar um novo diretorio eu pensei como o que ele cria não aparece vou tentar criar com o mesmo nome, mas a fatfs não deixa dá erro 8, não é bem oque eu esperava mas não cria, tentei varios dos novos que criei no disco usando os PC, não deixou criar ai tentei criar um novo ele disse que criou e novamente não apareceu no comando dir, mas aconteceu um fato novo, que agora dá para entender oque está acontecendo, ele tem uma janela de somente 1 diretorio então ele acha o ultimo setor com as entradas de diretorio põe na memoria direitinho, porém na hora de gravar não sei porque ele grava no setor 66 que com certeza não é o setor que ele leu, por isso tudo que ele cria não aparece na lista de diretorio. entendeu se não entendeu pergunte.
SOLVED as 18:00

     sector_t physical_sector = drives[0].parts[0].base + sector; <--------------FALTAVA ISSO NA disk_write ele gravava sector e não physical_sector
    printf(" physical_sector[%ld]\n",physical_sector);

    // Grava quantos setores a FatFs pedir (suportando o count)
    for (UINT i = 0; i < count; i++) {
        if (!ata_write_sector(physical_sector + i, buff + (i * 512))) {
    }


# 2026/06/16
Fase 1: O dump travava de forma inconsistente por culpa do loop de
        boot e da "amnésia" da UART 2.

Fase 2: O dump passou a funcionar, mas precisava de um delay artificial
        por caractere para não atropelar a concorrência das protothreads
        no Pico.

Fase 3: Migração para Dual-Core real. O barramento ficou livre, mas a
        FIFO de hardware de 8 posições virou o gargalo.

Fase 4 (Estado da Arte): Implementação da queue_t na RAM isolando os
        núcleos e Overclock síncrono para 300 MHz casando perfeitamente
        com o Pixel Clock do PIO.



# Log de erro: 2026-06-24

Os travamentos do sistema, quando for realmente um fato estará assinalado com *FATO

CPU utilizada é o MC68000P12F que o manual diz ser 16Mhz
E ele realmente rodou muito tempo em 16Mhz.

Detectei os seguintes fatos:

OBS.: Todas as conclusões foram feitas depois da versão 2.2,por isso a numerção que
segue a ordem cronológica não faz muito sentido.

1 - Codigo vbug2.0 totalmente em assembler rodando a 16Mhz.
    - travou por algums motivos de hardware sujeira de solta entre os pinos e wire-up.
    - *FATO: travou por excesso de velociade para o hardware, detectado muito mais tarde,
             depois de ter sem querer mudado o clock para 8Mhz.

2 - Codigo vbug2.1 em C e assembler rodando a 8Mhz.
    - *FATO: travou por excesso de velociade para o hardware.
    - *FATO: roda bem com clock 4Mhz e dtack em 2Mhz.
    Em 24/06/2026 coloquei em 8Mhz e funcinou sem problemas aparentes.Em análise...


3 - Codigo vbug2.2 totalmente em assembler rodando a 8Mhz.
    - Tentativa de fazer um codigo muito mais limpo depois da experiência adquirida.
    - Travou a 8Mhz sem nenhum motivo aparente.




