//
// Created by alon on 5/13/2017.
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#include "hw2_syscalls.h"

#define KRED    "\x1b[31m" // test fail
#define KGRN    "\x1b[32m" // test success
#define KYEL    "\x1b[33m" // boaring info
#define KBLU    "\x1b[34m" // RUN TEST
#define KMAG    "\x1b[35m" // test explenations
#define KCYN    "\x1b[36m" // print lines
#define KWHT    "\x1b[0m"
#define BOLD    "\033[1m"
#define EBOLD   "\033[0m"

#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_SHORT     5

// use for debugging. add where ever you think you might have a bug to see if the line was executed.
#define DT      printf(KCYN "Line[%d] By Task[%d]\t" KWHT, __LINE__,getpid()); fflush(stdout);

#define EXPLAIN(msg) printf(KMAG "\t%s " KWHT,#msg);DT printf("\n"); fflush(stdout)
typedef struct sched_param {
    int sched_priority, requested_time, sched_short_prio;
} sched_param_t;

#define TEST_EQUALS(res_arg, a,b) 	if  (a != b) { \
                                        printf( KRED "Expected [%s == %s] at LINE[%d] and Failed \n"KWHT,#a,#b,__LINE__); \
										res_arg = false; \
									} else {    \
                                        printf( KGRN "\t\tExpected [%s == %s] at LINE[%d] and Passed \n"KWHT,#a,#b,__LINE__); \
                                    } fflush(stdout);

#define TEST_DIFFERENT(res_arg, a,b) 	if (a == b) { \
                                            printf( KRED "\t\tExpected [%s != %s] at LINE[%d] and Failed \n",#a,#b,__LINE__); \
                                            fflush(stdout);  \
											res_arg = false; \
										} else {    \
                                            printf( KGRN "\t\tExpected [%s != %s] at LINE[%d] and Passed \n"KWHT,#a,#b,__LINE__); \
                                        } fflush(stdout);

#define RUN_TEST(name)  printf( KBLU BOLD "Running %s ...\n" EBOLD KWHT, #name) ;	\
                        fflush(stdout);         \
						if (!name()) { \
							printf( KRED "[FAILED]\n" KWHT);		\
						}	else 							\
						    printf( KGRN "[SUCCESS]\n" KWHT); \
                        fflush(stdout);


void long_task() {
    int i,j;
    for (i=0;i<10000;++i) for (j=0;j<1000;++j);
}

bool test_sched_set_schedule(){
    bool res = true;
    int task_id = getpid();
    sched_param_t param;
    EXPLAIN(verify shced_getscheduler works);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);

    param.sched_priority = 0;
    param.requested_time = 10;
    param.sched_short_prio = 10;

    TEST_EQUALS(res,sched_setscheduler(task_id,SCHED_OTHER,&param),0);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);

    EXPLAIN(test passing bad parameters to set schedule.);
    EXPLAIN(bad param);
    TEST_EQUALS(res,sched_setscheduler(task_id,SCHED_SHORT,NULL),-1);
    TEST_EQUALS(res,errno,EINVAL);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);

    EXPLAIN(bad policy);
    TEST_EQUALS(res,sched_setscheduler(task_id,SCHED_SHORT+1,&param),-1);
    TEST_EQUALS(res,errno,EINVAL);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);

    EXPLAIN(bad short priority.);
    param.sched_short_prio = -1;
    TEST_EQUALS(res,sched_setscheduler(task_id,SCHED_SHORT,&param),-1);
    TEST_EQUALS(res,errno,EINVAL);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);
    param.sched_short_prio = 0;

    EXPLAIN(bad short priority.);
    param.sched_short_prio = 140;
    TEST_EQUALS(res,sched_setscheduler(task_id,SCHED_SHORT,&param),-1);
    TEST_EQUALS(res,errno,EINVAL);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);
    param.sched_short_prio = 0;

    EXPLAIN(bad requested time.);
    param.requested_time = 0;
    TEST_EQUALS(res,sched_setscheduler(task_id,SCHED_SHORT,&param),-1);
    TEST_EQUALS(res,errno,EINVAL);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);
    param.requested_time=10;

    EXPLAIN(bad requested time.);
    param.requested_time = 3001;
    TEST_EQUALS(res,sched_setscheduler(task_id,SCHED_SHORT,&param),-1);
    TEST_EQUALS(res,errno,EINVAL);
    TEST_EQUALS(res,sched_getscheduler(task_id), SCHED_OTHER);
    param.requested_time=10;

    EXPLAIN(bad pid);
    TEST_EQUALS(res,sched_setscheduler(task_id*10,SCHED_SHORT,&param),-1);
    TEST_EQUALS(res,errno,ESRCH);

    EXPLAIN(Forking in order to set the new short policy.);

    int status = 1, son = fork();
    if (!son) {
        param.sched_priority = 0;
        param.sched_short_prio = 100;
        param.requested_time   = 10;
        son = getpid();
        TEST_EQUALS(res,sched_setscheduler(son,SCHED_SHORT,&param),0);
        TEST_EQUALS(res,sched_getscheduler(son),SCHED_SHORT);

        status = res? 1 : 0;
        exit(status);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);

    return res;
}
bool test_is_short() {
    bool res = true;
    int father = getpid();
    sched_param_t param;

    EXPLAIN(father is not a short);
    TEST_EQUALS(res,is_short(father),-1);
    TEST_EQUALS(res,errno,EINVAL);
    EXPLAIN( bad pid);
    TEST_EQUALS(res,is_short(-1),-1); // TODO return ESRCH or EINVAL?
    EXPLAIN(no task with the given pid.);
    TEST_EQUALS(res,is_short(father*10),-1);
    TEST_EQUALS(res,errno,ESRCH);

    EXPLAIN(Forking in order to set the new short policy.);

    int status = 1, son = fork();
    if (!son) {
        param.sched_priority = 0;
        param.sched_short_prio = 100;
        param.requested_time   = 10;
        son = getpid();
        TEST_EQUALS(res,sched_setscheduler(son,SCHED_SHORT,&param),0);
        TEST_EQUALS(res,sched_getscheduler(son),SCHED_SHORT);

        EXPLAIN(son is a short and not overdue yet.);
        TEST_EQUALS(res,is_short(son),1);

        EXPLAIN(Son is short but not overdue-> doing a long task.);

        long_task();
        TEST_EQUALS(res,is_short(son),0);

        status = res? 1 : 0;
        exit(status);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);

    return res;
}

