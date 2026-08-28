#!/bin/bash
# Arquivo que guarda o número da compilação
COUNTER_FILE="build_counter.txt"
rm build_date.inc
if [ ! -f "build_date.inc" ]; then
    echo ";From here it is generated and pushed by update_build.sh"  > build_date.inc
    echo "BUILD_DATE:" >> monitor.asm >> build_date.inc
    #echo '              DC.B "Build: '"$(date +'%Y-%m-%d %H:%M:%S')"'",13,10' >> build_date.inc
    # Extrai cada componente da data e soma tudo
    YEAR=$(date +'%Y')     # Ano (4 dígitos)
    MONTH=$(date +'%m')    # Mês (01-12)
    DAY=$(date +'%d')      # Dia (01-31)
    #HOUR=$(date +'%H')     # Hora (00-23)
    #MIN=$(date +'%M')      # Minuto (00-59)
    #SEC=$(date +'%S')      # Segundo (00-59)

    # Calcula a soma (remove zeros à esquerda para evitar octal)
    #TOTAL=$((10#$YEAR + 10#$MONTH + 10#$DAY + 10#$HOUR + 10#$MIN + 10#$SEC))
    TOTAL=$((10#$YEAR + 10#$MONTH + 10#$DAY))

    # Gera o arquivo .asm com a string e a soma
    echo '      DC.B "Build.: '"$(date +'%Y-%m-%d')"'",13,10' >> build_date.inc
    echo '      DC.B "Serial: '$DAY$TOTAL'",13,10,0' >> build_date.inc
    echo ";End generated code" >> build_date.inc
fi
# Se o arquivo não existe, cria com valor 1
if [ ! -f "$COUNTER_FILE" ]; then
    echo "1" > "$COUNTER_FILE"
fi

# Lê o valor atual, incrementa e salva
CURRENT_VALUE=$(cat "$COUNTER_FILE")
NEW_VALUE=$((CURRENT_VALUE + 1))
echo "$NEW_VALUE" > "$COUNTER_FILE"

# Gera o arquivo .asm com o contador
#echo '      DC.B "Compilation: #'$NEW_VALUE'",0' > build_counter.inc
