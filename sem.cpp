#include <stdexcept>
#include "sem.h"

Semaphore::Semaphore( int value )
{
    if(sem_init( & sem, 0, value ) != 0 )
        throw std::runtime_error("sem_init: failed");
}
Semaphore::~Semaphore()
{
    sem_destroy( & sem );
}

void Semaphore::P()
{
    if(sem_wait( & sem ) != 0 )
        throw std::runtime_error("sem_wait: failed");
}

void Semaphore::V() {
    if(sem_post( & sem ) != 0 )
        throw std::runtime_error("sem_post: failed");
}
