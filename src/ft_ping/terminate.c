#include <stdlib.h>
#include <unistd.h>

#include "ft_ping/terminate.h"
#include "ft_ping/main.h"

void terminate(void)
{
	free(g_vars.icmp_reply_buf);
	g_vars.icmp_reply_buf = NULL;
	free(g_vars.icmp_request);
	g_vars.icmp_request = NULL;
	close(g_vars.socket_fd);
	g_vars.socket_fd = -1;
}
