#!/bin/bash

# Arquivo que guarda o número da compilação
COUNTER_FILE="build_counter.txt"

# Se o arquivo não existe, cria com valor 1
if [ ! -f "$COUNTER_FILE" ]; then
    echo "1" > "$COUNTER_FILE"
fi

# Lê o valor atual, incrementa e salva
CURRENT_VALUE=$(cat "$COUNTER_FILE")
NEW_VALUE=$((CURRENT_VALUE + 1))
echo "$NEW_VALUE" > "$COUNTER_FILE"

# Gera o arquivo .asm com o contador
echo 'DC.B "Compilacao RETRO #'$NEW_VALUE'",0' > build_counter.inc
