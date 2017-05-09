#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sched.h"
#include "hw2_syscalls.h"

struct sched_param {
    int sched_priority;
    int requested_time;
    int sched_short_prio;
};

int main() {
    
    int pid = getpid();
    struct sched_param sp;
    sp.sched_priority = 0;
    sp.requested_time = 1;
    sp.sched_short_prio = 1;
    
    sys_sched_setscheduler(pid, SCHED_SHORT, &sp);
    
    sleep(5);

    
    printf("is_short = %d\n",is_short(pid));
    printf("short_remaining_time = %d\n",short_remaining_time(pid));
    printf("short_place_in_queue = %d\n",short_place_in_queue(pid));
    
    return 0;
}
