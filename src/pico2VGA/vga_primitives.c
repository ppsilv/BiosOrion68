#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/dma.h"

#include "vga_primitives.h"
#include "font.h"

// 5x7 font
void writeStringBold(char* str);
static void inc_cursor_y();
static void inc_cursor_x();
static void dec_cursor_y();
static void dec_cursor_x();



/*
struct cursor{
    uint16_t x;
    uint16_t y;
    bool visible;
    bool blink;
    uint16_t blink_interval;
    cursorShape_t shape ;// 0 - block, 1 - underline, 2 - line
};
typedef struct cursor cursor_t;
*/
typedef struct  {
    uint16_t width;  //320, 640
    uint16_t height; //240, 480
    cursor_t *cursor ;
    screenMode_t video_mode ;
    font_t font ;
    color_t textcolor ;
    color_t textbgcolor ;

    uint32_t txcount;
    uint16_t topmask;
    uint16_t bottommask;
    uint8_t tabspace;    
    uint8_t* vga_data_array;    
}vga_text_private_t;

static vga_t * vga = NULL ;

void PANIC(){
  panic("Erro critico no sistema!");
  while(1){
    
  }
}
vga_t * get_vga(){
    return vga;
}
static void clrscr(){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

  memset(&priv->vga_data_array[0], 0, priv->txcount) ;
  // reset cursor position
  priv->cursor->x = 0 ;
  priv->cursor->y = 0 ;
}

static void dec_cursor_x(){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;


  priv->cursor->x -= priv->font.width;

  if(priv->cursor->x < 0){
    priv->cursor->x = 0;       
  }
}

static void dec_cursor_y(){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

  priv->cursor->y -= priv->font.height  ;
  if(priv->cursor->y < 0){
    priv->cursor->y = 0;       
  }
}

static void inc_cursor_x(){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

  priv->cursor->x += priv->font.width;
  if(priv->cursor->x > 79){
    priv->cursor->x = 0; 
    inc_cursor_y();      
  }
}

static void inc_cursor_y(){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

  priv->cursor->y += priv->font.height  ;
  if(priv->cursor->y > 30){
    priv->cursor->y = 0;       
  }
}


static void set_vga_data_array(uint8_t video_data_array[])
{
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  priv->vga_data_array = &video_data_array[0];  
} 

static void set_vga_home(void){
  vga->setTextCursorPos(0,0);
}

static void set_vga_mode(uint8_t mode){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  priv->video_mode = mode;
  switch(mode){
    case MODE_TEXT_40_S:
      priv->width = 320;
      priv->height= 240;
      break;
    case MODE_TEXT_40_F:
      priv->width = 320;
      priv->height= 240;
      break;
    case MODE_TEXT_80_S:
      priv->width = 640;
      priv->height= 480;
      break;
    case MODE_TEXT_80_F:
      priv->width = 640;
      priv->height= 480;
      break;
    case MODE_320x240:
      priv->width = 320;
      priv->height= 240;
      break;
    case MODE_640x480:
      priv->width = 640;
      priv->height= 480;
      break;
    default:
      priv->width = 640;
      priv->height= 480;
      priv->video_mode = MODE_TEXT_80_S;
      break;

  }
}


/*
static void pchar(char c){
    tft_write(c);
}*/


void drawPixel(short x, short y, color_t color) {
    vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
    if((x > (priv->width-1)) | (x < 0) | (y > (priv->height-1)) | (y < 0) ) return;

    int pixel = ((priv->width * y) + x) ;
    if (pixel & 1) {
        priv->vga_data_array[pixel>>1] = (priv->vga_data_array[pixel>>1] & priv->topmask) | (color << 4) ;
    }
    else {
        priv->vga_data_array[pixel>>1] = (priv->vga_data_array[pixel>>1] & priv->bottommask) | (color) ;
    }
}

