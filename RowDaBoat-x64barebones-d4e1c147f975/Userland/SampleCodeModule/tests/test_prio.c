#include <stdint.h>
#include <std_io.h>
#include <syscall.h>

#define MINOR_WAIT 1000000                               // TODO: To prevent a process from flooding the screen
#define WAIT      10000000                              // TODO: Long enough to see theese processes beeing run at least twice

void bussy_wait(uint64_t n){
  uint64_t i;
  for (i = 0; i < n; i++);
}

static void endless_loop(){
  uint64_t pid = getpid();

  while(1){
    printf("%d ",pid);
    bussy_wait(MINOR_WAIT);
  }
}

#define TOTAL_PROCESSES 3

void test_prio(){
  uint64_t pids[TOTAL_PROCESSES];
  uint64_t i;

  for(i = 0; i < TOTAL_PROCESSES; i++)
    pids[i] = startProcessBg((int (*)(int, const char **)) endless_loop, 0, NULL);

  bussy_wait(WAIT);
  printf("\nCHANGING PRIORITIES...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++){
    switch (i % 3){
      case 0:
        nice(pids[i], 4); //lowest priority 
        break;
      case 1:
        nice(pids[i], 2); //medium priority
        break;
      case 2:
        nice(pids[i], 0); //highest priority
        break;
    }
  }

  bussy_wait(WAIT);
  printf("\nBLOCKING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++)
    kill(pids[i], BLOCKED);

  printf("CHANGING PRIORITIES WHILE BLOCKED...\n");
  for(i = 0; i < TOTAL_PROCESSES; i++){
    switch (i % 3){
      case 0:
        nice(pids[i], 2); //medium priority 
        break;
      case 1:
        nice(pids[i], 2); //medium priority
        break;
      case 2:
        nice(pids[i], 2); //medium priority
        break;
    }
  }

  printf("UNBLOCKING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++)
    kill(pids[i], ACTIVE);

  bussy_wait(WAIT);
  printf("\nKILLING...\n");

  for(i = 0; i < TOTAL_PROCESSES; i++)
    kill(pids[i], KILLED);
}

// int main(){
//   test_prio();
//   return 0;
// }
