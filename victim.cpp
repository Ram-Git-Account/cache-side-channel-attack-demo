#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#define PAGE_SIZE 4096
#define STRIDE 64

int main()
{
    int fd = open("shared_mem.bin", O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (ftruncate(fd, PAGE_SIZE) != 0) {
        perror("ftruncate");
        return 1;
    }

    unsigned char *shared =
        (unsigned char*) mmap(NULL, PAGE_SIZE,
        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    int secret = 60;

    std::cout << "Victim accessing index: " << secret << std::endl;

    while (true)
    {
        volatile unsigned char *addr = &shared[secret * STRIDE];
        *addr;   // repeatedly touch the cache line
    }

    return 0;
}