void drawHLine(int x, int y, int w, color_t color) {
    vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  // range checks
  if((x >= priv->width) || (y >= priv->height)) return;
  if((x + w - 1) >= priv->width)  w = priv->width  - x - 1;
  //
  //short xx = x;
  short both_color = color | (color<<4) ;
  // loner pixel at x -- align left with next byte boundary
  if((x & 1)) {
    drawPixel(x,y,color);
    x++ ;
    w-- ;
  }
  // draw loner pixel at end and adjust width
  if((w & 1)){
    drawPixel(x+w-1, y, color);
    w-- ;
  }
  // draw rest of line
  int len = (w>>1)  ;
  if (len>0 && y<480 ) memset(&priv->vga_data_array[320*y+(x>>1)], both_color, len) ;
 }

void fillRect(short x, short y, short w, short h, color_t color) {
   vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
   if((y + h - 1) >= priv->height) h = priv->height - y - 1;

  for(int i=x; i <= w;i++)
      for( int j=y;j <= h;j++)
          drawPixel( i,  j, color ) ;

//  for(int j=y; j<(y+h); j++) {
//    drawHLine(x, j, w, color) ;
//  }
}

static void drawChar_interna(short x, short y, uint8_t c, color_t color, color_t bg, uint8_t size) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  char i, j;
  if((x >= priv->width)            || // Clip right
     (y >= priv->height)           || // Clip bottom
     ((x + 6 * size - 1) < 0) || // Clip left
     ((y + 8 * size - 1) < 0))   // Clip top
    return;

    // Calcula a área do caractere
    short charWidth = 8 * size;
    short charHeight = 8 * size;    
    // PRIMEIRO: Limpa a área completa do caractere
    fillRect(x, y, charWidth, charHeight, BLACK);

  uint8_t lastColumn = 5 ;
  uint8_t altura = 8 ;
  uint8_t largura = 6 ;
  for (i=0; i<largura; i++ ) {
    uint8_t line;
    if (i == lastColumn)
      line = 0x0;
    else
      line = pgm_read_byte(font_5x7+(c*lastColumn)+i);
    for ( j = 0; j < altura; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, color);
        else {  // big size
          fillRect(x+(i*size), y+(j*size), size, size, color);
        }
      } else if (bg != color) {
        if (size == 1) // default size
          drawPixel(x+i, y+j, bg);
        else {  // big size
          fillRect(x+i*size, y+j*size, size, size, bg);
        }
      }
      line >>= 1;
    }
  }
}

void drawChar2( int start_x, int start_y, uint8_t char_code, int color,  int bgcolor, uint8_t size) {
    vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

    // Calcula a área do caractere
    size=1;
    short charWidth = 8 * size;
    short charHeight = 16 * size;    
    // PRIMEIRO: Limpa a área completa do caractere
    fillRect(start_x, start_y, charWidth, charHeight, bgcolor);

    const uint8_t *char_start = font_8x16 + (char_code * priv->font.height);
    // Percorrer todas as linhas (bytes) do caractere
    for (int row = 0; row < priv->font.height; row++) {
        uint8_t current_byte = char_start[row];

        // Percorrer todos os bits do byte (da esquerda para a direita)
        for (int bit = 7; bit >= 0; bit--) {
            // Calcular coordenadas do pixel
            int pixel_x = start_x + (7 - bit);  // 7-bit para ir da esquerda para direita
            int pixel_y = start_y + row;
            // Verificar se o bit está setado (1)
            if ((current_byte >> bit) & 1) {
                // Desenhar o pixel
                if( size == 1  )
                  drawPixel(pixel_x, pixel_y, color);
                else  
                  fillRect(pixel_x+(row*size), pixel_y+(bit*size), size, size, bgcolor);
            }else{
                // Desenhar o pixel
                if( size == 1  )
                  drawPixel(pixel_x, pixel_y, bgcolor);
                else  
                  fillRect(pixel_x+row*size, pixel_y+bit*size, size, size, bgcolor);
            }
        }
    }
}


