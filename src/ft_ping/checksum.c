#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "ft_ping/checksum.h"

uint16_t get_checksum(void *addr, size_t size)
{
	assert(addr != NULL);

	uint32_t sum = 0;
	size_t remaining_bytes_count = size;
	uint16_t *iter = addr;

	while (remaining_bytes_count >= 2) {
		sum += *iter;
		if (sum > 0xffff) {
			sum -= 0xffff;
		}
		iter++;
		remaining_bytes_count -= 2;
	}

	if (remaining_bytes_count == 1) {
		sum += *(uint8_t *)iter;
		if (sum > 0xffff) {
			sum -= 0xffff;
		}
	}
	return ~sum;
}
