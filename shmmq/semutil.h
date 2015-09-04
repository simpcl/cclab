#ifndef _SEMUTIL_H_
#define _SEMUTIL_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#ifdef __cplusplus
extern "C" {
#endif

int get_semvalue(key_t key);
int set_semvalue(int semid);
int del_semvalue(int semid);
int semaphore_p(int semid);
int semaphore_v(int semid);

#ifdef __cplusplus
}
#endif


#endif // SEMUTIL_H
