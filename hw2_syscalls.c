
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/list.h>

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

    list_t* head = ((prio_array_t *)(ts->array))->queue + prio;
    
    /* same priority */
    while (list_entry(head = head->prev, task_t, run_list) != ts) cnt++;
    prio--;
    /* else */
    for (;prio >= MAX_RT_PRIO; prio--)
        cnt += list_size(((prio_array_t *)(ts->array))->queue + prio);
    /* TODO: check if ok with overdue */
    return cnt;
}
