#!/bin/bash

# Número da semana no padrão ISO 8601 (01..53) - Padrão internacional comum
SEMANA_ISO=$(date +%V)
# Dia do ano (001..366)
DIA_DO_ANO=$(date +%j)

# Gera o arquivo .asm com o contador
echo "#define BUILD_COUNTER \"${SEMANA_ISO}${DIA_DO_ANO}\"" > build_counter.h
#echo "#define BUILD_COUNTER ${SEMANA_ISO}${DIA_DO_ANO}" > build_counter.h