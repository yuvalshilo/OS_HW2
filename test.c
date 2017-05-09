#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include "hw2_syscalls.h"

struct sched_param {
    int sched_priority;
    int requested_time;
    int sched_short_prio;
};

int main() {
    
    int pid = getpid();
    struct sched_param sp;
    
    sched_getparam(pid, &sp);
    
    sp.sched_priority = 0;
    sp.requested_time = 1;
    sp.sched_short_prio = 1;
    
    sched_setscheduler(pid, 5, &sp);
    
    sleep(2);
    
    printf("pid = %d\n", pid);
    printf("is_short = %d, errno = %d\n",is_short(pid), errno);
    printf("short_remaining_time = %d, errno = %d\n",short_remaining_time(pid), errno);
    printf("short_place_in_queue = %d, errno = %d\n",short_place_in_queue(pid), errno);
    
    return 0;
}
