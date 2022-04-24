#include <stdio.h>

void print_error(const char *executable, const char *error_title, const char *error_description)
{
	fprintf(stderr, "%s: %s: %s\n", executable, error_title, error_description);
}

