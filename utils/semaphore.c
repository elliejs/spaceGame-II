#include "semaphore.h"

extern inline
int SEM_INIT(int N);
extern inline
int SEM_POSTVAL(int S, unsigned short N, short V);
extern inline
int SEM_WAITVAL(int S, unsigned short N, short V);
extern inline
int SEM_POST(int S, unsigned short N);
extern inline
int SEM_WAIT(int S, unsigned short N);
extern inline
int SEM_GETVAL(int S, int N);
extern inline
int SEM_SETVAL(int S, int N, int V);
extern inline
int SEM_DESTROY(int S);
extern inline
int MTX_INIT(pthread_mutex_t * X);
extern inline
int MTX_LOCK(pthread_mutex_t * X);
extern inline
int MTX_UNLOCK(pthread_mutex_t * X);
extern inline
int MTX_DESTROY(pthread_mutex_t * X);
extern inline
int COND_INIT(pthread_cond_t * X);
extern inline
int COND_SIGNAL(pthread_cond_t * X);
extern inline
int COND_BROADCAST(pthread_cond_t * X);
extern inline
int COND_WAIT(pthread_cond_t * X, pthread_mutex_t * Y);
extern inline
int COND_DESTROY(pthread_cond_t * X);
extern inline
int RWLOCK_INIT(pthread_rwlock_t * X);
extern inline
int RWLOCK_RLOCK(pthread_rwlock_t * X);
extern inline
int RWLOCK_RUNLOCK(pthread_rwlock_t * X);
extern inline
int RWLOCK_WLOCK(pthread_rwlock_t * X);
extern inline
int RWLOCK_WUNLOCK(pthread_rwlock_t * X);
extern inline
int RWLOCK_DESTROY(pthread_rwlock_t * X);
