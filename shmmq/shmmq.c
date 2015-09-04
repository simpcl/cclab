
#include "shmmq.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define DEFAULT_MQ_SIZE         1024
#define DEFAULT_ITEM_SIZE       256
#define DEFAULT_HEAD_SIZE	4096
#define SEM_KEY			(1024)
#define SHM_KEY			(1025)


typedef struct _shmmq_head {
	unsigned int enq_pos;
	unsigned int deq_pos;
} shmmq_head;


static int		g_s_semid	= -1;
static int		g_s_shmid	= -1;
static int		g_s_mqsize	= DEFAULT_MQ_SIZE;
static int		g_s_itemsize	= DEFAULT_ITEM_SIZE;
static void*		g_s_pbase	= NULL;
static shmmq_head*	g_s_phead	= NULL;
static void*		g_s_pcontent	= NULL;

static char* errlist[] = {
	"Successful"
	"Invalid Argument",
	"Failed in get_semvalue",
	"Failed in set_semvalue",
	"Failed in del_semvalue",
	"Failed to shmget",
	"Failed to shmat",
	"Failed to shmdt",
	"Failed to shmctl",
	"Failed in semaphore_p",
	"Failed in semaphore_v",
	"Msg Queue Full"
};

enum {
	SUCCESS = 0,
	ERROR_INVALID_ARGUMENT,
	ERROR_GET_SEMVALUE,
	ERROR_SET_SEMVALUE,
	ERROR_DEL_SEMVALUE,
	ERROR_SHMGET,
	ERROR_SHMAT,
	ERROR_SHMDT,
	ERROR_SHMCTL,
	ERROR_SEMAPHORE_P,
	ERROR_SEMAPHORE_V,
	ERROR_MSGQUEUE_FULL
};


/************************************************************
 * function:  create or open the shared memory.
 * iscreated: 1, to create the shm; 0, only to open the shm.
 * mqsize:    the count of the items
 * itemsize:  the size of an item (bytes)
 * return:    0, correct; others, error number.
 ***********************************************************/
int shmmq_open(int iscreated, int mqsize, int itemsize)
{
	int nflag;
	int ret = 0;

	/* Initial semaphore */
	g_s_semid = get_semvalue((key_t)SEM_KEY);
	if (-1 == g_s_semid) {
		return ERROR_GET_SEMVALUE;
	}
	if (iscreated && -1 == set_semvalue(g_s_semid)) {
		return ERROR_SET_SEMVALUE;
	}

	if (-1 == semaphore_p(g_s_semid)) {
		return ERROR_SEMAPHORE_P;
	}
	while (1) 
	{
		if (iscreated) {
			nflag = 0666 | IPC_CREAT;
		} else {
			nflag = 0666;
		}

		g_s_mqsize = (mqsize <= 0) ? DEFAULT_MQ_SIZE : mqsize;
		g_s_itemsize = (itemsize <= 0) ? DEFAULT_ITEM_SIZE : itemsize;

		/* Initial shared memory */
		g_s_shmid = shmget((key_t)SHM_KEY, 
			g_s_mqsize * g_s_itemsize + DEFAULT_HEAD_SIZE,
			nflag);
		if (-1 == g_s_shmid) {
			ret = ERROR_SHMGET;
			break;
		}

		g_s_pbase = shmat(g_s_shmid, NULL, 0);
		if ((void *)-1 == g_s_pbase) {
			g_s_pbase = NULL;
			ret = ERROR_SHMAT;
			break;
		}

		g_s_phead = (shmmq_head*)g_s_pbase;
		g_s_pcontent = g_s_pbase + DEFAULT_HEAD_SIZE;

		/* Initial head of the shared memory  */
		if (iscreated) {
			g_s_phead->enq_pos = -1;
			g_s_phead->deq_pos = -1;
		}
		break;
	}
	if (ret && iscreated) {
		if (g_s_shmid >= 0) shmctl(g_s_shmid, IPC_RMID, 0);
		semaphore_v(g_s_semid);
		del_semvalue(g_s_semid);
		return ret;
	}

	if (-1 == semaphore_v(g_s_semid)) {
		return ERROR_SEMAPHORE_V;
	}

	return ret;
}


/************************************************************
 * function:  destroy or close the shared memory.
 * iscreated: 1, to destroy the shm; 0, only to close the shm.
 * return:    0, correct; others, error number.
 ***********************************************************/