void drawChar(uint8_t c, color_t color, color_t bg, uint8_t size) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

  if(size == 2)
    drawChar_interna( priv->cursor->x, priv->cursor->y, c, color, bg, size);
  else  
    drawChar2( priv->cursor->x, priv->cursor->y, c, color, bg, size);
}

void tft_write(uint8_t c){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

  switch(c){
    case 0x0A:  // \n
              priv->cursor->y += priv->font.height;
    case 0x0D:  // \r
              priv->cursor->x  = 0;
              break;
    case 0x09:  // \t
              int new_x = priv->cursor->x + priv->tabspace;
              if (new_x < priv->width){
                  priv->cursor->x = new_x;
              }
              break;
    default:
              drawChar( c, priv->textcolor, priv->textbgcolor, priv->font.size);
              //priv->cursor->x += priv->font.size*priv->font.width;
              priv->cursor->x += priv->font.width;  //multiply last so much time

              if(priv->cursor->x >= priv->width ) {
                priv->cursor->y += priv->font.height;
                priv->cursor->x = 0;
              }
  }
  if( priv->cursor->y >= priv->height ){
    clrscr();
  }
/*
  if (c == '\n') {
    priv->cursor->y += priv->font.height;
    priv->cursor->x  = 0;
  } else if (c == '\r') {
    // skip em
  } else if (c == '\t'){
      int new_x = priv->cursor->x + priv->tabspace;
      if (new_x < priv->width){
          priv->cursor->x = new_x;
      }
  } else {
  
    drawChar( c, priv->textcolor, priv->textbgcolor, priv->font.size);
    priv->cursor->x += priv->font.size*priv->font.width;

    if ((priv->cursor->x > (priv->width - priv->font.size*6))) {
      priv->cursor->y += priv->font.height;
      priv->cursor->x = 0;
    }
 
  }
  if( priv->cursor->y >= 480 ){
    clrscr();
  }
  */
}

void put_cursor(uint8_t c){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

  uint16_t cursorx=priv->cursor->x;
  uint16_t cursory=priv->cursor->y;

  cursorx += priv->font.size*priv->font.width;
  if ((cursorx > (priv->width - priv->font.size*6))) {
    cursory += priv->font.height;
    cursorx = 0;
  }
  if( priv->cursor->visible == true ){
    if( priv->cursor->blink == true ){
      if( c )
        drawChar( priv->cursor->shape, priv->textcolor, priv->textbgcolor, 1);
      else  
        drawChar( ' ', priv->textcolor, priv->textbgcolor, 1);
    }else{
        drawChar( priv->cursor->shape, priv->textcolor, priv->textbgcolor, 1);
    }
  }else{
        drawChar( ' ', priv->textcolor, priv->textbgcolor, 1);
  }
}

uint8_t bufferA[2400];
uint8_t bufferB[2400];
uint8_t *front_buffer = bufferA;
uint8_t *back_buffer = bufferB;

void vga_scroll() {
    // 1. Copia da linha 1 até 29 do front para a linha 0 até 28 do back
    // (80 colunas * 29 linhas = 2320 bytes)
    memcpy(back_buffer, front_buffer + 80, 2320);

    // 2. Limpa a nova linha 29 no back buffer
    memset(back_buffer + 2320, ' ', 80);

    // 3. TROCA OS PONTEIROS (O back vira front e vice-versa)
    uint8_t *temp = front_buffer;
    front_buffer = back_buffer;
    back_buffer = temp;

    // 4. Reposiciona o cursor na última linha
    vga->setTextCursorPos(0,29);
}

