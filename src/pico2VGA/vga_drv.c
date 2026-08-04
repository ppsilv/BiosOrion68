#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "vga320p_hsync.pio.h"
#include "vga320p_vsync.pio.h"
#include "vga320p_rgb.pio.h"
#include "vga640p_hsync.pio.h"
#include "vga640p_vsync.pio.h"
#include "vga640p_rgb.pio.h"
#include "orion_bus.pio.h"
#include "vga_drv.h"


// Give the I/O pins that we're using some names that make sense - usable in main()
//enum vga_pins {BUS=0,HSYNC=19, VSYNC=20, RED_PIN=21, LO_GRN=22, BLUE_PIN=26} ;
enum vga_pins {BUS=0,HSYNC=16, VSYNC=17, RED_PIN=20, LO_GRN=19, BLUE_PIN=18} ;

#define H_ACTIVE_1   327    // (active + frontporch - 1) - one cycle delay for mov
#define V_ACTIVE_1   479    // (active - 1)
#define H_ACTIVE_2   655    // (active + frontporch - 1) - one cycle delay for mov
#define V_ACTIVE_2   479    // (active - 1)

#define RGB_ACTIVE_1 159    // (horizontal active)/2 - 1
#define RGB_ACTIVE_2 319    // (horizontal active)/2 - 1

const PIO mpio0 = pio0;
const PIO mpio1 = pio1;
PIO bus_pio0=mpio0;
PIO bus_pio1=mpio1;
uint bus_sm;
// Manually select a few state machines from pio instance pio0.
uint hsync_sm = 0;
uint vsync_sm = 1;
uint rgb_sm = 2;

static void initDMA(char **active_buffer_ptr,unsigned int totalBytes,PIO pio);
static void initPio0(screenMode_t mode);
static void initDMA_320x200(char **active_buffer_ptr, PIO pio, uint sm);


void initReadBus_Pio()
{
    uint bus_read_offset = pio_add_program(mpio1, &orion_bus_program);

    uint bus_read_sm = 0;
    bus_sm = bus_read_sm;

    pio_sm_claim (mpio1, bus_read_sm);

    //bus_read_program_init(mpio1, bus_read_sm, bus_read_offset, BUS);
    orion_bus_program_init(mpio1, bus_read_sm, bus_read_offset) ;
}
static void initPio_320p();
static void initPio_640p();
#include "vga_primitives.h"

void pioReset(){

// 1. Para as 3 SMs simultaneamente para não perder o sincronismo
pio_set_sm_mask_enabled(mpio0, (1u << hsync_sm) | (1u << vsync_sm) | (1u << rgb_sm), false);

// 2. Remove os programas da memória de instrução (Libera os 32 slots)
// Isso é vital se os programas do modo novo forem diferentes
pio_clear_instruction_memory(mpio0);

// 3. Reseta o estado interno (registradores X, Y, PC e FIFOs)
pio_sm_restart(mpio0, hsync_sm);
pio_sm_restart(mpio0, vsync_sm);
pio_sm_restart(mpio0, rgb_sm);

// 4. Limpa os FIFOs para garantir que não sobrou lixo do modo anterior
pio_sm_clear_fifos(mpio0, hsync_sm);
pio_sm_clear_fifos(mpio0, vsync_sm);
pio_sm_clear_fifos(mpio0, rgb_sm);

// 5. Opcional: Se você usou pio_sm_claim, libere os índices
// pio_sm_unclaim(mpio0, hsync_sm); ...
}

void dmaReset(int dma_chan ){
    // Substitua 'dma_chan' pela variável que guarda o seu canal de DMA
    dma_channel_abort(dma_chan); 

    // Opcional: Limpar as interrupções do DMA se você usar handlers
    dma_channel_acknowledge_irq0(dma_chan);
}

