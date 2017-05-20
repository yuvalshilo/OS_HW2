#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#include "hw2_syscalls.h"

/* DELETE THESE LINES ONLY IF SYSCALL HEADER DOES NOT DEFINE THESE */ 
struct sched_param {
	int sched_priority; //ignored for SHORT processes
	int requested_time; //between 1 and 3000
	int sched_short_prio; //between 0 and 139
};
#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_SHORT     5
/* */
 
#define ASSERT_TEST(b) do { \
        if (!(b)) { \
                fprintf(stdout, "\nAssertion failed at %s:%d %s ",__FILE__,__LINE__,#b); \
                return false; \
        } \
} while (0)

#define RUN_TEST(test) do { \
        fprintf(stdout, "Running "#test"... "); \
        if (test()) { \
            fprintf(stdout, "[OK]\n");\
        } else { \
        	fprintf(stdout, "[Failed]\n"); \
        } \
} while(0)

/* LET US BEGIN */

void make_realtime(int policy){
    //Set the process to become realtime
    struct sched_param p;
    p.sched_priority = 1;
    assert(sched_setscheduler(getpid(), policy, &p) >= 0);
}

static pid_t nonShortCompletionOrderArray[2];
static pid_t *nonShortCompletionOrder = nonShortCompletionOrderArray;
static int* nonShortIndex;

void do_nonShortTasks_job(){
    int i = 0, j = 0;
    for( ; i < 10000 ; i++){
        for(j = 0 ; j < 1000 ; j++) { /* Nothingness is empty */ }
    }

    nonShortCompletionOrder[(*nonShortIndex)++] = getpid();
}

