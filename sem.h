#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

class Semaphore
{
public:

    Semaphore( int value );
    ~Semaphore();
    void P();
    void V();

private:
    sem_t sem;
};

