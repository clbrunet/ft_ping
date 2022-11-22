#include "ft_ping/utils/string.h"

size_t ft_strlen(const char *s)
{
	const char *start = s;
	while (*s != '\0') {
		s++;
	}
	return s - start;
}

size_t ft_strlcpy(char *dst, const char *src, size_t dstsize)
{
	if (src == NULL) {
		return 0;
	}
	size_t src_len = ft_strlen(src);
	if (dstsize == 0 || dst == NULL) {
		return src_len;
	}
	dstsize--;
	while (*src != '\0' && dstsize > 0) {
		*dst = *src;
		dst++;
		src++;
		dstsize--;
	}
	*dst = 0;
	return src_len;
}
