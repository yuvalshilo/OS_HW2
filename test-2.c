#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

#include "hw2_syscalls.h"


#define SCHED_OTHER		0
#define SCHED_FIFO		1
#define SCHED_RR		2
#define SCHED_SHORT     5
/* */
 
 
#define ASSERT_TEST(b) do { \
        if (!(b)) { \
                fprintf(stdout, "\nAssertion failed at %s:%d %s ",__FILE__,__LINE__,#b); \
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


static int i;

static int* ready1;
static int* ready2;
static int* ready3;

static int* histo;
static int* index;
#define MARK(num) do { \
    histo[*index]=num; \
    *index = *index+1; \
    } while(0);

void print_histo(int* arr, int size){
  int j=0;
  for(j=0;j<size;j++){
    printf("%d, \n",arr[j]);
  }

}


bool increasing(int* arr, int size ){
  int j=0;
  for(j=0;j<size-1;j++){
    if(arr[j]>arr[j+1]){
      printf("fail, order of events should be non-decreasing\n");
      print_histo(arr,size);
      return false;
    }
  }
  return true;
}


#define BIG_NUM 10000

#define mil_sleep(num) do{ \
  usleep(num*1000); \
  }\
  while(0);

bool test_the_test() {
    int tester=fork();
    if(tester==0){

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       int son1= fork();
       if(son1==0){
            while(!*ready1){};
            MARK(1);
            exit(0);
       }
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
            MARK(1);
            exit(0);
       }
       int son3= fork();
       if(son3==0){
            while(!*ready1){};
            MARK(1);
            exit(0);
       }
       sched_yield();
       MARK(0);
       sched_yield();
       *ready1=1;
       while(wait(NULL) != -1);

       MARK(2);
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       exit(0);




    }
    while(wait(NULL) != -1);
    return true; 
}






//other vs other
bool winter17_test0() {
    int tester=fork();
    if(tester==0){

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       int son1= fork();
       if(son1==0){
            while(!*ready1){};
            sched_yield();
            MARK(3);
            exit(0);
       }
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
            sched_yield();
            MARK(4);
            nice(-20);

            mil_sleep(50); //let son3 set his nice

            while(!*ready2){};
            sched_yield();
            MARK(7);

            exit(0);
       }
       int son3= fork();
       if(son3==0){
            while(!*ready1){};
            // sched_yield();
            MARK(4);
            nice(10);

            mil_sleep(50);//let son2 set his nice

            while(!*ready2){};
            MARK(8);

            exit(0);
       }
       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);


       p.sched_short_prio = 10;
       p.requested_time = 3000; 
       ASSERT_TEST( sched_setscheduler(son1, SCHED_SHORT, &p) >= 0);


       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);
       sleep(1);

       sched_yield();
       MARK(5);

       *ready2=1;

       sched_yield();
       MARK(6);


       while(wait(NULL) != -1);

       MARK(9);
     
       ASSERT_TEST( increasing(histo,arr_size));
       
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       munmap(ready2, sizeof(ready2));
       exit(0);
      

  }
  while(wait(NULL) != -1);
  return true; 
}

//short vs other
bool winter17_test3() {
    int tester=fork();
    if(tester==0){

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       int son1= fork();
       if(son1==0){
            while(!*ready1){};
            sched_yield();
            MARK(3);
            exit(0);
       }
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
            MARK(4);
         
            exit(0);
       }
      
       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);


       p.sched_short_prio = 10;
       p.requested_time = 3000; 
       ASSERT_TEST( sched_setscheduler(son1, SCHED_SHORT, &p) >= 0);


       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);
      

       while(wait(NULL) != -1);

       MARK(5);
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       munmap(ready2, sizeof(ready2));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//overdue vs other
bool winter17_test4() {
    int tester=fork();
    if(tester==0){
      
        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       int son1= fork();
       if(son1==0){
            while(!*ready1){};

            sched_yield();
            MARK(3);

            while(is_short(getpid()) != 0){};

            while(!*ready2){};

            MARK(7);

            exit(0);
       }
       int son2= fork();
       if(son2==0){
            while(!*ready2){

            };

            sched_yield();
            MARK(6);
         
            exit(0);
       }
      
       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);


       p.sched_short_prio = 10;
       p.requested_time = 10; 
       ASSERT_TEST( sched_setscheduler(son1, SCHED_SHORT, &p) >= 0);


       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);

       sleep(1);

       sched_yield();
       MARK(4);

       *ready2=1;

       sched_yield();
       MARK(5);

       sleep(1);
     
       sched_yield();
       MARK(8);

       while(wait(NULL) != -1);

       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       munmap(ready2, sizeof(ready2));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//short vs short