bool test_short_remaining_time(){
    bool res = true;
    sched_param_t param;
    int father = getpid();
    int i;

    EXPLAIN(father is not short.);
    TEST_EQUALS(res,short_remaining_time(father),-1);
    TEST_EQUALS(res,errno,EINVAL);

    EXPLAIN(there should not be a task with this pid.);
    TEST_EQUALS(res,short_remaining_time(father*100),-1);
    TEST_EQUALS(res,errno,ESRCH);

    EXPLAIN(Forking to test on a short son.);

    int status, son = fork();

    if (son == 0) {
        status = 1;
        son = getpid();
        int rq_time = 100;
        param.sched_priority = 0;
        param.sched_short_prio = rq_time;
        param.requested_time = rq_time;

        TEST_EQUALS(res, sched_setscheduler(son, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);

        printf(KMAG "\tSon[%d] is short not overdue and these are his remaining durations:\n" KWHT, son);
        printf(KMAG "\tSon[%d]'s durations should be contained in [ %d ... %d ].\n\t\t" KWHT,son, 0,rq_time);
        fflush(stdout);
        for (i = 0; (i < 10) && (is_short(son) == 1); ++i) {
            int time = short_remaining_time(son);
            printf(KYEL"--T[%d]=%d, " KWHT, i,time);
            fflush(stdout);
            long_task();
        }
        printf("\n");
        fflush(stdout);
        while(is_short(son)!=0) {
            printf(KMAG "\t\tDoing a long task.\n" KWHT, (int) res, status);
            fflush(stdout);
            long_task();
        }
        TEST_EQUALS(res,is_short(son),0);

        printf(KMAG "\tSon[%d] is short overdue and these are his remaining durations.\n" KWHT, son);
        printf(KMAG "\tSon[%d]'s durations should be contained in [ %d ... %d ].\n\t\t" KWHT,son, 0,(140-rq_time)*10);
        fflush(stdout);
        for (i = 0; i < 5; ++i) {
            int time = short_remaining_time(son);
            printf(KYEL"--T[%d]=%d, " KWHT, i,time);
            fflush(stdout);
            long_task();
        }
        printf("\n");
        fflush(stdout);
        status = res? 1 : 0;
        exit(status);
    }

    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_change_short_params(){
    bool res = true;
    sched_param_t param;
    int father = getpid();
    int status;
    int requested_time=2900;
    int sched_short_prio=0;
    param.requested_time=requested_time;
    param.sched_short_prio=sched_short_prio;

    EXPLAIN(Forking to create a short son with low prio.);

    int son = fork();
    if (son == 0){
        son = getpid();
        EXPLAIN(set the son.);
        TEST_EQUALS(res, sched_setscheduler(son, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
        TEST_EQUALS(res, sched_getparam(son,&param), 0);
        TEST_EQUALS(res, param.requested_time,requested_time);
        TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);
        int time = short_remaining_time(son);
        long_task();
        EXPLAIN(reset should fail);
        param.sched_short_prio = 100;
        param.requested_time = 3000;
        TEST_EQUALS(res, sched_setscheduler(son, SCHED_SHORT, &param), -1);
        TEST_EQUALS(res,errno,EPERM);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
        TEST_EQUALS(res, sched_getparam(son,&param), 0);
        TEST_EQUALS(res, param.requested_time,requested_time);
        TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);
        TEST_EQUALS(res,time-15 > short_remaining_time(son), true);

        param.sched_short_prio = 100;
        param.requested_time = 3000;
        TEST_EQUALS(res, sched_setparam(son, &param), -1);
        TEST_EQUALS(res,errno,EPERM);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
        TEST_EQUALS(res, sched_getparam(son,&param), 0);
        TEST_EQUALS(res, param.requested_time,requested_time);
        TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);
        TEST_EQUALS(res,time-15 > short_remaining_time(son), true);


        EXPLAIN(set with a negative policy is used to set param);
        param.sched_short_prio = 100;
        param.requested_time = 3000;
        TEST_EQUALS(res, sched_setscheduler(son, -1, &param), -1);
        TEST_EQUALS(res,errno,EPERM);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
        TEST_EQUALS(res, sched_getparam(son,&param), 0);
        TEST_EQUALS(res, param.requested_time,requested_time);
        TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);
        TEST_EQUALS(res,time-15 > short_remaining_time(son), true);

        param.sched_short_prio = 100;
        param.requested_time = 3000;
        TEST_EQUALS(res, sched_setscheduler(son, -1, &param), -1);
        TEST_EQUALS(res,errno,EPERM);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
        TEST_EQUALS(res, sched_getparam(0,&param), 0); // get param with 0 is on the calling process.
        TEST_EQUALS(res, param.requested_time,requested_time);
        TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);
        TEST_EQUALS(res,time-15 > short_remaining_time(son), true);
        status = res? 1 :0;
        exit(status);
    }

    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_fork(){
    bool res = true;
    sched_param_t param;
    int father = getpid();
    int status;
    int requested_time=45;
    int sched_short_prio=10;
    param.requested_time=requested_time;
    param.sched_short_prio=sched_short_prio;

    int son = fork();
    if (son==0) {
        son = getpid();
        EXPLAIN(Srtting the running task to be short);
        TEST_EQUALS(res, sched_setscheduler(son, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
        TEST_EQUALS(res, sched_getparam(son,&param), 0);
        TEST_EQUALS(res, param.requested_time,requested_time);
        TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);

        EXPLAIN(Son is forking- time should split and prio should stay constant);
        int grandson = fork();
        if (grandson == 0){
            EXPLAIN(Grandson is starting to run!);
            grandson = getpid();
            TEST_EQUALS(res,(short_remaining_time(grandson)<=1+requested_time/2),true);
            TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
            TEST_EQUALS(res, sched_getparam(son,&param), 0);
            TEST_EQUALS(res, param.requested_time,requested_time);
            TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);
            exit(res?1:0);
        }
        EXPLAIN(Son forked and should run before his father);
        EXPLAIN(Rather than the time slice- son params should not change);
        TEST_EQUALS(res,(short_remaining_time(son)<=requested_time/2),true);
        TEST_EQUALS(res, sched_getscheduler(son), SCHED_SHORT);
        TEST_EQUALS(res, sched_getparam(son,&param), 0);
        TEST_EQUALS(res, param.requested_time,requested_time);
        TEST_EQUALS(res, param.sched_short_prio,sched_short_prio);

        EXPLAIN(Son will run till he becomes an overdue to let his grandson exit);
        while (is_short(son)){
            long_task(); // takes around 20 ms on my cpu.
        }
        EXPLAIN(Son is overdue and running);
        TEST_EQUALS(res,is_short(son),0);
        EXPLAIN(Son will try to fork- should fail.);
        TEST_EQUALS(res,fork(),-1);
        wait(&status);
        TEST_DIFFERENT(res,status,0);
        exit(res?1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_nice(){
    bool res = true;


    return res;
}
#define AMOUNT_OF_SONS  4
bool test_order(){
    bool res = true;
    sched_param_t param;
    int father = getpid();
    int status;
    param.requested_time=3000;
    param.sched_short_prio=0;
    int test = fork();
    if (test == 0){
        test = getpid();
        int pids[AMOUNT_OF_SONS] = {0};
        int i;
        EXPLAIN(Setting up the test);
        TEST_EQUALS(res, sched_setscheduler(test, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(test), SCHED_SHORT);

        EXPLAIN(The test has a long duration and a high prio. should not stop rather than a RT task);
        EXPLAIN(test is checking his own place in queue);

        TEST_EQUALS(res,short_place_in_queue(test),0);

        for (i=0;i<AMOUNT_OF_SONS;++i){
            pids[i] = fork();
            if (pids[i] == 0){
                EXPLAIN(task is runing for the first time- should happen only after father allowed);
                exit(1);
            }
        }
        EXPLAIN(Looking up the location of each son- the location in the array + 1 for the test.);
        for (i=0;i<AMOUNT_OF_SONS;++i){
            TEST_EQUALS(res,short_place_in_queue(pids[i]),i+1);
        }
        EXPLAIN(Allow the sons to run by waiting - dq the run list);
        while(wait(&status)!=-1){
            EXPLAIN(Test caught a zombie son);
        }

        exit( res?1 : 0 );
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_other_prior_to_overdue(){
    bool res = true;
    sched_param_t param;
    int status;
    param.requested_time=5;
    param.sched_short_prio=0;
    int test = fork();
    if (test == 0){
        test = getpid();
        EXPLAIN(Forking before the setup- to keep a task with other policy);
        int other = fork();
        if (other == 0){
            EXPLAIN(I should run only after my father becomes a overdue - line below is GOOD!);
            exit(1);
        }
        EXPLAIN(Setting up the test);
        TEST_EQUALS(res, sched_setscheduler(test, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(test), SCHED_SHORT);
        while(is_short(test));
        EXPLAIN(Test is overdue);
        TEST_EQUALS(res,is_short(test), 0);
        wait(&status);
        exit(res?1:0);
    }

    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_short_place_in_rq_while_target_is_not_in_rq(){
    bool res = true;
    sched_param_t param;
    int status;
    param.requested_time=50;
    param.sched_short_prio=20;

    int test = fork();
    if (test == 0){
        test = getpid();
        int son = fork();
        if (son == 0){
            EXPLAIN(Son is running!);
            EXPLAIN(Son is setting his father-test to be a short);
            TEST_EQUALS(res, sched_setscheduler(test, SCHED_SHORT, &param), 0);
            TEST_EQUALS(res, sched_getscheduler(test), SCHED_SHORT);
            EXPLAIN(Father should not be in the RQ!);
            TEST_EQUALS(res,short_place_in_queue(test),-1);
            exit(res?1:0);
        }
        EXPLAIN(Test is entering the wait list- wont be in the run queue.);
        wait(&status);
        exit(res?1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_RR_and_FIFO_policy_still_works(){
    bool res = true;
    sched_param_t param;
    param.sched_priority = 120;
    int status, test = fork();
    if (test == 0){
        test = getpid();

        EXPLAIN(Setting the RR policy but param is invalid );
        TEST_EQUALS(res,sched_setscheduler(test,SCHED_RR,&param),-1);
        TEST_EQUALS(res,sched_getscheduler(test), SCHED_OTHER);
        TEST_EQUALS(res, errno, EINVAL);

        EXPLAIN(Setting the OTHER policy but param is invalid );
        TEST_EQUALS(res,sched_setscheduler(test,SCHED_RR,&param),-1);
        TEST_EQUALS(res,sched_getscheduler(test), SCHED_OTHER);
        TEST_EQUALS(res, errno, EINVAL);

        EXPLAIN(Setting the RR policy with valid param );
        param.sched_priority=20;
        TEST_EQUALS(res,sched_setscheduler(test,SCHED_RR,&param),0);
        TEST_EQUALS(res,sched_getscheduler(test), SCHED_RR);

        EXPLAIN(Setting the FIFO policy with valid param );
        TEST_EQUALS(res,sched_setscheduler(test,SCHED_FIFO,&param),0);
        TEST_EQUALS(res,sched_getscheduler(test), SCHED_FIFO);

        exit(res?1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_yield_the_cpu_to_other_and_lowe_prio(){
    EXPLAIN(Test will try to yield the cpu to a OTHER task - cpu stays at the test.);

    bool res = true;
    sched_param_t param;
    param.sched_priority = 100;
    param.sched_short_prio = 21;
    param.requested_time = 3000;

    int status, test = fork();
    if (test == 0) {
        test = getpid();
        int case1 = fork();
        if (case1 == 0){
            EXPLAIN(Case1 should only run once the test is overdue or the test set the case as short);
            exit(res?1:0);
        }

        EXPLAIN(Setting up the test - after the next lines case1 should not run till alowed! );
        TEST_EQUALS(res, sched_setscheduler(test, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(test), SCHED_SHORT);

        TEST_EQUALS(res, sched_yield(),0);
        EXPLAIN(The test should keep running without letting his son run.);

        EXPLAIN(Test will try to yield the cpu to a short task with lower prio - cpu stays at the test);
        param.sched_short_prio+=10;
        TEST_EQUALS(res, sched_setscheduler(case1, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(case1), SCHED_SHORT);

        TEST_EQUALS(res, sched_yield(),0);
        EXPLAIN(The test should keep running without letting his son run.);
        long_task();
        EXPLAIN(case1 didnt start runing therefore should have the initial amount of time);
        TEST_EQUALS(res, short_remaining_time(case1), param.requested_time);
        exit(res?1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}
bool test_yield_the_cpu_to_lower_prio_short(){

    EXPLAIN(Test will try to yield the cpu to a task with other policy and then to a short task with lower prio);

    bool res = true;
    sched_param_t param;
    param.sched_priority = 100;
    param.sched_short_prio = 21;
    param.requested_time = 3000;

    int status, test = fork();
    if (test == 0) {
        test = getpid();
        int case1 = fork();
        if (case1 == 0){
            EXPLAIN(Case1 should only run once the test is overdue or the test set the case as short);
            exit(res?1:0);
        }
        int case2 = fork();

        EXPLAIN(Setting up the test - after the next lines case1 should not run till alowed! );
        TEST_EQUALS(res, sched_setscheduler(test, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(test), SCHED_SHORT);

        TEST_EQUALS(res, sched_yield(),0);
        EXPLAIN(The test should keep running without letting his son run.);
        exit(res?1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

bool test_yield_the_cpu_between_short(){

    EXPLAIN(Test will try to yield the cpu to a task with other policy and then to a short task with lower prio);

    bool res = true;
    sched_param_t param;
    param.sched_priority = 100;
    param.sched_short_prio = 21;
    param.requested_time = 40;

    int status, test = fork();
    if (test == 0) {
        test = getpid();
        TEST_EQUALS(res, sched_setscheduler(test, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(test), SCHED_SHORT);

        int case1 = fork();
        if (case1 == 0){
            EXPLAIN(Case1 - The next task that should run is the test);
            TEST_EQUALS(res, sched_yield(),0);
            EXPLAIN(Case1 is running);
            exit(res?1:0);
        }

        EXPLAIN(Test - The next task that should run is case1);
        TEST_EQUALS(res, sched_yield(),0);
        EXPLAIN(Test - The next task that should run is case1);
        TEST_EQUALS(res, sched_yield(),0);
        EXPLAIN(Test is running);

        exit(res?1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}


bool test_other_before_overdue(){

    EXPLAIN(Test that a task with other policy runs before overdue);

    bool res = true;
    sched_param_t param;
    param.sched_priority = 100;
    param.sched_short_prio = 21;
    param.requested_time = 5;

    int status, test = fork();
    if (test == 0) {
        test = getpid();

        int case1 = fork();
        if (case1 == 0){
            EXPLAIN(Case1 should run only after the test is overdue);
            TEST_EQUALS(res,is_short(test),0);
            exit(res?1:0);
        }
        TEST_EQUALS(res, sched_setscheduler(test, SCHED_SHORT, &param), 0);
        TEST_EQUALS(res, sched_getscheduler(test), SCHED_SHORT);
        TEST_EQUALS(res, is_short(test), 1);
        long_task();
        TEST_EQUALS(res,is_short(test),0);

        EXPLAIN(Test should run only after other finishes- therefore wont exit the runqueue);
        wait(&status);

        exit(res?1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

#define AMOUNT_OF_OVERDUE   10
bool test_order_between_overdues(){

    EXPLAIN(Test the order between overdues- by creating overdues with predetrmined ordedr);

    bool res = true;
    sched_param_t param;
    param.requested_time = 5;

    int pids[AMOUNT_OF_OVERDUE] = {0};
    int status, test = fork();

    if (test == 0){
        EXPLAIN(Father is other and will creatre shorts with a low duration);
        EXPLAIN(The shorts will recieve the cpu but become overdue very fast- and return the cpu to the father);
        int i;
        for (i=0; i < AMOUNT_OF_OVERDUE; ++i ){
            param.sched_short_prio = i*i; // overdues don'tdetermine their  place in queue by prio
            int son = fork();
            if (son == 0) {
                TEST_EQUALS(res,is_short(getpid()),1);
                long_task();
                TEST_EQUALS(res,is_short(getpid()),0);
                exit(res?1:0);
            }
            EXPLAIN(Created a son- setting him to be short- son will steal the cpu.);
            pids[i] = son;
            TEST_EQUALS(res, sched_setscheduler(son, SCHED_SHORT, &param), 0);

        }
        EXPLAIN(Verify all the sons are overdue.);
        for (i=0;i<AMOUNT_OF_OVERDUE;++i){
            TEST_EQUALS(res, is_short(pids[i]), 0 );
            TEST_EQUALS(res,short_remaining_time(pids[i]) > 10*(140-i*i) - 3, true); // give a few ms for slack and rounding
        }
        for (i=0;i<AMOUNT_OF_OVERDUE;++i){
            TEST_EQUALS(res, short_place_in_queue(pids[i]), i );
        }
        EXPLAIN(Clean up.);
        while(wait(&status) != -1 ){
            TEST_DIFFERENT(res,status,0);
        }
        exit(res? 1:0);
    }
    wait(&status);
    TEST_DIFFERENT(res,status,0);
    return res;
}

int main() {

    RUN_TEST(test_sched_set_schedule);
    RUN_TEST(test_is_short);
    RUN_TEST(test_short_remaining_time);
    RUN_TEST(test_change_short_params);
    RUN_TEST(test_fork);
    RUN_TEST(test_order);
    RUN_TEST(test_other_prior_to_overdue);
    RUN_TEST(test_short_place_in_rq_while_target_is_not_in_rq);
    RUN_TEST(test_RR_and_FIFO_policy_still_works);
    RUN_TEST(test_yield_the_cpu_to_other_and_lowe_prio);
    RUN_TEST(test_yield_the_cpu_between_short);
    RUN_TEST(test_other_before_overdue);
    RUN_TEST(test_order_between_overdues);

    return 0;
}

