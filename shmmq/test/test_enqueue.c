#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

#include <shmmq.h>

const char* shmmq_get_errstr(int errno);
int shmmq_get_enq_pos();
int shmmq_get_deq_pos();
void shmmq_set_enq_pos(unsigned int pos);
void shmmq_set_deq_pos(unsigned int pos);

int main()
{
	int ret = 0;
	int i = 0;

	ret = shmmq_open(1, 0, 0);
	if (ret) {
		printf("shmmq_open is failed: %d\n", ret);
		return -1;
	} else {
		printf("shmmq_open is sucessful\n");
	}


	ret = shmmq_enqueue(&i, sizeof(i));
	printf("shmmq_enqueue: (%u, %u), %d, %d\n", 
			shmmq_get_enq_pos(), 
			shmmq_get_deq_pos(), 
			i, 
			ret);

	i++;
	shmmq_set_enq_pos(3);
	shmmq_set_deq_pos(3);
	ret = shmmq_enqueue(&i, sizeof(i));
	printf("shmmq_enqueue: (%u, %u), %d, %d\n", 
			shmmq_get_enq_pos(), 
			shmmq_get_deq_pos(), 
			i, 
			ret);

	i++;
	shmmq_set_enq_pos(1023);
	shmmq_set_deq_pos(1023);
	ret = shmmq_enqueue(&i, sizeof(i));
	printf("shmmq_enqueue: (%u, %u), %d, %d\n", 
			shmmq_get_enq_pos(), 
			shmmq_get_deq_pos(), 
			i, 
			ret);

	i++;
	shmmq_set_enq_pos(1024);
	shmmq_set_deq_pos(1);
	ret = shmmq_enqueue(&i, sizeof(i));
	printf("shmmq_enqueue: (%u, %u), %d, %d\n", 
			shmmq_get_enq_pos(), 
			shmmq_get_deq_pos(), 
			i, 
			ret);

	i++;
	shmmq_set_enq_pos(~0);
	shmmq_set_deq_pos((~0)-1022);
	ret = shmmq_enqueue(&i, sizeof(i));
	printf("shmmq_enqueue: (%u, %u), %d, %d\n", 
			shmmq_get_enq_pos(), 
			shmmq_get_deq_pos(), 
			i, 
			ret);

	i++;
	shmmq_set_enq_pos(1022);
	shmmq_set_deq_pos((unsigned int)~0);
	ret = shmmq_enqueue(&i, sizeof(i));
	printf("shmmq_enqueue: (%u, %u), %d, %d\n", 
			shmmq_get_enq_pos(), 
			shmmq_get_deq_pos(), 
			i, 
			ret);

	
	ret = shmmq_close(1);
	if (ret) {
		printf("shmmq_close is failed: %d\n", ret);
		return -1;
	} else {
		printf("shmmq_close is successful\n");
	}


	return 0;
}