bool winter17_test5() {
    int tester=fork();
    if(tester==0){
      

       int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       int son1= fork();
       if(son1==0){
            while(!*ready1){};
            sched_yield();
            MARK(3);
            exit(0);
       }
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
            sched_yield();
            MARK(4);
            exit(0);
       }
       int son3= fork();
       if(son3==0){
            while(!*ready1){};
            sched_yield();
            MARK(5);
            exit(0);
       }
       sched_yield();
       MARK(0);
       sched_yield();

       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);

       p.sched_priority = 0; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_OTHER, &p) >= 0);

       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);


       p.sched_priority = 50;
       p.requested_time = 3000; 

       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son1, SCHED_SHORT, &p) >= 0);

       p.sched_short_prio = 20;
       ASSERT_TEST( sched_setscheduler(son2, SCHED_SHORT, &p) >= 0);

       p.sched_short_prio = 30;
       ASSERT_TEST( sched_setscheduler(son3, SCHED_SHORT, &p) >= 0);

       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);

       while(wait(NULL) != -1);

       MARK(6);
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//short run in fifo
bool winter17_test6() {
    int tester=fork();
    if(tester==0){
      

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       int son1= fork();
       if(son1==0){
            while(!*ready1){};
            sched_yield();
            MARK(8);
            exit(0);
       }
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
       
            MARK(7);
            exit(0);
       }
       int son3= fork();
       if(son3==0){
            while(!*ready1){};
      
            MARK(6);
            exit(0);
       }

       int son4= fork();
       if(son4==0){
            while(!*ready1){};
            sched_yield();
            MARK(5);
            exit(0);
       }

       int son5= fork();
       if(son5==0){
            while(!*ready1){};
   
            MARK(4);
            exit(0);
       }

       int son6= fork();
       if(son6==0){
            while(!*ready1){};

            MARK(3);
            exit(0);
       }

       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);



       p.sched_priority = 50;
       p.requested_time = 3000; 

       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son3, SCHED_SHORT, &p) >= 0);

       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son2, SCHED_SHORT, &p) >= 0);

       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son1, SCHED_SHORT, &p) >= 0);

       p.sched_short_prio = 5;
       ASSERT_TEST( sched_setscheduler(son6, SCHED_SHORT, &p) >= 0);

       p.sched_short_prio = 5;
       ASSERT_TEST( sched_setscheduler(son5, SCHED_SHORT, &p) >= 0);

       p.sched_short_prio = 5;
       ASSERT_TEST( sched_setscheduler(son4, SCHED_SHORT, &p) >= 0);

       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);

       while(wait(NULL) != -1);

       MARK(9);
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//short vs other vs overdue
bool winter17_test8() {
    int tester=fork();
    if(tester==0){
      

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

        //other
       int son1= fork();
       if(son1==0){
            while(!*ready1){};
            sched_yield();
            MARK(7);
            exit(0);
       }

       //other
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
       
            sched_yield();
            MARK(7);
            exit(0);
       }

       //short
       int son3= fork();
       if(son3==0){
            while(!*ready1){};
      
            sched_yield();
            MARK(6);
            exit(0);
       }

       //short
       int son4= fork();
       if(son4==0){
            while(!*ready1){};

            sched_yield();
            MARK(5);
            exit(0);
       }

       //overdue
       int son5= fork();
       if(son5==0){
            while(!*ready1){};
            
            sched_yield();
            MARK(4);
            while(is_short(getpid()) != 0){};
            MARK(9);
            sched_yield();
            MARK(11);
            sched_yield();
            MARK(12);
            exit(0);
       }

        //overdue
       int son6= fork();
       if(son6==0){
            while(!*ready1){};

            sched_yield();
            MARK(3);
            while(is_short(getpid()) != 0){};
            MARK(8);
            sched_yield();
            MARK(10);

            exit(0);
       }

       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);



      


       p.sched_priority = 50;
       p.requested_time = 3000; 
       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son3, SCHED_SHORT, &p) >= 0);


       p.sched_priority = 50;
       p.requested_time = 3000; 
       p.sched_short_prio = 5;
       ASSERT_TEST( sched_setscheduler(son4, SCHED_SHORT, &p) >= 0);

       p.sched_priority = 50;
       p.requested_time = 20; 
       p.sched_short_prio = 4;
       ASSERT_TEST( sched_setscheduler(son5, SCHED_SHORT, &p) >= 0);

       p.sched_priority = 50;
       p.requested_time = 20; 
       p.sched_short_prio = 3;
       ASSERT_TEST( sched_setscheduler(son6, SCHED_SHORT, &p) >= 0);

       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);

       while(wait(NULL) != -1);

       MARK(13);
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//short yield to lower prio
bool winter17_test11_a() {
    int tester=fork();
    if(tester==0){
      

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       
       int son1= fork();
       if(son1==0){
            while(!*ready1){};
          
            MARK(5);
            exit(0);
       }

      
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
       
            sched_yield();
            MARK(3);
            sched_yield();
            MARK(4);

            exit(0);
       }

       

       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);


       p.sched_priority = 50;
       p.requested_time = 3000; 
       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son1, SCHED_SHORT, &p) >= 0);


       p.sched_priority = 50;
       p.requested_time = 3000; 
       p.sched_short_prio = 5;
       ASSERT_TEST( sched_setscheduler(son2, SCHED_SHORT, &p) >= 0);

      

       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);

       while(wait(NULL) != -1);

       MARK(6);
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//short yield to same prio
bool winter17_test11_b() {
    int tester=fork();
    if(tester==0){
      

       int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;

       
       int son1= fork();
       if(son1==0){
            while(!*ready1){};
          
            MARK(3);
            sched_yield();
            MARK(5);
            sched_yield();
            MARK(7);
            exit(0);
       }

      
       int son2= fork();
       if(son2==0){
            while(!*ready1){};
       
            
            MARK(4);
            sched_yield();

            MARK(6);

            exit(0);
       }

       

       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);


       p.sched_priority = 50;
       p.requested_time = 3000; 
       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son1, SCHED_SHORT, &p) >= 0);


       p.sched_priority = 50;
       p.requested_time = 3000; 
       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son2, SCHED_SHORT, &p) >= 0);

      

       sched_yield();
       MARK(1);

       *ready1=1;

       sched_yield();
       MARK(2);

       while(wait(NULL) != -1);

       MARK(8);
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//RT back from wait vs short
bool winter17_test12() {
    int tester=fork();
    if(tester==0){
      

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;
         ready3 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        *ready3=0;

       
       int son1= fork();
       if(son1==0){
            while(!*ready1){};    
            MARK(2);
            sched_yield();
            MARK(3);
            mil_sleep(50);
            MARK(5);
            *ready2=1;
            sched_yield();
            MARK(6);
            *ready3=1;
            sched_yield();
            MARK(7);
            exit(0);
       }

      
       int son2= fork();
       if(son2==0){
            while(!*ready1){};  
            
            MARK(4);
        
            while(!*ready3){
              if(*ready2){
                MARK(8)
              }
            }

            MARK(9);

            exit(0);
       }

       

       sched_yield();
       MARK(0);
       sched_yield();

       //make me RT
       p.sched_priority = 50; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_RR, &p) >= 0);


       p.sched_priority = 40;
       p.requested_time = 3000; 
       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son1, SCHED_RR, &p) >= 0);


       p.sched_priority = 50;
       p.requested_time = 3000; 
       p.sched_short_prio = 10;
       ASSERT_TEST( sched_setscheduler(son2, SCHED_SHORT, &p) >= 0);

      
       sched_yield();
       MARK(1);
       *ready1=1;
       while(wait(NULL) != -1);

     
       MARK(10);

 
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       munmap(ready2, sizeof(ready2));
       munmap(ready3, sizeof(ready3));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}


