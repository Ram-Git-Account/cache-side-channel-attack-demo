#include <x86intrin.h>
#include <stdio.h>
#include <stdint.h>
#include <iostream>

using namespace std;

char a;

uint64_t measure_access_time(volatile  char *addr) {
    uint64_t start, end;

    _mm_mfence();      // ensure previous memory ops complete
    _mm_lfence();      // serialize instruction stream

    start = __rdtsc();

    *addr;             // memory access

    _mm_lfence();      // prevent reordering
    end = __rdtsc();

    return end - start;
}

int main() {

    volatile  char *addr = &a;

    

    // Flush from cache
    _mm_clflush((void*)addr);

    _mm_mfence();   // ensure flush completes

    uint64_t time1 = measure_access_time(addr);
    uint64_t time2 = measure_access_time(addr);

    printf("Access time: %lu cycles\n", time1);
    printf("Access time: %lu cycles\n", time2);

    return 0;
}