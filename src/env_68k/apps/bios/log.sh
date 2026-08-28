
#!/bin/bash

# Número da semana do ano (00..53) - Começando no Domingo
SEMANA_DOMINGO=$(date +%U)

# Número da semana do ano (00..53) - Começando na Segunda-feira
SEMANA_SEGUNDA=$(date +%W)

# Número da semana no padrão ISO 8601 (01..53) - Padrão internacional comum
SEMANA_ISO=$(date +%V)

# Dia da semana (1..7) - Sendo 1 segunda-feira
DIA_SEMANA=$(date +%u)

# Nome do dia da semana (ex: Segunda-feira, Terça-feira...)
NOME_DIA=$(date +%A)

# Dia do ano (001..366)
DIA_DO_ANO=$(date +%j)

# Exibindo os resultados
echo "Semana do ano (ISO): $SEMANA_ISO"
echo "Dia da semana (número): $DIA_SEMANA ($NOME_DIA)"
echo "Dia do ano: $DIA_DO_ANO"


