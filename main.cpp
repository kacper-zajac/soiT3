//
//
//
//
//
//
#include <random>
#include "sem.h"

#include <pthread.h>

#define BUFSIZE 25

struct bufor {
    int even = 0;
    int odd = 0;
    int buf[BUFSIZE];
};

struct bufor b;
int prodEvenWaiting = 0;
int prodOddWaiting = 0;
int consEvenWaiting = 0;
int consOddWaiting = 0;

Semaphore static mutex(1);
Semaphore static mutexProdOdd(1);
Semaphore static mutexProdEven(1);
Semaphore static mutexConsOdd(1);
Semaphore static mutexConsEven(1);

//static struct sembuf xyz;
// flaga 0600

thread_local std::mt19937 gen{std::random_device{}()};

void showStatus(){
    for (int i = 0; i < b.odd+b.even ; ++i) {
        printf(" %d |",b.buf[i]);
    }
    printf("\n");
}

template<typename T>
T random(T min, T max) {
    return std::uniform_int_distribution<T>{min, max}(gen);
}

int getFromBuf();

void putToBuf(int val);

bool prodOddCanProduce() {
    return b.odd < b.even;
}

bool prodEvenCanProduce() {
    return b.even < 10;
}

bool consOddCanConsume() {
    return b.even + b.odd > 6;
}

bool consEvenCanConsume() {
    return b.even + b.odd > 2;
}

void produceOddNum() {
    unsigned int x = (1 + 2 * random(0, 49));
    putToBuf(x);
}

void produceEvenNum() {
    unsigned int x = (2 * random(0, 49));
    putToBuf(x);
}

void consumeEvenNum() {
    if (b.buf[0] % 2 == 0) {
        int x = getFromBuf();
    } else printf("Can't consume even\n");
}

void consumeOddNum() {
    if (b.buf[0] % 2 != 0) {
        int x = getFromBuf();
    } else printf("Can't consume odd\n");
}


void* prodOdd(void *arg) {
    mutex.P();
    if (!prodOddCanProduce()) {
        printf("Can't\n");
        ++prodOddWaiting;
        mutex.V();
        mutexProdOdd.P();
    }
    produceOddNum();
    if (prodEvenCanProduce() && prodEvenWaiting > 0) {
        --prodEvenWaiting;
        mutexProdEven.V();
    } else if (consOddCanConsume() && consOddWaiting > 0) {
        --consOddWaiting;
        mutexConsOdd.V();
    } else if (consEvenCanConsume() && consEvenWaiting > 0) {
        --consEvenWaiting;
        mutexConsEven.V();
    } else mutex.V();
}

void* prodEven(void *arg) {
    mutex.P();
    if (!prodEvenCanProduce()) {
        printf("Can't\n");
        ++prodEvenWaiting;
        mutex.V();
        mutexProdEven.P();
    }
    produceEvenNum();
    if (prodOddCanProduce() && prodOddWaiting > 0) {
        --prodOddWaiting;
        mutexProdOdd.V();
    } else if (consEvenCanConsume() && consEvenWaiting > 0) {
        --consEvenWaiting;
        mutexConsEven.V();
    } else if (consOddCanConsume() && consOddWaiting > 0) {
        --consOddWaiting;
        mutexConsOdd.V();
    } else mutex.V();
}

void* consEven(void *arg) {
    mutex.P();
    if (!consEvenCanConsume()) {
        printf("Can't\n");
        ++consEvenWaiting;
        mutex.V();
        mutexConsEven.P();
    }
    consumeEvenNum();
    if (consOddCanConsume() && consOddWaiting > 0) {
        --consOddWaiting;
        mutexConsOdd.V();
    } else if (prodEvenCanProduce() && prodEvenWaiting > 0) {
        --prodEvenWaiting;
        mutexProdEven.V();
    } else if (prodOddCanProduce() && prodOddWaiting > 0) {
        --prodOddWaiting;
        mutexProdOdd.V();
    } else mutex.V();
}

void* consOdd(void *arg) {
    mutex.P();
    if (!consOddCanConsume()) {
        printf("Can't\n");
        ++consOddWaiting;
        mutex.V();
        mutexConsOdd.P();
    }
    consumeOddNum();
    if (consEvenCanConsume() && consEvenWaiting > 0) {
        --consEvenWaiting;
        mutexConsEven.V();
    } else if (prodEvenCanProduce() && prodEvenWaiting > 0) {
        --prodEvenWaiting;
        mutexProdEven.V();
    } else if (prodOddCanProduce() && prodOddWaiting > 0) {
        --prodOddWaiting;
        mutexProdOdd.V();
    } else mutex.V();
}


void putToBuf(int val) {
    if ((val % 2) == 0) {
        if (b.even < 10) {
            b.buf[b.even + b.odd] = val;
            b.even++;
            printf("Wsadzam %d, odd: %d, even: %d\n", val, b.odd, b.even);
        } else printf("Nie wsadzam nic, nie sa spelnione warunki\n");
    } else if (b.even > b.odd) {
        b.buf[b.even + b.odd] = val;
        b.odd++;
        printf("Wsadzam %d, odd: %d, even: %d\n", val, b.odd, b.even);
    } else printf("Nie wsadzam nic, nie sa spelnione warunki\n");
}

int bufPop() {
    int x = b.buf[0];
    for (int i = 0; i < b.odd + b.even - 1; ++i) {
        b.buf[i] = b.buf[i + 1];
    }
    return x;
}

int getFromBuf() {   // if -1 nothing happend
    int x = -1;
    if (b.buf[0] % 2 == 0) {
        if (b.even + b.odd >= 3) {
            x = bufPop();
            b.even--;
            printf("Wyjmuje %d\n", x);
        } else printf("Nic nie wyjmuje, nie sa spelnione warunki \n");
    } else if (b.even + b.odd >= 7) {
        x = bufPop();
        b.odd--;
        printf("Wyjmuje %d\n", x);
    } else printf("Nic nie wyjmuje, nie sa spelnione warunki \n");
    return x;

}

pthread_t tid;
int main() {
    int i = 0;
    int err;
    int x = random(1,4);
    while(1)
    {   switch (x) {
            case 1:
                err = pthread_create(&(tid), NULL, &prodEven, NULL);
                printf("1");
                break;
            case 2:
                err = pthread_create(&(tid), NULL, &prodOdd, NULL);
                printf("2");
                break;
            case 3:
                err = pthread_create(&(tid), NULL, &consEven, NULL);
                printf("3");
                break;
            case 4:
                err = pthread_create(&(tid), NULL, &consOdd, NULL);
                printf("4");
                break;
        }
        // pthread_join(tid, NULL);
        showStatus();
        x = random(1,4);
        //sleep(1);
    }
    sleep(3);
    return 0;
}
