#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/sem.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

//unless you're apple
#if !defined(__APPLE__)
//The calling program must define this union as follows:
union semun {
  int              val;    /* Value for SETVAL */
  struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
  unsigned short  *array;  /* Array for GETALL, SETALL */
  struct seminfo  *__buf;  /* Buffer for IPC_INFO
                              (Linux-specific) */
};
#endif

inline
int SEM_INIT(int N) {
  int S = semget(IPC_PRIVATE, N, 0400 | 0200 | IPC_CREAT | IPC_EXCL);
  #if defined(SG_DEBUG)
    if(S == -1) perror("[ERROR] SEM INIT");
  #endif
  return S;
}

inline
int SEM_POSTVAL(int S, int N, short V) {
  struct sembuf postval = {.sem_num = N, .sem_op = (short) V, .sem_flg = 0};
  int R = semop(S, &postval, 1);
  #if defined(SG_DEBUG)
    if (R == -1) perror("[ERROR] SEM POSTVAL");
  #endif
  return R;
}

inline
int SEM_WAITVAL(int S, int N, short V) {
  struct sembuf waitval = {.sem_num = N, .sem_op = (short) -V, .sem_flg = 0};
  int R = semop(S, &waitval, 1);
  #if defined(SG_DEBUG)
    if (R == -1) perror("[ERROR] SEM WAITVAL");
  #endif
  return R;
}

inline
int SEM_POST(int S, int N) {
  struct sembuf postval = {.sem_num = N, .sem_op = (short) 1, .sem_flg = 0};
  int R = semop(S, &postval, 1);
  #if defined(SG_DEBUG)
    if (R == -1) perror("[ERROR] SEM POST");
  #endif
  return R;
}

inline
int SEM_WAIT(int S, int N) {
  struct sembuf waitval = {.sem_num = N, .sem_op = (short) -1, .sem_flg = 0};
  int R = semop(S, &waitval, 1);
  #if defined(SG_DEBUG)
    if (R == -1) perror("[ERROR] SEM WAIT");
  #endif
  return R;
}

inline
int SEM_GETVAL(int S, int N) {
  int R = semctl(S, N, GETVAL);
  #if defined(SG_DEBUG)
    if (R == -1) perror("[ERROR] SEM GET VALUE");
  #endif
  return R;
}

inline
int SEM_SETVAL(int S, int N, int V) {
  union semun setval = {.val = V};
  int R = semctl(S, N, SETVAL, setval);
  #if defined(SG_DEBUG)
    if (R == -1) perror("[ERROR] SEM SET VALUE");
  #endif
  return R;
}

inline
int SEM_DESTROY(int S) {
  int R = semctl(S, 0, IPC_RMID);
  #if defined(SG_DEBUG)
    if (R == -1) perror("[ERROR] SEM SET VALUE");
  #endif
  return R;
}

inline
int MTX_INIT(pthread_mutex_t * X) {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
// #if defined(__APPLE__)
//   pthread_mutexattr_setpolicy_np(&attr, PTHREAD_MUTEX_POLICY_FAIRSHARE_NP);
// #endif
  // pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
  pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

  int R = pthread_mutex_init(X, &attr);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] MUTEX INIT: %s\n", strerror(R));
  #endif
  return R;
}

inline
int MTX_LOCK(pthread_mutex_t * X) {
  int R = pthread_mutex_lock(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] MUTEX LOCK: %s\n", strerror(R));
  #endif
  return R;
}

inline
int MTX_UNLOCK(pthread_mutex_t * X) {
  int R = pthread_mutex_unlock(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] MUTEX UNLOCK: %s\n", strerror(R));
  #endif
  return R;
}

inline
int MTX_DESTROY(pthread_mutex_t * X) {
  int R = pthread_mutex_destroy(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] MUTEX DESTROY: %s\n", strerror(R));
  #endif
  return R;
}

inline
int COND_INIT(pthread_cond_t * X) {
  pthread_condattr_t attr;
  pthread_condattr_init(&attr);
  pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  int R = pthread_cond_init(X, &attr);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] COND INIT: %s\n", strerror(R));
  #endif
  return R;
}

inline
int COND_SIGNAL(pthread_cond_t * X) {
  int R = pthread_cond_signal(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] COND SIGNAL: %s\n", strerror(R));
  #endif
  return R;
}

inline
int COND_BROADCAST(pthread_cond_t * X) {
  int R = pthread_cond_broadcast(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] COND BROADCAST: %s\n", strerror(R));
  #endif
  return R;
}

inline
int COND_WAIT(pthread_cond_t * X, pthread_mutex_t * Y) {
  int R = pthread_cond_wait(X, Y);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] COND WAIT: %s\n", strerror(R));
  #endif
  return R;
}

inline
int COND_DESTROY(pthread_cond_t * X) {
  int R = pthread_cond_destroy(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] COND WAIT: %s\n", strerror(R));
  #endif
  return R;
}

inline
int RWLOCK_INIT(pthread_rwlock_t * X) {
  pthread_rwlockattr_t attr;
  pthread_rwlockattr_init(&attr);
  pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
  int R = pthread_rwlock_init(X, &attr);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] WRLOCK INIT: %s\n", strerror(R));
  #endif
  return R;
}

inline
int RWLOCK_RLOCK(pthread_rwlock_t * X) {
  int R = pthread_rwlock_rdlock(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] RWLOCK RLOCK: %s\n", strerror(R));
  #endif
  return R;
}

inline
int RWLOCK_RUNLOCK(pthread_rwlock_t * X) {
  int R = pthread_rwlock_unlock(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] RWLOCK RUNLOCK: %s\n", strerror(R));
  #endif
  return R;
}

inline
int RWLOCK_WLOCK(pthread_rwlock_t * X) {
  int R = pthread_rwlock_wrlock(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] RWLOCK WLOCK: %s\n", strerror(R));
  #endif
  return R;
}

inline
int RWLOCK_WUNLOCK(pthread_rwlock_t * X) {
  int R = pthread_rwlock_unlock(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] RWLOCK WUNLOCK: %s\n", strerror(R));
  #endif
  return R;
}

inline
int RWLOCK_DESTROY(pthread_rwlock_t * X) {
  int R = pthread_rwlock_destroy(X);
  #if defined(SG_DEBUG)
    if (R != 0) printf("[ERROR] RWLOCK DESTROY: %s\n", strerror(R));
  #endif
  return R;
}



#endif /* end of include guard: SEMAPHORE_H */