static void setTextCursor_old(uint16_t x, uint16_t y) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  if((x >= priv->width) || (y >= priv->height)) 
    return;
  if(x*priv->font.width >= priv->width) {
    x = x*priv->font.width / priv->font.width;
    if( y < priv->height )
      y = y *priv->font.height;
    else
      y = 0 ;
  }else{
    x = x*priv->font.width;
    if( y < priv->height )
      y = y *priv->font.height;
    else
      y = 0 ;
  }

  priv->cursor->x = x;
  priv->cursor->y = y;
}
static void setTextCursor(uint16_t x, uint16_t y) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  
  // 1. Verifica se a posição em caracteres está além da matriz de texto
  if((x >= priv->width) || (y >= priv->height)) 
    return;

  // 2. Calcula a posição real em PIXELS usando variáveis locais
  // para evitar o efeito bola de neve multiplicativa
  uint16_t pixel_x = x * priv->font.width;
  uint16_t pixel_y = y * priv->font.height;

  // 3. Garante que a posição em pixels não vai estourar a resolução da tela
  if (pixel_x >= priv->width)  pixel_x = 0;
  if (pixel_y >= priv->height) pixel_y = 0;

  // 4. Salva com segurança no cursor do sistema
  priv->cursor->x = pixel_x;
  priv->cursor->y = pixel_y;
}
/*
static void setTextSize(uint8_t s) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  if(s >0 ){
    priv->font.size = s;
  }else{
    priv->font.size = 1;
  }
}
*/
static uint8_t getTextSize(void) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  
  return priv->font.size;
}
void set_blink_interval(uint16_t interval)
{
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  priv->cursor->blink_interval = interval;
}
static uint16_t get_blink_interval(void){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  return priv->cursor->blink_interval;
}
static void setTextCursorVisible(bool v) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  priv->cursor->visible = v;
}
static void setTextCursorBlink(bool b) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  priv->cursor->blink = b;
}
//static void setTextColor(char c) {
//  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
//  priv->textcolor = priv->textbgcolor = c;
//}


static void setTextColor(char c, char b) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  priv->textcolor = c;
  priv->textbgcolor = b;
}

static uint8_t getTextColor(void) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  return priv->textcolor;
}


static void printString(char* str){
    while (*str){
        tft_write(*str++);
    }
}

static void printString1(char* str,int32_t num){
  uint8_t buf[8+1];
  while (*str){
      tft_write(*str++);
  }
  sprintf(buf,"%x",num);
  uint8_t size = sizeof(buf);
  buf[--size]='\0';
  uint8_t i=0;
  while(buf[i]){
    tft_write(buf[i++]);
  }
  tft_write('\n');
}
static void printString2(char* str,int32_t num){
  uint8_t buf[8+1];
  while (*str){
      tft_write(*str++);
  }
  sprintf(buf,"%x",num);
  uint8_t size = sizeof(buf);
  buf[--size]='\0';
  uint8_t i=0;
  while(buf[i]){
    tft_write(buf[i++]);
  }
}


static void setTextColorBig(color_t color, char background) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  priv->textcolor = color;
  priv->textbgcolor = background;
}
/* 
static void writeStringBold(char* str){
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;

    char temp_bg ;
    temp_bg = priv->textbgcolor;
    while (*str){
        char c = *str++;
        drawChar_interna(priv->cursor->x, priv->cursor->y, c, priv->textcolor, priv->textbgcolor, priv->font.size);
        drawChar_interna(priv->cursor->x+1, priv->cursor->y, c, priv->textcolor, priv->textcolor, priv->font.size);
        priv->cursor->x += 7 * priv->font.size ;
    }
    priv->textbgcolor = temp_bg ;
}
*/
short readPixel(short x, short y) {
  vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
  int pixel = ((640 * y) + x) ;
  short color ;
  if (pixel & 1) {
      color = priv->vga_data_array[pixel>>1] >> 4 ;
  }
  else {
      color = priv->vga_data_array[pixel>>1] & 0xf  ;
  }
  return color ;
}
extern void __not_in_flash_func(vga_line_handler)();
//ESTA FUNÇÃO ESTÁ OK PARA 320 TESTADO 26/03
void init_vga_320x200(){
  // 1. Habilita a interrupção específica do PIO0
  pio_set_irq0_source_enabled(pio0, pis_interrupt1, true);
  // 2. Diz qual função deve rodar
  irq_set_exclusive_handler(PIO0_IRQ_0, vga_line_handler);
  // 3. Liga a interrupção no nível do processador
  irq_set_enabled(PIO0_IRQ_0, true);
  irq_set_priority(PIO0_IRQ_0, 0);  
}

