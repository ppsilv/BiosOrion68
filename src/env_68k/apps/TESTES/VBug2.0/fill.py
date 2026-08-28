#!/usr/bin/env python3
import os
import sys

def fill_file(input_file, target_size):
    """Preenche um arquivo binário com 0x00 até o tamanho especificado"""
    
    # Verifica se o arquivo existe
    if not os.path.exists(input_file):
        print(f"Erro: Arquivo '{input_file}' não encontrado!")
        return False
    
    # Obtém o tamanho atual do arquivo
    current_size = os.path.getsize(input_file)
    
    # Verifica se já está do tamanho correto
    if current_size == target_size:
        print(f"O arquivo '{input_file}' já tem o tamanho correto de {target_size} bytes.")
        return True
    
    # Verifica se o arquivo é maior que o alvo
    if current_size > target_size:
        print(f"Erro: O arquivo '{input_file}' tem {current_size} bytes, que é maior que o tamanho alvo de {target_size} bytes.")
        return False
    
    # Calcula quantos bytes precisam ser adicionados
    bytes_needed = target_size - current_size
    
    # Preenche com 0x00
    try:
        with open(input_file, 'ab') as f:  # 'ab' mode para adicionar em modo binário
            f.write(b'\x00' * bytes_needed)
        
        print(f"Arquivo '{input_file}' preenchido com sucesso.")
        print(f"Bytes adicionados: {bytes_needed} (0x00)")
        print(f"Tamanho final: {target_size} bytes")
        return True
    
    except Exception as e:
        print(f"Erro ao preencher o arquivo: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Uso: fill.py <arquivo_binario> <tamanho_desejado>")
        print("Exemplo: fill.py firmware.bin 8192")
        sys.exit(1)
    
    try:
        target_size = int(sys.argv[2])
        if target_size <= 0:
            raise ValueError
    except ValueError:
        print("Erro: O tamanho deve ser um número inteiro positivo!")
        sys.exit(1)
    
    input_file = sys.argv[1]
    success = fill_file(input_file, target_size)
    
    sys.exit(0 if success else 1)
