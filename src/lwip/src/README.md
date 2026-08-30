api/      - The code for the high-level wrapper API. Not needed if
            you use the lowel-level call-back/raw API.

apps/     - Higher layer applications that are specifically programmed
            with the lwIP low-level raw API.

core/     - The core of the TPC/IP stack; protocol implementations,
            memory and buffer management, and the low-level raw API.

include/  - lwIP include files.

netif/    - Generic network interface device drivers are kept here.

For more information on the various subdirectories, check the FILES
file in each directory.


Vou te dar essa lista olhando o `Filelists.cmake` de verdade dentro
do repositório — é literalmente o arquivo que aquele `CMakeLists.txt`
que você colou importa (`include(src/Filelists.cmake)`), e é a fonte
de verdade que o próprio projeto lwIP usa pra saber quais arquivos
compõem o core. Não vou chutar de memória.Preciso ser honesto com
você em vez de te dar falsa certeza: os resultados de busca me deram
o link certo, mas não o conteúdo completo do arquivo (a busca cortou
nos detalhes de versão). Antes de te passar qualquer lista como "100%
certa", vou puxar o conteúdo completo direto da fonte.Consegui o
conteúdo completo e oficial do `Filelists.cmake` (versão atual do
repositório, lwIP **2.2.2-dev**). Aqui está a lista, extraída direto
da fonte — não da minha memória:

## Core mínimo (`lwipcore_SRCS`) — sempre necessário
```
src/core/init.c
src/core/def.c
src/core/dns.c
src/core/inet_chksum.c
src/core/ip.c
src/core/mem.c
src/core/memp.c
src/core/netif.c
src/core/pbuf.c
src/core/raw.c
src/core/stats.c
src/core/sys.c
src/core/tcp.c
src/core/tcp_in.c
src/core/tcp_out.c
src/core/timeouts.c
src/core/udp.c
```
(Deixei de fora `altcp.c`, `altcp_alloc.c`, `altcp_tcp.c` — são a
camada de TCP "abstrato" pra TLS opcional; você não precisa disso
pro seu caso.)

## IPv4 (`lwipcore4_SRCS`) — você precisa, já que não vai usar IPv6
```
src/core/ipv4/acd.c
src/core/ipv4/autoip.c
src/core/ipv4/dhcp.c
src/core/ipv4/etharp.c
src/core/ipv4/icmp.c
src/core/ipv4/igmp.c
src/core/ipv4/ip4_frag.c
src/core/ipv4/ip4.c
src/core/ipv4/ip4_addr.c
```

## Netif (só o que interessa da lista `lwipnetif_SRCS`)
```
src/netif/ethernet.c
```
(`bridgeif.c`, `bridgeif_fdb.c` e `slipif.c` são pra bridge Ethernet
e SLIP — não se aplicam ao seu caso de uma única interface Ethernet
via W5100.)

## O que **não** entra na sua build
- `lwipcore6_SRCS` (IPv6) — descarte, você não mencionou precisar.
- `lwipapi_SRCS` (`api_lib.c`, `sockets.c`, `tcpip.c`, etc.) — essa
é a API sequencial/sockets, que exige `sys_arch` com threads/semáforos
reais. Como você vai rodar `NO_SYS=1` (bare-metal, polling), você usa
a **raw API** direto (`tcp_new`, `udp_new`, callbacks) e não precisa
de nada dessa lista.
- `lwipppp_SRCS`, `lwipsixlowpan_SRCS`, e tudo de `lwipallapps_SRCS`
(SNMP, HTTP, MQTT, SNTP, mDNS, etc.) — só inclua se for usar essas
aplicações especificamente.

## Total: 27 arquivos `.c` do lwIP core+ipv4+ethernet, mais os 3
arquivos de porte que você vai escrever (`cc.h`, `sys_arch.c/.h`,
`lwipopts.h`) e o `ethernetif.c` de ponte com seu driver W5100.

Isso é a lista confirmada direto do `Filelists.cmake` do commit
atual do `master`. Se quiser travar numa release estável em vez do
`master` (que é dev/`2.2.2.dev`), a última tag oficial é a
`STABLE-2_2_1_RELEASE` — os nomes de arquivo dessa lista são estáveis
há várias releases, então não deve mudar nada relevante pro seu caso,
mas se quiser posso confirmar o `Filelists.cmake` daquela tag específica
antes de você clonar.
