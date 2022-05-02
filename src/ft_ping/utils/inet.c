#include <stdint.h>

#include "ft_ping/utils/inet.h"

static uint16_t host_network_conversion_uint16(uint16_t uint16)
{
	uint8_t *bytes = (uint8_t *)&uint16;
	return (bytes[0] << 8) | (bytes[1] << 0);
}

uint16_t ft_htons(uint16_t hostshort)
{
	return host_network_conversion_uint16(hostshort);
}

uint16_t ft_ntohs(uint16_t netshort)
{
	return host_network_conversion_uint16(netshort);
}

static uint32_t host_network_conversion_uint32(uint32_t uint32)
{
	uint8_t *bytes = (uint8_t *)&uint32;
	return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3] << 0);
}

uint32_t ft_htonl(uint32_t hostlong)
{
	return host_network_conversion_uint32(hostlong);
}

uint32_t ft_ntohl(uint32_t netlong)
{
	return host_network_conversion_uint32(netlong);
}
