#include <assert.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#include "ft_ping/checksum.h"

static uint16_t get_data_sum(uint16_t *addr, size_t size)
{
	assert(addr != NULL);

	uint32_t sum = 0;
	size_t remaining_bytes_count = size;

	while (remaining_bytes_count >= 2) {
		sum += *addr;
		if (sum > UINT16_MAX) {
			sum = (sum & UINT16_MAX) + (sum >> 16);
		}
		addr++;
		remaining_bytes_count -= 2;
	}

	if (remaining_bytes_count == 1) {
		sum += *(uint8_t *)addr;
		if (sum > UINT16_MAX) {
			sum = (sum & UINT16_MAX) + (sum >> 16);
		}
	}
	return sum;
}

uint16_t get_checksum(void *addr, size_t size)
{
	assert(addr != NULL);

	return ~get_data_sum((uint16_t *)addr, size);
}

bool is_checksum_valid(void *addr, size_t size, uint16_t *checksum_ptr)
{
	assert(addr != NULL);
	assert(addr <= (void *)checksum_ptr && (void *)checksum_ptr < addr + size);

	bool is_checksum_valid = true;
	uint16_t checksum_backup = *checksum_ptr;
	*checksum_ptr = 0;
	if (get_data_sum(addr, size) + checksum_backup != UINT16_MAX) {
		is_checksum_valid = false;
	}
	*checksum_ptr = checksum_backup;
	return is_checksum_valid;
}