bool test_nonShortTasks(){
    pid_t testProcess = fork();
    if(testProcess == 0){

        //Setup shared variables
        nonShortIndex = mmap(NULL, sizeof(*nonShortIndex), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *nonShortIndex = 0;
        nonShortCompletionOrder = mmap(NULL, sizeof(nonShortCompletionOrderArray), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        //We will fork a realtime task and a regular one, and wait until their completion (that means original scheduling algo did not change)
        pid_t other = fork();
        if(other == 0){
            sleep(1); // the OTHER process started running before the realtime one, we sleep for one second to make sure the realtime process got his priority in the run queue
            do_nonShortTasks_job();
            exit(0);
        }

        pid_t realtime = fork();
        if(realtime == 0){
           make_realtime(SCHED_RR);
            do_nonShortTasks_job();
            exit(0);
        }
        while(wait(NULL) != -1); // Cleanup

        ASSERT_TEST(nonShortCompletionOrder[0] == realtime);
        ASSERT_TEST(nonShortCompletionOrder[1] == other);

        // Unmap shared variables  
        munmap(nonShortCompletionOrder, sizeof(nonShortCompletionOrder));
        munmap(nonShortIndex, sizeof(nonShortIndex));

        exit(0);
    }
    while(wait(NULL) != -1); // Cleanup

    return true;
}

static int* ready;

bool test_syscalls(){
    pid_t testProcess = fork();
    if(testProcess == 0){

        make_realtime(SCHED_RR); // The father needs to have higher running priority over his son 

        //setup shared variables
        ready = mmap(NULL, sizeof(*ready), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *ready = 0;

        //setparam, getparam, setscheduler, getscheduler, setnice
        pid_t gonnaBeShort = fork();
        if(gonnaBeShort == 0){
            struct sched_param p;
            
            //Invalid input checks:
            p.sched_priority = 0; // Make sure not checked!!
            p.requested_time = 0; // Should fail
            p.sched_short_prio = 0; // Should be ok

            ASSERT_TEST(sched_setscheduler(getpid() , SCHED_SHORT, &p) == -1 && errno == EINVAL);
            
            p.sched_priority = 0; // Make sure not checked!!
            p.requested_time = 1; // Should be ok
            p.sched_short_prio = 140; // Should fail

            ASSERT_TEST(sched_setscheduler(getpid() , SCHED_SHORT, &p) == -1 && errno == EINVAL);

            p.sched_priority = 1; // Still, make sure not checked!!
            p.requested_time = 3001; // Should fail
            p.sched_short_prio = 139; // Should be ok

            ASSERT_TEST(sched_setscheduler(getpid() , SCHED_SHORT, &p) == -1 && errno == EINVAL);

            *ready = 1;
            while(is_short(getpid()) != 1 && is_short(getpid()) != 0){ sched_yield(); } // Yield for father to set us as SHORT
            
            // Now we are SHORT
            
            //Nice call should FAIL
            // ASSERT_TEST(nice(1) < 0);

            // All should fail because the process is already a SHORT one
            p.sched_priority = 0; // Will not change anymore
            p.requested_time = 30; 
            p.sched_short_prio = 1;
        
            ASSERT_TEST(sched_setscheduler(getpid() , SCHED_OTHER, &p) == -1 && errno == EPERM);
            ASSERT_TEST(sched_setparam(getpid(), &p) == -1 && errno == EPERM);

            // The task should still be SHORT, no possible way that all of the above took 3000 msecs ^
            ASSERT_TEST(sched_getscheduler(getpid()) == SCHED_SHORT);
            while(is_short(getpid()) != 0);// Wait until we become overdue

            
            ASSERT_TEST(sched_getscheduler(getpid()) == SCHED_SHORT); // Overdue SHORT is still considered SHORT (for those who implemented using another policy name)

            struct sched_param params;
            ASSERT_TEST(sched_getparam(getpid(), &params) >= 0);
            ASSERT_TEST(params.requested_time == 3000 && params.sched_short_prio == 0); // Params should be as we set them initialy

            //And we're done here
            exit(0);
        }

        while(!*ready); // Wait until son is ready to become SHORT
        struct sched_param p1;
        p1.sched_priority = 1337; // ELITE STUFF
        p1.requested_time = 3000; // Just because
        p1.sched_short_prio = 0; // Maximum prio engaged

        ASSERT_TEST(sched_setscheduler(gonnaBeShort, SCHED_SHORT, &p1) >= 0);

        wait(NULL); // Wait until son exits
        
        *ready = 0; // "lock" cleanup
        
        gonnaBeShort = fork(); // Another test
        if(gonnaBeShort == 0){

            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &p1) >= 0); // A process may change his policy to become SHORT

            exit(0);
        }
        wait(NULL);

        p1.sched_priority = 0;
        ASSERT_TEST(sched_setscheduler(getpid(), SCHED_OTHER, &p1) >= 0); // We should be able to convert ourselves back to being OTHER

        //is_short, short_remaining_time, short_place_in_queue
        
        //calling is_short on a regular task:
        ASSERT_TEST(is_short(getpid()) < 0 && errno == EINVAL);

        //calling is_short on a non-existent task
        ASSERT_TEST(is_short(-100) < 0 && errno == ESRCH);

        p1.requested_time = 1000;
        gonnaBeShort = fork();
        if(gonnaBeShort == 0){
            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &p1) >= 0);

            ASSERT_TEST(is_short(getpid()) == 1);
            while(is_short(getpid()) == 1); // lets wait until the process becomes overdue
            ASSERT_TEST(is_short(getpid()) == 0); // Hey overdue, whatsup?

            exit(0);
        }
        wait(NULL);

        //short_remaining_time
        //calling short_remaining_time on a regular task:
        ASSERT_TEST(short_remaining_time(getpid()) < 0 && errno == EINVAL);

        //calling short_remaining_time on a non-existent task
        ASSERT_TEST(short_remaining_time(-100) < 0 && errno == ESRCH);

        gonnaBeShort = fork();
        if(gonnaBeShort == 0){
            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &p1) >= 0); 

            ASSERT_TEST(short_remaining_time(getpid()) <= 1000); // No way a SHORT process' remaining_time will be higher than what was allocated for him

            int i = 1;
            for( ; i < 10000000 ; i++); // Wait a bit to see remaining_time decrease
            ASSERT_TEST(short_remaining_time(getpid()) < 1000);

            while(is_short(getpid()) == 1); // lets wait until the process becomes overdue

            ASSERT_TEST(short_remaining_time(getpid()) <= 1400); // Overdue timeslice calculation + above explanation
            i = 1;
            for( ; i < 10000000 ; i++); // Wait a bit to see remaining_time decrease
            ASSERT_TEST(short_remaining_time(getpid()) < 1400);


            exit(0);
        }
        wait(NULL);

        //short_place_in_queue
        //calling short_place_in_queue on a regular task:
        ASSERT_TEST(short_place_in_queue(getpid()) < 0 && errno == EINVAL);

        //calling short_place_in_queue on a non-existent task
        ASSERT_TEST(short_place_in_queue(-100) < 0 && errno == ESRCH);

        /* Remember the father process is realtime, he has the highest priority among his children and himself */
        make_realtime(SCHED_RR);

        struct sched_param placeParams;
        placeParams.sched_short_prio = 2;
        placeParams.requested_time = 1000; // We want son1 to become overdue before son2

        pid_t son1 = fork();
        if(son1 == 0){
            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &placeParams) == 0);
            sched_yield();

            while(is_short(getpid()) == 1); // wait until we become overdue
            ASSERT_TEST(short_place_in_queue(getpid()) == 0);
            sched_yield(); // let son2 become overdue
            ASSERT_TEST(short_place_in_queue(getpid()) == 0);
            sched_yield();
            exit(0);
        }
        pid_t son2 = fork();
        placeParams.sched_short_prio = 1; // Higher priority than son1
        placeParams.requested_time = 3000; // son2 should become overdue before son1
        if(son2 == 0){
            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &placeParams) == 0);
            ASSERT_TEST(short_place_in_queue(getpid()) == 0); // son2 has the higher priority, so he should be first in queue
            ASSERT_TEST(short_place_in_queue(son1) == 1);

            sched_yield();
            while(is_short(getpid()) == 1); // wait until we become overdue
            ASSERT_TEST(short_place_in_queue(getpid()) == 0);
            ASSERT_TEST(short_place_in_queue(son1) == 1);
            sched_yield();

            exit(0);
        }

        wait(NULL); wait(NULL); // Wait until son1 and son2 have finished

        *ready = 0; // Cleanup

        munmap(ready, sizeof(*ready));
        exit(0);
    }
    while(wait(NULL) != -1); // Cleanup

    return true;
}