//ESTA FUNÇÃO ESTÁ OK PARA 320 TESTADO 26/03
void initVGA(  char **active_buffer_ptr,unsigned int totalBytes,screenMode_t mode) {
    pioReset();
    switch(mode){    
        case MODE_TEXT_40_S:
        case MODE_TEXT_40_F:
        case MODE_320x240:
            initPio_320p(mode);
            initDMA_320x200(active_buffer_ptr, bus_pio0, rgb_sm);
            break;
        case MODE_TEXT_80_S:
        case MODE_TEXT_80_F:
        case MODE_640x480:    
            initPio_640p(mode);
            initDMA(active_buffer_ptr,totalBytes,bus_pio0);
            break;
    }    

}
//ESTA FUNÇÃO ESTÁ OK PARA 320 TESTADO 26/03
static void initPio_320p(){
    uint hsync_offset = pio_add_program(mpio0, &hsync320p_program);
    uint vsync_offset = pio_add_program(mpio0, &vsync320p_program);
    uint rgb_offset = pio_add_program(mpio0, &rgb320p_program);
 
    pio_sm_claim (mpio0, hsync_sm);
    pio_sm_claim (mpio0, vsync_sm);
    pio_sm_claim (mpio0, rgb_sm);

    hsync320p_program_init(mpio0, hsync_sm, hsync_offset, HSYNC);
    vsync320p_program_init(mpio0, vsync_sm, vsync_offset, VSYNC);
    rgb320p_program_init(mpio0, rgb_sm, rgb_offset, BLUE_PIN);

    pio_sm_put_blocking(mpio0, hsync_sm, H_ACTIVE_1);
    pio_sm_put_blocking(mpio0, vsync_sm, V_ACTIVE_1);
    pio_sm_put_blocking(mpio0, rgb_sm, RGB_ACTIVE_1);

    pio_enable_sm_mask_in_sync(mpio0, ((1u << hsync_sm) | (1u << vsync_sm) | (1u << rgb_sm)));
}
static void initPio_640p(){
    uint hsync_offset = pio_add_program(mpio0, &hsync_program);
    uint vsync_offset = pio_add_program(mpio0, &vsync_program);
    uint rgb_offset = pio_add_program(mpio0, &rgb_program);
 
    pio_sm_claim (mpio0, hsync_sm);
    pio_sm_claim (mpio0, vsync_sm);
    pio_sm_claim (mpio0, rgb_sm);

    hsync_program_init(mpio0, hsync_sm, hsync_offset, HSYNC);
    vsync_program_init(mpio0, vsync_sm, vsync_offset, VSYNC);
    rgb_program_init(mpio0, rgb_sm, rgb_offset, BLUE_PIN);

    pio_sm_put_blocking(mpio0, hsync_sm, H_ACTIVE_2);
    pio_sm_put_blocking(mpio0, vsync_sm, V_ACTIVE_2);
    pio_sm_put_blocking(mpio0, rgb_sm, RGB_ACTIVE_2);

    pio_enable_sm_mask_in_sync(mpio0, ((1u << hsync_sm) | (1u << vsync_sm) | (1u << rgb_sm)));
}

