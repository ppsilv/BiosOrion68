.section .text
.align 2

.global rtc_ds3231_read_time
.global rtc_ds3231_set_time
.global rtc_eeprom_read_byte
.global rtc_eeprom_write_byte

.extern ds3231_read_time
.extern ds3231_set_time
.extern eeprom_read_byte
.extern eeprom_write_byte


/*
    trap #4 Servico #4
    Send: Nothing
    Return: D0 Pointer to struct rtc_time_t
*/
rtc_ds3231_read_time:
    jsr     ds3231_read_time  | Chama a função
    bra     trap4_exit

/*
    trap #4 Servico #2
    Send: A pointer to struct rtc_time_t
    Return: Nothing    
*/
rtc_ds3231_set_time:
    move.l  %D0, -(%sp)       | 1º parâmetro: data in D0
    jsr     ds3231_set_time    | Chama a função
    addq.l  #4, %sp            | Limpa 4 bytes da pilha (1 ponteiro)    
    bra     trap4_exit

/*
    trap #4 Servico #3
    Send: Address to be read
    Return: Data read in D0
*/
rtc_eeprom_read_byte:
    jsr eeprom_read_byte
    bra     trap4_exit

/*
    trap #4 Servico #4
    Send: D0 address to be written, D1 data to be written
    Return: Nothing
*/
rtc_eeprom_write_byte:
    move.l  %D2, -(%sp)       | 2º parâmetro: data 
    move.l  %D0, -(%sp)       | 1º parâmetro: addr 
    jsr     eeprom_write_byte  | Chama a função
    addq.l  #8, %sp            | Limpa 8 bytes da pilha (2 args x 4 bytes)
    bra     trap4_exit

trap4_exit:
    movem.l (%sp)+,%d1-%d7/%a0-%a6
    rte
