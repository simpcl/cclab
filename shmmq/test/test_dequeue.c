#include <stdio.h>

#include <shmmq.h>

int main()
{
	int ret = 0;

	ret = shmmq_open(1, 0, 0);
	if (ret) {
		printf("shmmq_open is error: %d", ret);
		return -1;
	}

	return 0;
}
