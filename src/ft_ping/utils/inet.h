#ifndef FT_PING_UTILS_INET_H
#define FT_PING_UTILS_INET_H

#include <stdint.h>

uint32_t ft_htonl(uint32_t hostlong);
uint16_t ft_htons(uint16_t hostshort);
uint32_t ft_ntohl(uint32_t netlong);
uint16_t ft_ntohs(uint16_t netshort);

#endif
