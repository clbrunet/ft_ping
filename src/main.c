#include <stdio.h>

int main (int argc, char *argv[])
{
	if (argc == 1) {
		fprintf(stderr, "ft_ping: usage error: Destination address required\n");
		return 1;
	}
	(void)argv;
	return 0;
}
