#include <c_type.h>

int isalpha(int c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int isdigit(int c) {
	return (c >= '0' && c <= '9');
}
