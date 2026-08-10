void gravar_eeprom_byte(unsigned int endereco_real, unsigned char dado_real) {
    // Assumindo os ponteiros ajustados para o barramento do 68k
    // com a base da ROM inclusa.
    
    volatile unsigned char *eeprom = (volatile unsigned char *)BASE_ROM;

    // 1. Sequência de Destravamento por Software (Software Data Protection)
    eeprom[0x5555 << 1] = 0xAA;
    eeprom[0x2AAA << 1] = 0x55;
    eeprom[0x5555 << 1] = 0xA0; // Comando de escrita de byte

    // 2. Escrita do dado desejado
    eeprom[endereco_real << 1] = dado_real;

    // 3. Aguarda o término usando DATA Polling no Bit 7
    // Ele fica em loop enquanto o Bit 7 lido for diferente do Bit 7 original
    while ((eeprom[endereco_real << 1] & 0x80) != (dado_real & 0x80)) {
        // Aguarda o silício terminar de queimar o byte...
    }
}