pid_t forkOrderArray[2];
pid_t* forkOrder = forkOrderArray;
int* forkOrder_index;

bool test_fork(){
    pid_t testProcess = fork();
    if(testProcess == 0){

        struct sched_param p;
        p.sched_priority = 1337; // ELITE STUFF
        p.requested_time = 1000; // Just because
        p.sched_short_prio = 2; 

        if(fork() == 0){
            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &p) >= 0);
            if(fork() == 0){
                ASSERT_TEST(short_remaining_time(getpid()) <= 500);
                struct sched_param child_p;
                sched_getparam(getpid(), &child_p);
                ASSERT_TEST(child_p.sched_short_prio == 2);
                exit(0);
            }
            ASSERT_TEST(short_remaining_time(getpid()) <= 500);
            wait(NULL);

            //Wait until overdue
            while(is_short(getpid()) != 0); 

            ASSERT_TEST(fork() == -1 && errno == EACCES); // Overdue should not be able to fork

            exit(0);
        }
        wait(NULL);

        //setup shared variables
        forkOrder = mmap(NULL, sizeof(forkOrderArray), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        forkOrder_index = mmap(NULL, sizeof(*forkOrder_index), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *forkOrder_index = 0;

        if(fork() == 0){ // Fork order
            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &p) >= 0); // Make ourselves SHORT
            pid_t son = fork();
            if(son == 0){ // A forked child of a SHORT process will be scheduled after his father
                forkOrder[(*forkOrder_index)++] = getpid();
                exit(0);
            }

            forkOrder[(*forkOrder_index)++] = getpid();
            wait(NULL);

            ASSERT_TEST(forkOrder[0] == getpid() && forkOrder[1] == son);

            exit(0);
        }

        munmap(forkOrder, sizeof(forkOrderArray));
        munmap(forkOrder_index, sizeof(*forkOrder_index));

        exit(0);
    }
    while(wait(NULL) != -1); // Cleanup
    return true;
}

