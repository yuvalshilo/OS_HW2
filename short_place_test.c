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

bool short_place_check() {
	struct sched_param p1;
	p1.sched_priority = 30;
	p1.requested_time = 3000;
	p1.sched_short_prio = 10;

	struct sched_param p2;
	p2.sched_priority = 30;
	p2.requested_time = 3000;
	p2.sched_short_prio = 20;

	pid_t testProcess = fork();
	if (testProcess == 0) {
		make_realtime(SCHED_RR);
		pid_t pid1 = fork();
		if (pid1 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid2 = fork();
		if (pid2 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid3 = fork();
		if (pid3 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid4 = fork();
		if (pid4 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid5 = fork();
		if (pid5 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		//father set sons as short
		ASSERT_TEST(sched_setscheduler(pid1, SCHED_SHORT, &p2) == 0);
		ASSERT_TEST(sched_setscheduler(pid2, SCHED_SHORT, &p2) == 0);
		ASSERT_TEST(sched_setscheduler(pid3, SCHED_SHORT, &p1) == 0);
		ASSERT_TEST(sched_setscheduler(pid4, SCHED_SHORT, &p1) == 0);
		ASSERT_TEST(sched_setscheduler(pid5, SCHED_SHORT, &p1) == 0);

		//check order:
		ASSERT_TEST(short_place_in_queue(pid3) == 0);
		ASSERT_TEST(short_place_in_queue(pid4) == 1);
		ASSERT_TEST(short_place_in_queue(pid5) == 2);
		ASSERT_TEST(short_place_in_queue(pid1) == 3);
		ASSERT_TEST(short_place_in_queue(pid2) == 4);


		while (wait(NULL) != -1); // Cleanup
		exit(0);
	}
	while (wait(NULL) != -1); // Cleanup


							  /*NEW TEST!!!!!!!*/

	struct sched_param p11;
	p11.sched_priority = 30;
	p11.requested_time = 3000;
	p11.sched_short_prio = 11;

	struct sched_param p12;
	p12.sched_priority = 30;
	p12.requested_time = 3000;
	p12.sched_short_prio = 12;

	struct sched_param p13;
	p13.sched_priority = 30;
	p13.requested_time = 3000;
	p13.sched_short_prio = 13;

	struct sched_param p14;
	p14.sched_priority = 30;
	p14.requested_time = 3000;
	p14.sched_short_prio = 14;

	struct sched_param p15;
	p15.sched_priority = 30;
	p15.requested_time = 3000;
	p15.sched_short_prio = 15;

	testProcess = fork();
	if (testProcess == 0) {
		make_realtime(SCHED_RR);
		pid_t pid1 = fork();
		if (pid1 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid2 = fork();
		if (pid2 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid3 = fork();
		if (pid3 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid4 = fork();
		if (pid4 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		pid_t pid5 = fork();
		if (pid5 == 0) {
			while (is_short(getpid()) != 1);
			while (is_short(getpid()) != 0);
			exit(0);
		}
		//father set sons as short
		ASSERT_TEST(sched_setscheduler(pid1, SCHED_SHORT, &p15) == 0);
		ASSERT_TEST(sched_setscheduler(pid2, SCHED_SHORT, &p14) == 0);
		ASSERT_TEST(sched_setscheduler(pid3, SCHED_SHORT, &p13) == 0);
		ASSERT_TEST(sched_setscheduler(pid4, SCHED_SHORT, &p12) == 0);
		ASSERT_TEST(sched_setscheduler(pid5, SCHED_SHORT, &p11) == 0);

		//check order:
		ASSERT_TEST(short_place_in_queue(pid1) == 4);
		ASSERT_TEST(short_place_in_queue(pid2) == 3);
		ASSERT_TEST(short_place_in_queue(pid3) == 2);
		ASSERT_TEST(short_place_in_queue(pid4) == 1);
		ASSERT_TEST(short_place_in_queue(pid5) == 0);

		struct sched_param p;
		p.sched_priority = 0;
		//make myself OTHER so sons will run and become overdue;
		ASSERT_TEST(sched_setscheduler(getpid(), SCHED_OTHER, &p) >= 0);
		//now sons are overdue;
		ASSERT_TEST(short_place_in_queue(pid1) == 4);
		ASSERT_TEST(short_place_in_queue(pid2) == 3);
		ASSERT_TEST(short_place_in_queue(pid3) == 2);
		ASSERT_TEST(short_place_in_queue(pid4) == 1);
		ASSERT_TEST(short_place_in_queue(pid5) == 0);

		while (wait(NULL) != -1); // Cleanup
		exit(0);
	}


	while (wait(NULL) != -1); // Cleanup

	return true;
}



int main(){
