#include "libft.h"
#include <stdio.h>
int main() {
	printf("%lx %lx\n", 0xff, ft_atol_base("ff", "0123456789abcdef"));
	printf("%lx %lx\n", 0x1a2d0, ft_atol_base("1a2d0-", "0123456789abcdef"));
	printf("%lx %lx\n", 0x1b2e0, ft_atol_base("1b2e0-", "0123456789abcdef"));
	return 0;
}