//extern char *active_buffer;
#define TXCOUNT 153600 // Total pixels/2 (since we have 2 pixels per byte)
char vga_video_data_array0[TXCOUNT];
char vga_video_data_array1[TXCOUNT];
char *active_buffer = (char *)&vga_video_data_array0[0];
unsigned char buffer=0;
font_t *font;


//ESTA FUNÇÃO ESTÁ OK PARA 320 TESTADO 26/03
vga_t* create_screen(screenMode_t mode){ //,uint8_t active_buffer1[],uint32_t txcount,font_t * font1){
  vga = (vga_t*)malloc(sizeof(vga_t));
  vga_text_private_t* priv = (vga_text_private_t*)malloc(sizeof(vga_text_private_t));
  if (!vga || !priv) {
      free(vga);
      free(priv);
      return NULL;
  }
  font = set_font(FONTE_8X16);
    // Initialize the VGA screen
  initVGA(  &active_buffer, TXCOUNT , mode) ;

  if( mode == MODE_TEXT_40_S ||
      mode == MODE_TEXT_40_F ||
      mode == MODE_320x240
    ){
    init_vga_320x200();
  }
  vga->_private = priv;
  
  priv->cursor = create_default_cursor() ;  
  priv->textcolor = WHITE ;
  priv->textbgcolor = BLACK ;
  priv->font.name = font->name;
  priv->font.width = font->width ;
  priv->font.height= font->height ;
  priv->font.size = font->size ;   
  priv->font.data = font->name ;
  priv->tabspace = 4;
  priv->txcount = TXCOUNT ;
  priv->topmask = 0b00001111 ;
  priv->bottommask = 0b11110000 ;
  priv->vga_data_array = &active_buffer[0] ;

  set_vga_mode(mode);

  vga->printString = printString;
  vga->setTextColor = setTextColor;
  vga->getTextColor = getTextColor;
  vga->getTextSize = getTextSize;
  vga->setTextCursorPos = setTextCursor;
  vga->clrscr = clrscr;
  vga->setTextCursorVisible = setTextCursorVisible;
  vga->setTextCursorBlink = setTextCursorBlink;
  vga->get_blink_interval = get_blink_interval;
  vga->set_blink_interval = set_blink_interval;
  vga->set_vga_home = set_vga_home;
  vga->set_vga_mode = set_vga_mode;
  vga->pchar = tft_write;
  vga->set_vga_data_array = set_vga_data_array;
  vga->printString1 = printString1;
  vga->printString2 = printString2;
  vga->inc_cursor_y = inc_cursor_y;
  vga->inc_cursor_x = inc_cursor_x;
  vga->dec_cursor_y = dec_cursor_y;
  vga->dec_cursor_x = dec_cursor_x;

  return vga;
}
/*
extern vga_t *vga;


void change_modeold(uint8_t mode){
    // 1. Libera APENAS o que é dinâmico dentro do private, se houver
    vga_text_private_t* priv = (vga_text_private_t*)vga->_private;
    if(priv->cursor) {
        // Se o cursor foi alocado dinamicamente, libere aqui
        // free(priv->cursor); 
    }

    // 2. Em vez de free(vga), vamos apenas "reconfigurar"
    // Pode ser necessário parar os PIOs/DMA antes de mudar o modo
    // stop_vga_dma(); 
    char buf[8];
    sprintf(buf,"\nmode [%d]\n",mode);
    vga->printString(buf);
    vga->printString("Chamando initVGA\n");
    // 3. Chame as rotinas de inicialização de hardware para o novo modo
    initVGA(&active_buffer, TXCOUNT, mode);
    
    vga->printString("Se modo 320p chama init vga 32-x200\n");
    if(mode == MODE_TEXT_40_S || mode == MODE_TEXT_40_F || mode == MODE_320x240) {
        init_vga_320x200();
    }

    // 4. Atualize os campos de estado do vga e o modo
    vga->printString("Configurando mode");
    vga->screen_mode = mode;
    set_vga_mode(mode);
    
    // Atualize as fontes e masks no priv existente sem dar free nele
    font = set_font(FONTE_8X16);
    priv->font.height = font->height;
    // ... atualizar demais campos do priv ...
}


// Esta função deve ser chamada APENAS UMA VEZ no seu main.c
vga_t* create_vga_instance() {
    vga_t* instance = (vga_t*)malloc(sizeof(vga_t));
    vga_text_private_t* priv = (vga_text_private_t*)malloc(sizeof(vga_text_private_t));

    if (!instance || !priv) {
        free(instance);
        free(priv);
        return NULL;
    }

    // Link fixo entre a estrutura principal e a privada
    instance->_private = priv;

    // Atribuição fixa dos ponteiros de função (vtable manual)
    instance->printString = printString;
    instance->setTextColor = setTextColor;
    instance->getTextColor = getTextColor;
    instance->getTextSize = getTextSize;
    instance->setTextCursorPos = setTextCursor;
    instance->clrscr = clrscr;
    instance->setTextCursorVisible = setTextCursorVisible;
    instance->setTextCursorBlink = setTextCursorBlink;
    instance->get_blink_interval = get_blink_interval;
    instance->set_blink_interval = set_blink_interval;
    instance->set_vga_home = set_vga_home;
    instance->set_vga_mode = set_vga_mode;
    instance->pchar = tft_write;
    instance->set_vga_data_array = set_vga_data_array;

    return instance;
}

// Esta é a função que faz o trabalho pesado de trocar o modo sem dar free
void vga_setup_mode(vga_t* instance, screenMode_t mode) {
    if (!instance) return;

    vga_text_private_t* priv = (vga_text_private_t*)instance->_private;

    // 1. Configuração de Hardware (Para o PIO/DMA antes de mudar)
    // Se você tiver uma função para parar o DMA, chame-a aqui.
    
    font = set_font(FONTE_8X16);
    initVGA(&active_buffer, TXCOUNT, mode);

    if (mode == MODE_TEXT_40_S || mode == MODE_TEXT_40_F || mode == MODE_320x240) {
        init_vga_320x200();
    }

    // 2. Atualização dos dados de controle (sem malloc!)
    priv->cursor = create_default_cursor(); 
    priv->textcolor = WHITE;
    priv->textbgcolor = BLACK;
    priv->font.name = font->name;
    priv->font.width = font->width;
    priv->font.height = font->height;
    priv->font.size = font->size;
    priv->font.data = font->name;
    priv->tabspace = 4;
    priv->txcount = TXCOUNT;
    priv->topmask = 0b00001111;
    priv->bottommask = 0b11110000;
    priv->vga_data_array = &active_buffer[0];

    instance->screen_mode = mode;
    set_vga_mode(mode);
}
*/

#include "eeprom.h"
#include "hardware/resets.h"
extern sys_config_t vga_nvc_config;

void change_mode(screenMode_t mode) {
    
//  pio_clear_instruction_memory(pio0);
//  pio_clear_instruction_memory(pio1);  

  eeprom_load_config(&vga_nvc_config);
  char buf[64];
  sprintf(buf,"Antes da configuracao: [%d]\n",(int8_t)vga_nvc_config.video_mode);
  vga->printString(buf);
  vga_nvc_config.video_mode = mode;  
  eeprom_save_config(&vga_nvc_config);
  sprintf(buf,"Depois da configuracao:[%d]\n",(int8_t)vga_nvc_config.video_mode);
  vga->printString(buf);

  // 1. Coloca o PIO0 e o DMA em estado de reset (limpa FIFOs e máquinas de estado)
  reset_block(RESETS_RESET_PIO0_BITS | RESETS_RESET_DMA_BITS);

  // 2. Tira do reset e espera o hardware confirmar que está pronto
  unreset_block_wait(RESETS_RESET_PIO0_BITS | RESETS_RESET_DMA_BITS);

  reset();

}