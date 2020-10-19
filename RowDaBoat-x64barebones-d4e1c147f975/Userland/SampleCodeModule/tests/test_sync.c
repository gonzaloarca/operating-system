#include <sem.h>
#include <std_io.h>
#include <std_num.h>
#include <stdint.h>
#include <test_util.h>

#define TOTAL_PAIR_PROCESSES 2
#define SEM_ID 7854
#define PRINT_ID 3268 //Pipe agregado para imprimir global en el padre

int64_t global; //shared memory

void slowInc(int64_t *p, int64_t inc) {
	int64_t aux = *p;
	aux += inc;

	runNext();

	*p = aux;
}

void inc(uint64_t sem, int64_t value, uint64_t N) {
	uint64_t i;
	Semaphore *semaph;

	if(sem) {
		semaph = semOpen(SEM_ID, 1);
	}

	if(sem && semaph == NULL) {
		printf("ERROR OPENING SEM\n");
		return;
	}

	for(i = 0; i < N; i++) {
		if(sem)
			semWait(semaph);

		slowInc(&global, value);

		if(sem)
			semPost(semaph);
	}

	if(sem)
		semClose(semaph);

	printf("Final value: %d\n", global);
	return;
}

int incMain(int argc, char *argv[]) {
	int fd[2];

	if(pipeOpen(PRINT_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return 1;
	}

	dup2(fd[1], 1);
	pipeClose(fd[0]);
	pipeClose(fd[1]);

	uint64_t sem = (uint64_t)strToPositiveInt(argv[1], NULL);

	int64_t value;
	if(argv[2][0] == '-') {
		value = (-1) * (int64_t)strToPositiveInt(argv[2] + 1, NULL);
	} else {
		value = (int64_t)strToPositiveInt(argv[2], NULL);
	}

	uint64_t n = (uint64_t)strToPositiveInt(argv[3], NULL);

	inc(sem, value, n);

	pipeClose(1);

	return 0;
}

void test_sync() {
	uint64_t i;
	int fd[2], len;
	char buffer[100];

	if(pipeOpen(PRINT_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	global = 0;
	char *args1[4] = {"inc", "1", "1", "100"};
	char *args2[4] = {"inc", "1", "-1", "100"};

	printf("CREATING PROCESSES...(WITH SEM)\n");

	for(i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
		startProcessBg((int (*)(int, const char **))incMain, 4, (const char **)args1);
		startProcessBg((int (*)(int, const char **))incMain, 4, (const char **)args2);
	}
	for(int i = 0; i < TOTAL_PAIR_PROCESSES * 2; i++) {
		len = read(fd[0], buffer, 100);
		write(1, buffer, len);
	}

	pipeClose(fd[0]);
	pipeClose(fd[1]);

	return;
}

void test_no_sync() {
	uint64_t i;

	global = 0;
	char *args1[4] = {"inc", "0", "1", "100"};
	char *args2[4] = {"inc", "0", "-1", "100"};

	printf("CREATING PROCESSES...(WITHOUT SEM)\n");

	for(i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
		startProcessBg((int (*)(int, const char **))incMain, 4, (const char **)args1);
		startProcessBg((int (*)(int, const char **))incMain, 4, (const char **)args2);
	}
	exit();
}
