#include <assert.h>
#include <stddef.h>

#include "ft_ping/ip.h"
#include "ft_ping/utils/inet.h"
#include "ft_ping/checksum.h"

bool is_iphdr_valid(struct iphdr *iphdr, uint16_t expected_total_length)
{
	assert(iphdr != NULL);
	assert(expected_total_length >= sizeof(struct iphdr));

	if (iphdr->version != IPVERSION) {
		return false;
	}
	if (iphdr->ihl != 5) {
		return false;
	}
	if (ft_ntohs(iphdr->tot_len) != expected_total_length) {
		return false;
	}
	if (ft_ntohs(iphdr->frag_off) != 0) {
		return false;
	}
	if (iphdr->protocol != IPPROTO_ICMP) {
		return false;
	}
	if (is_checksum_valid(iphdr, sizeof(struct iphdr), &iphdr->check) != true) {
		return false;
	}
	return true;
}
