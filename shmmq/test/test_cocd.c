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
	int pid = 0;
	int i;

	ret = shmmq_open(1, 0, 0);
	if (ret) {
		printf("parent process - shmmq_open is failed: %d\n", ret);
		return -1;
	} else {
		printf("parent process - shmmq_open is sucessful\n");
	}

	pid = fork();
	if (pid == -1) {
		printf("parent process - Failed to fork a child process!\n");
		return -1;
	} else if (pid == 0) {
		printf("child process - is beginning ... \n");

		ret = shmmq_open(0, 0, 0);
		if (ret) {
			printf("child process - shmmq_open is failed: %d\n", ret);
			return -1;
		} else {
			printf("child process - shmmq_open is sucessful\n");
		}


		for (i=0; i<10; i++) {
			ret = shmmq_enqueue(&i, sizeof(i));
			printf("child process - shmmq_enqueue: (%d, %d), %d, %d\n", 
					shmmq_get_enq_pos(), shmmq_get_deq_pos(), i, ret);
		}

		ret = shmmq_close(0);
		if (ret) {
			printf("child process - shmmq_close is failed: %d\n", ret);
			return -1;
		} else {
			printf("child process - shmmq_close is successful\n");
		}

		printf("child process - is end ... \n");
	}


	for (i=0; i<10; i++) 
	{
		int tmp;
		sleep(1);
		ret = shmmq_dequeue(&tmp, sizeof(tmp));
		printf("parent process - shmmq_dequeue: (%d, %d), %d, %d\n", 
					shmmq_get_enq_pos(), shmmq_get_deq_pos(), tmp, ret);
	}

	int status;
	waitpid(pid, &status, 0); 

	ret = shmmq_close(1);
	if (ret) {
		printf("parent process - shmmq_close is failed: %d\n", ret);
		return -1;
	} else {
		printf("parent process - shmmq_close is successful\n");
	}


	return 0;
}