static pid_t scheduleOrderArray[4];
static pid_t *schedulingOrder = scheduleOrderArray;
static int* scheduling_index;

void do_scheduleOrder_job(){
    int i = 0 , j = 0;
    for( ; i < 100000 ; i++){ 
        for(j = 0 ; j < 100 ; j++);
     }

    schedulingOrder[(*scheduling_index)++] = getpid();
}

bool test_policiesSchedulingOrder(){
    pid_t testProcess = fork();
    if(testProcess == 0){
        //Setup shared variables
        scheduling_index = mmap(NULL, sizeof(*scheduling_index), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *scheduling_index = 0;
        schedulingOrder = mmap(NULL, sizeof(scheduleOrderArray), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        pid_t anOther = fork();
        if(anOther == 0){
            sched_yield(); // Let father continue
            //anOther is already OTHER, no need to change him
            do_scheduleOrder_job();
            exit(0);
        }

        pid_t aShort = fork();
        if(aShort == 0){
            sched_yield(); // Let father continue
            do_scheduleOrder_job();
            while(is_short(getpid()) != 0); // Wait until we become overdue
            do_scheduleOrder_job();
            exit(0);
        }

        pid_t aRealtime = fork();
        if(aRealtime == 0){
            sched_yield(); // Let father continue
            do_scheduleOrder_job();
            exit(0);
        }

        //we need the father to be realtime so aRealtime does not take control straightaway (or any other task)
        make_realtime(SCHED_RR);

        //Now that all sons are alive we can set their parameters
        struct sched_param p1; p1.sched_priority = 80; // Lower priority than the father (so aRealtime will not take control)
        ASSERT_TEST(sched_setscheduler(aRealtime, SCHED_RR, &p1) == 0);

        struct sched_param p2; p2.requested_time = 3000; p2.sched_short_prio = 10; // Some random variables
        ASSERT_TEST(sched_setscheduler(aShort, SCHED_SHORT, &p2) == 0);

        struct sched_param p3; p3.sched_priority = 0;
        ASSERT_TEST(sched_setscheduler(getpid(), SCHED_OTHER, &p3) == 0);
        
        wait(NULL); wait(NULL); wait(NULL); wait(NULL); wait(NULL); wait(NULL); wait(NULL); wait(NULL); // reminds me of OK OK OK OK OK OK 

        ASSERT_TEST(schedulingOrder[0] == aRealtime && schedulingOrder[1] == aShort && schedulingOrder[2] == anOther && schedulingOrder[3] == aShort);

        // unmap shared memory
        munmap(scheduling_index, sizeof(*scheduling_index));
        munmap(schedulingOrder, sizeof(scheduleOrderArray));

        exit(0);
    }
    while(wait(NULL) != -1); // Cleanup

    return true;
}

static pid_t priorityOrderArray[2];
static pid_t *priorityOrder = priorityOrderArray;
static int* priority_index;

bool test_shortPriorityCheck(){
    pid_t testProcess = fork();
    if(testProcess == 0){
        //Setup shared variables
        priority_index = mmap(NULL, sizeof(*priority_index), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *priority_index = 0;
        priorityOrder = mmap(NULL, sizeof(priorityOrderArray), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        pid_t firstShort = fork();
        if(firstShort == 0){
            sched_yield(); // Let the father continue
            while(is_short(getpid()) != 1);

            priorityOrder[(*priority_index)++] = getpid();

            exit(0);
        }

        pid_t secondShort = fork();
        if(secondShort == 0){
            struct sched_param param;
            param.requested_time = 1000;
            param.sched_short_prio = 30; // We have lower priority than firstShort

            ASSERT_TEST(sched_setscheduler(getpid(), SCHED_SHORT, &param) == 0);

            param.sched_short_prio = 1; // We have lower priority than firstShort
            ASSERT_TEST(sched_setscheduler(firstShort, SCHED_SHORT, &param) == 0);

            priorityOrder[(*priority_index)++] = getpid();

            exit(0);
        }

        wait(NULL); wait(NULL);

        ASSERT_TEST(priorityOrder[0] == firstShort && priorityOrder[1] == secondShort);

        // unmap shared memory
        munmap(priority_index, sizeof(*priority_index));
        munmap(priorityOrder, sizeof(priorityOrderArray));

        exit(0);
    }
    while(wait(NULL) != -1); // Cleanup

    return true;
}

#define RR_SIZE 30

static pid_t roundRobinArray[RR_SIZE];
static pid_t *roundRobin = roundRobinArray;
static int* roundRobin_index;

bool test_overdueRR(){
    
    //Setup shared variables
    roundRobin_index = mmap(NULL, sizeof(*roundRobin_index), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *roundRobin_index = 0;
    roundRobin = mmap(NULL, sizeof(roundRobinArray), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    pid_t testProcess = fork();
    if(testProcess == 0){
        pid_t one, two, three; // One Two Three, DRINK!
        struct sched_param p; p.sched_short_prio = 80; p.requested_time = 100;
        
        
        one = fork();
        if(one == 0){
            sched_setscheduler(getpid(), SCHED_SHORT, &p);
            while(is_short(getpid()) == 1); // Waiting until overdue , wake me up when timeslice endddddddds

            while(*roundRobin_index < RR_SIZE){
                roundRobin[(*roundRobin_index)++] = getpid();
                sched_yield(); // Should move us to the end of the queue
            }

            exit(0);
        }

        p.sched_short_prio = 139;
        two = fork();
        if(two == 0){
            sched_setscheduler(getpid(), SCHED_SHORT, &p);
            while(is_short(getpid()) == 1); // Waiting until overdue , wake me up when timeslice endddddddds

            while(*roundRobin_index < RR_SIZE){
                roundRobin[(*roundRobin_index)++] = getpid();
                sched_yield(); // Should move us to the end of the queue
            }

            exit(0);
        }

        p.sched_short_prio = 42;
        three = fork();
        if(three == 0){
            sched_setscheduler(getpid(), SCHED_SHORT, &p);
            while(is_short(getpid()) == 1); // Waiting until overdue , wake me up when timeslice endddddddds

            while(*roundRobin_index < RR_SIZE){
                roundRobin[(*roundRobin_index)++] = getpid();
                sched_yield(); // Should move us to the end of the queue
            }

            exit(0);
        }
        wait(NULL); wait(NULL); wait(NULL);

        int read_index = 3;
        for( ; read_index < RR_SIZE ; read_index++)
            ASSERT_TEST(roundRobin[read_index] == roundRobin[read_index - 3]); // 3 overdue tasks, are being scheduled in the same order every time (round robin)   

        exit(0);
    }
    while(wait(NULL) != -1); // Cleanup

    munmap(roundRobin, sizeof(roundRobinArray));
    munmap(roundRobin_index, sizeof(*roundRobin_index));

    return true;
}

int main(){
    setbuf(stdout, NULL);
    RUN_TEST(test_nonShortTasks);
    RUN_TEST(test_syscalls);
    RUN_TEST(test_fork);
    RUN_TEST(test_policiesSchedulingOrder);
    RUN_TEST(test_shortPriorityCheck);
    RUN_TEST(test_overdueRR);
    return 0;
}
