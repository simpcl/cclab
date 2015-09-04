#include <stdio.h>

#include <shmmq.h>

int main()
{
	int ret = 0;

	ret = shmmq_close(1);
	if (ret) {
		printf("shmmq_close is error: %d", ret);
		return -1;
	}

	return 0;
}
