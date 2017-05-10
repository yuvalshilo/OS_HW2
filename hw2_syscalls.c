
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>

int sys_is_short(pid_t pid) {
    struct task_struct* ts = find_task_by_pid(pid);
    if (!ts) {
        return -ESRCH;
    }
    if (ts->policy != SCHED_SHORT) {
        return -EINVAL;
    }
    return  (ts->is_overdue == OVERDUE ? 0 : 1);
}


int sys_short_remaining_time(pid_t pid) {
    struct task_struct* ts = find_task_by_pid(pid);
    if (!ts) {
        return -ESRCH;
    }
    if (ts->policy != SCHED_SHORT) {
        return -EINVAL;
    }

    return ts->time_slice;
}

int sys_short_place_in_queue(pid_t pid)
{
    struct task_struct* ts = find_task_by_pid(pid);
    if (!ts) {
        return -ESRCH;
    }
    if (ts->policy != SCHED_SHORT) {
        return -EINVAL;
    }
    int cnt = 0;
    int prio = ts->prio;
    int is_overdue = ts->is_overdue;
    
    for_each_task(p) {
        if (p->is_overdue == is_overdue && p->short_prio <= prio) cnt++
    }

    return (cnt - 1 /* without this task */);
}
