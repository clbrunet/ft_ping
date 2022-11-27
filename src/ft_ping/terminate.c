#include <stdlib.h>
#include <unistd.h>

#include "ft_ping/terminate.h"
#include "ft_ping/main.h"

void terminate(void)
{
	free(g_ping.icmp_reply_buf);
	g_ping.icmp_reply_buf = NULL;
	free(g_ping.icmp_request);
	g_ping.icmp_request = NULL;
	close(g_ping.socket_fd);
	g_ping.socket_fd = -1;
}