//fork short, father should run first
bool winter17_test13() {
    int tester=fork();
    if(tester==0){
      

        int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;


        //make me short
       p.sched_priority = 40;
       p.requested_time = 3000; 
       p.sched_short_prio = 10; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_SHORT, &p) >= 0);

        MARK(0);
       int son1= fork();
       if(son1==0){
            MARK(2);
            exit(0);
       }

       MARK(1);
       sched_yield();
       MARK(3);
       while(wait(NULL) != -1);
       MARK(4);


   
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       munmap(ready2, sizeof(ready2));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}

//half timeslice after fork
bool winter17_test14() {
    int tester=fork();
    if(tester==0){
      

         int arr_size=20;
        histo = mmap(NULL, sizeof(int)*arr_size , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        index = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready1 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        ready2 = mmap(NULL, sizeof(int) , PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        struct sched_param p;
        for(i=0;i<arr_size;i++){
            histo[i]=BIG_NUM;
        }
        
        *index=0;
        *ready1=0;  
        *ready2=0;


        //make me short
       p.sched_priority = 40;
       p.requested_time = 3000; 
       p.sched_short_prio = 10; 
       ASSERT_TEST( sched_setscheduler(getpid(), SCHED_SHORT, &p) >= 0);

       int time_remain;
       time_remain= short_remaining_time(getpid());
       ASSERT_TEST( (time_remain>2700) && (time_remain<3300) );


       MARK(0);
       int son1= fork();
       if(son1==0){
            time_remain= short_remaining_time(getpid());
            ASSERT_TEST( (time_remain>1200) && (time_remain<1800) );
            MARK(3);
            exit(0);
       }

       MARK(1);
       time_remain= short_remaining_time(getpid());
       ASSERT_TEST( (time_remain>1200) && (time_remain<1800) );
        

       MARK(2);

       sched_yield();

       MARK(4);
       while(wait(NULL) != -1);
       MARK(5);


 
       ASSERT_TEST( increasing(histo,arr_size));
       munmap(histo, sizeof(histo));
       munmap(index, sizeof(index));
       munmap(ready1, sizeof(ready1));
       munmap(ready2, sizeof(ready2));
       exit(0);

  }
  while(wait(NULL) != -1);
  return true; 
}



int main(){
    setbuf(stdout, NULL);
  
    RUN_TEST(test_the_test);
   
    RUN_TEST(winter17_test0);        //other vs other
    RUN_TEST(winter17_test3);        //short vs other
    RUN_TEST(winter17_test4);        //overdue vs other
    RUN_TEST(winter17_test5);        //short vs short
    RUN_TEST(winter17_test6);        //short run in fifo
    RUN_TEST(winter17_test8);        //short vs other vs overdue
    RUN_TEST(winter17_test11_a);     //short yield to lower prio
    RUN_TEST(winter17_test11_b);     //short yield to same prio
    RUN_TEST(winter17_test12);       //RT back from wait vs short
    RUN_TEST(winter17_test13);       //fork short, father should run first
    RUN_TEST(winter17_test14);       //half timeslice after fork
    return 0;
}
