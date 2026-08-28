#!/usr/bin/env python3
import sys
import os

def mesclar_binarios(arq_base, arq_topo, endereco_destino, arq_saida="rom_final.bin"):
    # Converte o endereço para inteiro se for passado como string Hex (ex: 0x8000)
    if isinstance(endereco_destino, str):
        if endereco_destino.lower().startswith("0x"):
            offset = int(endereco_destino, 16)
        else:
            offset = int(endereco_destino)
    else:
        offset = endereco_destino

    print(f"[*] Lendo arquivo base: {arq_base}")
    with open(arq_base, "rb") as f:
        dados_base = bytearray(f.read())

    tamanho_base = len(dados_base)
    print(f"[i] Tamanho do arquivo base: {tamanho_base} bytes (hex: {hex(tamanho_base)})")
    print(f"[i] Endereço alvo para o segundo arquivo: {offset} bytes (hex: {hex(offset)})")

    # Validação crítica: o arquivo base não pode ser maior que o endereço de destino
    if tamanho_base > offset:
        print(f"\n[ERRO] O arquivo base ({tamanho_base} bytes) já ultrapassou")
        print(f"       o endereço de destino configurado ({offset} bytes)!")
        print("       A concatenação causaria sobreposição de dados. Abortando.")
        sys.exit(1)

    # Calcula quantos bytes faltam para chegar no endereço alvo
    bytes_faltantes = offset - tamanho_base
    
    # Preenche o espaço vazio com 0xFF (padrão de EPROM apagada)
    if bytes_faltantes > 0:
        print(f"[+] Preenchendo buraco de {bytes_faltantes} bytes com 0xFF...")
        dados_base.extend([0xFF] * bytes_faltantes)

    print(f"[*] Lendo arquivo secundário: {arq_topo}")
    with open(arq_topo, "rb") as f:
        dados_topo = f.read()

    # Concatena o segundo arquivo exatamente na posição do offset
    dados_base.extend(dados_topo)
    tamanho_final = len(dados_base)

    # Grava o resultado final
    print(f"[+] Gravando resultado em: {arq_saida}")
    with open(arq_saida, "wb") as f:
        f.write(dados_base)

    print(f"[✓] Sucesso! ROM gerada com tamanho total de {tamanho_final} bytes ({hex(tamanho_final)}).\n")

if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("Uso correto:")
        print("  python3 mesclar_rom.py <arquivo_base> <arquivo_segundo> <endereco_hex_ou_dec> [arquivo_saida]")
        print("\nExemplos:")
        print("  python3 mesclar_rom.py crt0.bin keyboard.bin 0x8000")
        print("  python3 mesclar_rom.py bios.bin monitor.bin 32768 rom_completa.bin")
        sys.exit(1)

    # Captura os argumentos
    arquivo_1 = sys.argv[1]
    arquivo_2 = sys.argv[2]
    endereco = sys.argv[3]
    
    # Se o usuário não digitar o nome do arquivo de saída, usa o padrão "rom_final.bin"
    saida = sys.argv[4] if len(sys.argv) > 4 else "rom_final.bin"

    # Executa a mesclagem
    try:
        mesclar_binarios(arquivo_1, arquivo_2, endereco, saida)
    except FileNotFoundError as e:
        print(f"\n[ERRO] Arquivo não encontrado: {e.filename}")
    except ValueError:
        print(f"\n[ERRO] Endereço inválido: '{endereco}'. Use formatos como 0x8000 ou 32768.")

