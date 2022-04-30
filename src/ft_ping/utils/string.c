#include "ft_ping/utils/string.h"

size_t ft_strlen(const char *s)
{
	const char *start = s;
	while (*s != '\0') {
		s++;
	}
	return s - start;
}