int shmmq_close(int isdestroyed)
{
	int ret = 0;

	if (g_s_pbase) {
		if (-1 == shmdt(g_s_pbase)) {
			ret = ERROR_SHMDT;
		}
		g_s_pbase = NULL;
	}

	if (!isdestroyed) return ret;

	if (g_s_shmid >= 0) {
		if (shmctl(g_s_shmid, IPC_RMID, 0) == -1) {
			ret = ERROR_SHMCTL;
        	}
		g_s_shmid = -1;
	}

	if (g_s_semid >= 0) {
		if (-1 == del_semvalue(g_s_semid)) {
			ret = ERROR_DEL_SEMVALUE;
		}
		g_s_semid = -1;
	}

	return ret;
}


/************************************************************
 * function:  enqueueing.
 * item:      a pointer to the element to enqueue
 * size:      the size of the element
 * return:    0, correct; others, error number.
 ***********************************************************/
int shmmq_enqueue(void* item, int size)
{
	int ret = 0;

	if (!item || size <= 0 || size > g_s_itemsize) {
		return ERROR_INVALID_ARGUMENT;
	}

	if (-1 == semaphore_p(g_s_semid)) {
		return ERROR_SEMAPHORE_P;
	}

/*	if (g_s_phead->enq_pos < g_s_phead->deq_pos) {
		unsigned int a = ~0;
		a -= g_s_phead->deq_pos;
		if (a + g_s_phead->enq_pos + 2 >= g_s_mqsize) {
			ret = ERROR_MSGQUEUE_FULL;	
			goto queue_full;
		}
	} else if (g_s_phead->enq_pos == g_s_phead->deq_pos + g_s_mqsize -1) {
		ret = ERROR_MSGQUEUE_FULL;
		goto queue_full;
	}
*/
	if (g_s_phead->deq_pos + g_s_mqsize - 1 == g_s_phead->enq_pos) {
		ret = ERROR_MSGQUEUE_FULL;
		goto queue_full;
	}

	unsigned int pos = (g_s_phead->enq_pos+1) % g_s_mqsize;
	memcpy(g_s_pcontent + pos * g_s_itemsize, item, size);
	g_s_phead->enq_pos++;
	
queue_full:

	if (-1 == semaphore_v(g_s_semid)) {
		return ERROR_SEMAPHORE_V;
	}

	return ret;
}


/************************************************************
 * function:  dequeueing.
 * item:      a pointer to the element to dequeue
 * size:      the size of the element
 * return:    0, correct; others, error number.
 ***********************************************************/
int shmmq_dequeue(void* item, int size)
{
	int ret = 0;
	
	if (!item || size <= 0 || size > g_s_itemsize) {
		return ERROR_INVALID_ARGUMENT;
	}

	if (-1 == semaphore_p(g_s_semid)) {
		return ERROR_SEMAPHORE_P;
	}

	if (g_s_phead->enq_pos == g_s_phead->deq_pos) {
		ret = ERROR_MSGQUEUE_FULL;
		goto queue_full;
	}

	unsigned int pos = (g_s_phead->deq_pos+1) % g_s_mqsize;
	memcpy(item, g_s_pcontent + pos * g_s_itemsize, size);
	g_s_phead->deq_pos++;
	

queue_full:

	if (-1 == semaphore_v(g_s_semid)) {
		return ERROR_SEMAPHORE_V;
	}
	
	return ret;
}


/************************************************************
 * function:  clear the overdue items from the queue.
 * sf:        a pointer to function which can calculate the critical index
 * return:    0, correct; others, error number.
 ***********************************************************/
int shmmq_clear(SF sf)
{
	int ret = 0;
	int begin;
	int end;

	if (sf == NULL) {
		return ERROR_INVALID_ARGUMENT;
	}


	if (-1 == semaphore_p(g_s_semid)) {
		return ERROR_SEMAPHORE_P;
	}

	begin = g_s_phead->deq_pos % g_s_mqsize;
	end = g_s_phead->enq_pos % g_s_mqsize;

	begin = (*sf)(begin, end);
	if (begin < 0 || begin >= g_s_mqsize) {
		ret = ERROR_INVALID_ARGUMENT;
	}

	if (-1 == semaphore_v(g_s_semid)) {
		return ERROR_SEMAPHORE_V;
	}

	return ret;
}

const char* shmmq_get_errstr(int errno)
{
	if (errno < 0 || errno > 11) return NULL;
	return errlist[errno];
}

int shmmq_get_enq_pos()
{
	return g_s_phead->enq_pos;
}

int shmmq_get_deq_pos()
{
	return g_s_phead->deq_pos;
}

void shmmq_set_enq_pos(unsigned int pos)
{
	g_s_phead->enq_pos = pos;
}

void shmmq_set_deq_pos(unsigned int pos)
{
	g_s_phead->deq_pos = pos;
}

