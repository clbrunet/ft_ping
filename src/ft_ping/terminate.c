#include <stdlib.h>
#include <unistd.h>

#include "ft_ping/terminate.h"
#include "ft_ping/main.h"

void terminate(void)
{
	free(g_vars.icmp_request);
	close(g_vars.socket_fd);
}
