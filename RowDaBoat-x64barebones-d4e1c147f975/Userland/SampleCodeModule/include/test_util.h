#include <stdint.h>

uint32_t GetUint();
uint32_t GetUniform(uint32_t max);
uint8_t memcheck(void *start, uint8_t value, uint32_t size);

void test_mm();
void test_prio();
void test_processes();
void test_sync();
void test_no_sync();
void test_pipe();
