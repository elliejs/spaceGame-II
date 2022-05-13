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
  pthread_mutexattr_setpolicy_np(&attr, PTHREAD_MUTEX_POLICY_FAIRSHARE_NP);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
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

#endif /* end of include guard: SEMAPHORE_H */
