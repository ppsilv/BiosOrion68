    vga->init = 
    vga->clrscr = 
    vga->pchar = 
    vga->draw_pixel = 
    vga->draw_hline = 
    vga->fill_rect = 
    vga->draw_char_interna = 
    vga->draw_char = 
    vga->print_string = 
    vga->set_text_cursor = 
    vga->set_text_color = 
    vga->set_text_color2 = 
    vga->set_text_wrap = 
    vga->tft_write = 
    vga->set_text_color_big = 
    vga->write_string_bold = 
    vga->read_pixel = 
 vga_text_init(vga_text_t* vga, uint8_t mode) ;
 vga_text_clrscr(vga_text_t* vga) ;
 vga_text_pchar(vga_text_t* vga, uint8_t c) ;
 vga_text_draw_pixel(vga_text_t* vga, int16_t x, int16_t y, color_t color) ;
 vga_text_draw_hline(vga_text_t* vga, int16_t x, int16_t y, int16_t w, color_t color) ;
 vga_text_fill_rect(vga_text_t* vga, int16_t x, int16_t y, int16_t w, int16_t h, color_t color) ;
 vga_text_draw_char_interna(vga_text_t* vga, int16_t x, int16_t y, uint8_t c,
 vga_text_draw_char(vga_text_t* vga, uint8_t c, color_t color, color_t bg, uint8_t size) ;
 vga_text_set_text_cursor(vga_text_t* vga, int16_t x, int16_t y) ;
 vga_text_set_text_color(vga_text_t* vga, color_t c) ;
 vga_text_set_text_color2(vga_text_t* vga, color_t c, color_t b) ;
 vga_text_set_text_wrap(vga_text_t* vga, bool w) ;
 vga_text_tft_write(vga_text_t* vga, uint8_t c) ;
 vga_text_print_string(vga_text_t* vga, int8_t* str) ;
 vga_text_set_text_color_big(vga_text_t* vga, color_t color, color_t background) ;
 vga_text_write_string_bold(vga_text_t* vga, int8_t* str) ;
 vga_text_read_pixel(const vga_text_t* vga, int16_t x, int16_t y) ;
 
    // Getters
    vga->get_cursor_x = vga_text_get_cursor_x;
    vga->get_cursor_y = vga_text_get_cursor_y;
    vga->get_width = vga_text_get_width;
    vga->get_height = vga_text_get_height;
    vga->get_textsize = vga_text_get_text_size;

    // Setters
    vga->set_cursor_x = vga_text_set_cursor_x;
    vga->set_cursor_y = vga_text_set_cursor_y;
    vga->set_width = vga_text_set_width;
    vga->set_height = vga_text_set_height;
    vga->set_textsize = vga_text_set_textsize;


 (const vga_text_t* vga) ;
 (const vga_text_t* vga) ;
    
