#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Configurações padrão essenciais para o Pico W / Pico 2 W
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_HOSTNAME         1

// Ajustes de Memória (O Pico 2 tem bastante RAM, dá para deixar folgado)
#define MEM_LIBC_MALLOC             0
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    16000
#define MEMP_NUM_TCP_PCB            16
#define MEMP_NUM_ARP_QUEUE          10

// Recursos de Rede Habilitados
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)

#define LWIP_DHCP                   1
#define LWIP_DNS                    1

// Se não for usar IPv6, deixe desativado para economizar RAM
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0

// Adicione estas linhas para satisfazer a checagem de sanidade do lwIP:
#define MEMP_NUM_TCP_SEG            32
#define TCP_SND_QUEUELEN            32

#endif