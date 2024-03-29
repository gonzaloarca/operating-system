#include <std_io.h>
#include <syscalls.h>
#include <test_util.h>

#include <comandos.h>

//TO BE INCLUDED
static void endless_loop() {
	while(1)
		;
}

#define MAX_PROCESSES 10 //Should be around 80% of the the processes handled by the kernel

enum State { ERROR,
	     RUNNING,
	     AUX_BLOCKED,
	     AUX_KILLED }; // Nota: Cambie de nombre los ultimos dos estados que se usan internamente para la syscall kill

typedef struct P_rq {
	uint32_t pid;
	enum State state;
} p_rq;

void test_processes() {
	p_rq p_rqs[MAX_PROCESSES];
	uint8_t rq;
	uint8_t alive = 0;
	uint8_t action;

	while(1) {
		// Create MAX_PROCESSES processes
		for(rq = 0; rq < MAX_PROCESSES; rq++) {
			p_rqs[rq].pid = startProcessBg((int (*)(int, const char **))endless_loop, 0, NULL);
			if(p_rqs[rq].pid == -1) {
				printf("Error creating process\n");
				return;
			} else {
				p_rqs[rq].state = RUNNING;
				alive++;
			}
		}
		// Randomly kills, blocks or unblocks processes until every one has been killed
		while(alive > 0) {

			for(rq = 0; rq < MAX_PROCESSES; rq++) {
				action = GetUniform(2) % 2;
				printf("%u\n", (unsigned int)p_rqs[rq].pid);
				switch(action) {
				case 0:
					if(p_rqs[rq].state == RUNNING || p_rqs[rq].state == AUX_BLOCKED) {
						if(kill(p_rqs[rq].pid, KILLED) == -1) {
							listProcess();
							printf("Error killing process %u\n", (unsigned int)p_rqs[rq].pid);
							return;
						}
						p_rqs[rq].state = AUX_KILLED;
						alive--;
					}
					break;

				case 1:
					if(p_rqs[rq].state == RUNNING) {
						if(block(p_rqs[rq].pid) == -1) {
							printf("Error blocking process %u\n", (unsigned int)p_rqs[rq].pid);
							return;
						}
						p_rqs[rq].state = AUX_BLOCKED;
					}
					break;
				}
			}

			// Randomly unblocks processes
			for(rq = 0; rq < MAX_PROCESSES; rq++)
				if(p_rqs[rq].state == AUX_BLOCKED && GetUniform(2) % 2) {
					if(unblock(p_rqs[rq].pid) == -1) {
						printf("Error unblocking process %u\n", (unsigned int)p_rqs[rq].pid);
						return;
					}
					p_rqs[rq].state = RUNNING;
				}
		}
	}
}

// int main(){
//   test_processes();
//   return 0;
// }
