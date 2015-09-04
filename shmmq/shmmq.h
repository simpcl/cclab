#ifndef _SHMMQ_
#define _SHMMQ_


#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int (*SF)(unsigned int begin, unsigned int end);

int shmmq_open(int iscreated, int mqsize, int itemsize);
int shmmq_close(int isdestroyed);
int shmmq_enqueue(void *pitem, int size);
int shmmq_dequeue(void *pitem, int size);
int shmmq_clear(SF sf);
const char* shmmq_get_errstr(int errno);


#ifdef __cplusplus
}
#endif


#endif // _SHMMQ_
