#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "semutil.h"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int get_semvalue(key_t key)
{
    int semid;
    semid = semget(key, 1, 0666|IPC_CREAT);
    if (semid < 0) {
        return -1;
    }

    return semid;  
}

int set_semvalue(int semid)
{
    union semun sem_union;

    sem_union.val = 1;
    if (-1 == semctl(semid, 0, SETVAL, sem_union)) {
        return -1;
    }

    return 0;
}

int del_semvalue(int semid)
{
    union semun sem_union;
    if (-1 == semctl(semid, 0, IPC_RMID, sem_union)) {
        return -1;
    }

    return 0;
}

int semaphore_p(int semid)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (-1 == semop(semid, &sem_b, 1)) {
        return -1;
    }

    return 0;
}

int semaphore_v(int semid)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (-1 == semop(semid, &sem_b, 1)) {
        return -1;
    }

    return 0;
}



