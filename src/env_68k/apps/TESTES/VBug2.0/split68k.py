#!/usr/bin/python3
import os
import sys

def split_68k_binary(input_file):
    """Divide um arquivo binário para MC68000 seguindo a convenção:
       EVEN (HIGH) -> D8-D15 (bytes pares/MSB)
       ODD (LOW)   -> D0-D7  (bytes ímpares/LSB)
       Padrão usado por Withit Sirichote em projetos reais"""
    
    # Verifica se o arquivo existe
    if not os.path.exists(input_file):
        print(f"Erro: Arquivo '{input_file}' não encontrado!")
        sys.exit(1)
    
    # Gera nomes dos arquivos de saída (convenção EVEN/ODD)
    base_name = os.path.splitext(input_file)[0]
    even_file = f"{base_name}_high.bin"  # HIGH (D8-D15)
    odd_file = f"{base_name}_low.bin"    # LOW (D0-D7)
    
    # Lê o arquivo original
    with open(input_file, 'rb') as f:
        data = f.read()
    
    # Verifica tamanho e adiciona padding se necessário
    if len(data) % 2 != 0:
        print(f"Aviso: Arquivo com tamanho ímpar ({len(data)} bytes), adicionando padding 0x00")
        data += b'\x00'  # Padding com 0x00 é mais seguro para vetores
    
    # Processa os bytes (agora corretamente!)
    even_bytes = bytearray()  # HIGH (bytes pares/D8-D15)
    odd_bytes = bytearray()   # LOW (bytes ímpares/D0-D7)
    
    for i in range(0, len(data), 2):
        even_bytes.append(data[i])     # Byte PAR -> EVEN (D8-D15)
        if i+1 < len(data):
            odd_bytes.append(data[i+1])  # Byte ÍMPAR -> ODD (D0-D7)
        else:
            odd_bytes.append(0x00)      # Padding para alinhamento
    
    # Escreve os arquivos de saída
    with open(even_file, 'wb') as f_even, open(odd_file, 'wb') as f_odd:
        f_even.write(even_bytes)
        f_odd.write(odd_bytes)
    
    print(f"Arquivos gerados para MC68000 (convenção EVEN/ODD):")
    print(f"- EVEN (D8-D15, HIGH/bytes pares): {even_file}")
    print(f"- ODD  (D0-D7, LOW/bytes ímpares): {odd_file}")
    print(f"Tamanho original: {len(data)} bytes | EVEN: {len(even_bytes)} bytes | ODD: {len(odd_bytes)} bytes")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Uso: split68k.py <arquivo_binario>")
        print("Exemplo: split68k.py firmware.bin")
        print("Nota: Saída seguirá padrão EVEN=HIGH(D8-D15), ODD=LOW(D0-D7)")
        sys.exit(1)
    
    split_68k_binary(sys.argv[1])