int rgb_chan_0;
int rgb_chan_1;
static void initDMA(char **active_buffer_ptr,unsigned int totalBytes,PIO pio){

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // ============================== PIO DMA Channels =================================================
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    dmaReset(rgb_chan_0);
    dmaReset(rgb_chan_1);
    // DMA channels - 0 sends color data, 1 reconfigures and restarts 0
    rgb_chan_0 = dma_claim_unused_channel(true);
    rgb_chan_1 = dma_claim_unused_channel(true);


    // change this to true of other DMA channels interfer with video
    #define rgb_high_priority false

    // Channel Zero (sends color data to PIO VGA machine)
    dma_channel_config c0 = dma_channel_get_default_config(rgb_chan_0);  // default configs
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);              // 8-bit txfers
    channel_config_set_read_increment(&c0, true);                        // yes read incrementing
    channel_config_set_write_increment(&c0, false);                      // no write incrementing
    channel_config_set_dreq(&c0, DREQ_PIO0_TX2) ;                        // DREQ_PIO0_TX2 pacing (FIFO)
    channel_config_set_chain_to(&c0, rgb_chan_1);                        // chain to other channel
    channel_config_set_high_priority (&c0, rgb_high_priority) ;

    dma_channel_configure(
        rgb_chan_0,                 // Channel to be configured
        &c0,                        // The configuration we just created
        &pio->txf[rgb_sm],          // write address (RGB PIO TX FIFO)
        *active_buffer_ptr,             // The initial read address (pixel color array)
        totalBytes,                 // Number of transfers; in this case each is 1 byte.
        false                       // Don't start immediately.
    );

    // Channel One (reconfigures the first channel)
    dma_channel_config c1 = dma_channel_get_default_config(rgb_chan_1);   // default configs
    channel_config_set_transfer_data_size(&c1, DMA_SIZE_32);              // 32-bit txfers
    channel_config_set_read_increment(&c1, false);                        // no read incrementing
    channel_config_set_write_increment(&c1, false);                       // no write incrementing
    channel_config_set_chain_to(&c1, rgb_chan_0);                         // chain to other channel

    dma_channel_configure(
        rgb_chan_1,                         // Channel to be configured
        &c1,                                // The configuration we just created
        &dma_hw->ch[rgb_chan_0].read_addr,  // Write address (channel 0 read address)
        active_buffer_ptr,                    // Read address (POINTER TO AN ADDRESS)
        1,                                  // Number of transfers, in this case each is 4 byte
        false                               // Don't start immediately.
    );

    // Start DMA channel 0. Once started, the contents of the pixel color array
    // will be continously DMA's to the PIO machines that are driving the screen.
    // To change the contents of the screen, we need only change the contents
    // of that array.
    dma_start_channel_mask((1u << rgb_chan_0)) ;
}    

//320x200


// Variáveis globais para controle
int rgb_chan_0;
static uint16_t vga_line_counter = 0;
static bool vga_repeat_step = false;
char * vga_buffer_global;
//ESTA FUNÇÃO ESTÁ OK PARA 320 TESTADO 26/03
void initDMA_320x200(char **active_buffer_ptr, PIO pio, uint sm) {
    vga_buffer_global = *active_buffer_ptr;
    rgb_chan_0 = dma_claim_unused_channel(true);

    dmaReset(rgb_chan_0);

    dma_channel_config c0 = dma_channel_get_default_config(rgb_chan_0);
    channel_config_set_transfer_data_size(&c0, DMA_SIZE_8);   // Manda byte a byte
    channel_config_set_read_increment(&c0, true);             // Avança na memória
    channel_config_set_write_increment(&c0, false);           // Escreve sempre no mesmo lugar (PIO)
    
    // DREQ: O PIO dita o ritmo (só manda quando a FIFO do PIO tiver espaço)
    channel_config_set_dreq(&c0, pio_get_dreq(pio, sm, true));

    dma_channel_configure(
        rgb_chan_0,
        &c0,
        &pio->txf[sm],      // Destino: FIFO de dados do PIO
        active_buffer_ptr,             // Origem inicial
        160,                // TAMANHO DE UMA LINHA (320 pixels / 2)
        false               // NÃO começa ainda
    );
}
//ESTA FUNÇÃO ESTÁ OK PARA 320 TESTADO 26/03
void __not_in_flash_func(vga_line_handler)() {
    pio_interrupt_clear(mpio0, 1); 

    // Apenas reinicia o DMA de dados apontando para o endereço atual
    // O segredo é que o DMA agora é disparado pelo DREQ do PIO de vídeo
    // e a CPU só precisa garantir que o endereço mude a cada 2 linhas físicas.
    
    static int physical_line = 0;
    static int data_line = 0;

    // Dispara o DMA para a linha de dados atual
    dma_channel_set_read_addr(rgb_chan_0, &vga_buffer_global[data_line * 160], true);

    physical_line++;
    if (physical_line % 2 == 0) {
        data_line++; // Só avança a linha de dados a cada 2 linhas físicas
    }

    if (data_line >= 240) {
        data_line = 0;
        physical_line = 0;
    }
}
