#ifndef SEMAPHORE_H
#define SEMAPHORE_H

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

#define SEM_INIT(S, N)        (S) = semget(IPC_PRIVATE, (N), 0400 | 0200 | IPC_CREAT | IPC_EXCL)
#define SEM_POSTVAL(S, N, V)  { struct sembuf postval = {.sem_num = (N), .sem_op =         (V), .sem_flg = 0}; semop((S), &postval, 1); }
#define SEM_WAITVAL(S, N, V)  { struct sembuf waitval = {.sem_num = (N), .sem_op = -(short)(V), .sem_flg = 0}; semop((S), &waitval, 1); }
#define SEM_POST(S, N)        { struct sembuf post    = {.sem_num = (N), .sem_op =           1, .sem_flg = 0}; semop((S), &post,    1); }
#define SEM_WAIT(S, N)        { struct sembuf wait    = {.sem_num = (N), .sem_op =          -1, .sem_flg = 0}; semop((S), &wait,    1); }
#define SEM_GETVAL(S, N)      semctl((S), (N), GETVAL)
#define SEM_SETVAL(S, N, V)   { union semun setval = {.val = (V)}; semctl((S), (N), SETVAL, setval); }
#define SEM_DESTROY(S)        semctl((S), 0, IPC_RMID)

#endif
