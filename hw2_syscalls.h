

#ifndef hw2_syscalls_h
#define hw2_syscalls_h

typedef struct sched_param {
    int sched_priority, requested_time, sched_short_prio;
} sched_param_t;

int is_short(pid_t pid) {
    unsigned int res;
    __asm__ (
             "int $0x80;"
             : "=a" (res)
             : "0" (243), "b" (pid)
             : "memory"
             );
    if (res >= (unsigned long)(- 125)) {
        errno = -res;
        res = - 1;
    }
    return   (int)  res ;
}

int short_remaining_time(pid_t pid)   {
    unsigned int res;
    __asm__ (
             "int $0x80;"
             : "=a" (res)
             : "0" (244), "b" (pid)
             : "memory"
             );
    if (res >= (unsigned long)(- 125)) {
        errno = -res;
        res = - 1;
    }
    return   (int)  res ;
}

int short_place_in_queue(pid_t pid)   {
    unsigned int res;
    __asm__ (
             "int $0x80;"
             : "=a" (res)
             : "0" (245), "b" (pid)
             : "memory"
             );
    if (res >= (unsigned long)(- 125)) {
        errno = -res;
        res = - 1;
    }
    return   (int) res ;
}


#endif /* hw2_syscalls_h */
