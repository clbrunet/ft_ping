#include "ft_ping/utils/error.h"

const char *ft_gai_strerror(int errcode)
{
	switch (errcode) {
		case -105:
			return "Parameter string not correctly encoded";
		case -104:
			return "Interrupted by a signal";
		case -103:
			return "All requests done";
		case -102:
			return "Request not canceled";
		case -101:
			return "Request canceled";
		case -100:
			return "Processing request in progress";
		case -11:
			return "System error";
		case -10:
			return "Memory allocation failure";
		case -9:
			return "Address family for hostname not supported";
		case -8:
			return "Servname not supported for ai_socktype";
		case -7:
			return "ai_socktype not supported";
		case -6:
			return "ai_family not supported";
		case -5:
			return "No address associated with hostname";
		case -4:
			return "Non-recoverable failure in name resolution";
		case -3:
			return "Temporary failure in name resolution";
		case -2:
			return "Name or service not known";
		case -1:
			return "Bad value for ai_flags";
		default:
			return "Unknown error";
	}
}
