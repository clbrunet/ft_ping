#ifndef FT_PING_MAIN_H
#define FT_PING_MAIN_H

#include <bits/types/struct_timeval.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <netinet/in.h>
#include <netdb.h>

#define EWMA_FACTOR 8

typedef struct
{
	struct sockaddr_in sockaddr_in;
	char name[NI_MAXHOST];
	char ip[INET_ADDRSTRLEN];
} destination_t;

typedef struct
{
	const char *program_name;
	int64_t packet_count;
	bool should_print_timestamp;
	bool is_quiet;
	bool is_verbose;
	size_t icmp_payload_size;
	bool is_ttl_specified;
	int ttl;
	destination_t destination;

	int socket_fd;
	uint16_t icmp_request_id;
	uint8_t *icmp_request;
	size_t icmp_reply_buf_size;
	uint8_t *icmp_reply_buf;

	bool should_stop_on_alarm;
	size_t transmitted_packets_count;
	size_t received_packets_count;
	size_t error_packets_count;
	struct timeval first_sending_time;
	size_t ms_from_first_sending_time;
	double min_rtt;
	double rtt_sum;
	double max_rtt;
	double squared_rtt_sum;
	double ewma;
} ping_t;

extern ping_t g_ping;

void interrupt_handler(int signum);

#endif
