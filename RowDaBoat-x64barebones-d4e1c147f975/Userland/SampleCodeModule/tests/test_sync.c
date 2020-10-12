#include <stdint.h>
#include <std_io.h>
#include <syscalls.h>

// FALTA PROBAR

uint64_t my_create_process(char *name)
{
    return 0;
}

uint64_t my_sem_wait(char *sem_id)
{
    return 0;
}

uint64_t my_sem_post(char *sem_id)
{
    return 0;
}

uint64_t my_sem_close(char *sem_id)
{
    return 0;
}

#define TOTAL_PAIR_PROCESSES 2
#define SEM_ID 7

int64_t global; //shared memory

void slowInc(int64_t *p, int64_t inc)
{
    int64_t aux = *p;
    aux += inc;
    runNext();
    *p = aux;
}

void inc(uint64_t sem, int64_t value, uint64_t N)
{
    uint64_t i;
    printf("sem = %d, value = %d, N = %d\n", (int)sem, (int)value, (int)N);
    sem_t *semaph;
    if(sem){
        semaph = semOpen(SEM_ID, 1);
    }

    if (sem && semaph == NULL)
    {
        printf("ERROR OPENING SEM\n");
        return;
    }

    for (i = 0; i < N; i++)
    {
        if (sem)
            semWait(semaph);
        
        slowInc(&global, value);

        if (sem)
            semPost(semaph);
    }

    if (sem)
        semClose(semaph);

    printf("Final value: %d\n", global);
    exit();
}

void test_sync()
{
    uint64_t i;

    global = 0;
    const char *args1[4] = {"inc", "1", "1", "1000000"};
    const char *args2[4] = {"inc", "1", "-1", "1000000"};

    printf("CREATING PROCESSES...(WITH SEM)\n");

    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
    {
        startProcessBg((int (*)(int, const char **))inc, 4, args1);
        startProcessBg((int (*)(int, const char **))inc, 4, args2);
    }
    exit();
}

void test_no_sync()
{
    uint64_t i;

    global = 0;
    const char *args1[4] = {"inc", "0", "1", "1000000"};
    const char *args2[4] = {"inc", "0", "-1", "1000000"};


    printf("CREATING PROCESSES...(WITHOUT SEM)\n");

    for (i = 0; i < TOTAL_PAIR_PROCESSES; i++)
    {
        startProcessBg((int (*)(int, const char **))inc, 4, args1);
        startProcessBg((int (*)(int, const char **))inc, 4, args2);
    }
    exit();
}

// int main()
// {
//     test_sync();
//     return 0;
// }
