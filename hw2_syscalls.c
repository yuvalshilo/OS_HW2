
int sys_is_short(pid_t pid) {
    struct task_t* ts = find_task_by_pid(pid);
    if (!ts) {
        return -ESRCH;
    }
    if (ts->policy != SCHED_SHORT) {
        return -EINVAL
    }
    return  (ts->is_overdue == OVERDUE ? 0 : 1);
}

int sys_short_remaining_time(pid_t pid) {
    struct task_t* ts = find_task_by_pid(pid);
    if (!ts) {
        return -ESRCH;
    }
    if (ts->policy != SCHED_SHORT) {
        return -EINVAL
    }

    return ts->time_slice;
}
