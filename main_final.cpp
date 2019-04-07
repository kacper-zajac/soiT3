//
//
//
//
//
//
#include <random>

#include <pthread.h>
#include <semaphore.h>
#include <zconf.h>

#define BUFSIZE 20

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

sem_t mutex;
sem_t mutexProdOdd;
sem_t mutexProdEven;
sem_t mutexConsOdd;
sem_t mutexConsEven;


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
    return b.even + b.odd > 6 && b.buf[0] %2 != 0;
}

bool consEvenCanConsume() {
    return b.even + b.odd > 2 && b.buf[0] %2 == 0;
}

void produceOddNum() {
    sem_wait(&mutexProdOdd);
    unsigned int x = (1 + 2 * random(0, 49));
    putToBuf(x);
    sem_post(&mutexProdOdd);
}

void produceEvenNum() {
    sem_wait(&mutexProdEven);
    unsigned int x = (2 * random(0, 49));
    putToBuf(x);
    sem_post(&mutexProdEven);
}

void consumeEvenNum() {
    sem_wait(&mutexConsEven);
    int x = getFromBuf();
    sem_post(&mutexConsEven);
}

void consumeOddNum() {
    sem_wait(&mutexConsOdd);
    int x = getFromBuf();
    sem_post(&mutexConsOdd);
}


void* prodOdd(void *arg) {
    sem_wait(&mutex);
    if (!prodOddCanProduce()) {
        printf("Can't\n");
        ++prodOddWaiting;
        sem_post(&mutex);
        //sem_wait(&mutexProdOdd);
    }
    produceOddNum();
    if (prodEvenCanProduce() && prodEvenWaiting > 0) {
        --prodEvenWaiting;
        sem_post(&mutexProdEven);
    } else if (consOddCanConsume() && consOddWaiting > 0) {
        --consOddWaiting;
        sem_post(&mutexConsOdd);
    } else if (consEvenCanConsume() && consEvenWaiting > 0) {
        --consEvenWaiting;
        sem_post(&mutexConsEven);
    } else sem_post(&mutex);
}

void* prodEven(void *arg) {
    sem_wait(&mutex);
    if (!prodEvenCanProduce()) {
        printf("Can't\n");
        ++prodEvenWaiting;
        sem_post(&mutex);
        //sem_wait(&mutexProdEven);
    }
    produceEvenNum();
    if (prodOddCanProduce() && prodOddWaiting > 0) {
        --prodOddWaiting;
        sem_post(&mutexProdOdd);
    } else if (consEvenCanConsume() && consEvenWaiting > 0) {
        --consEvenWaiting;
        sem_post(&mutexConsEven);
    } else if (consOddCanConsume() && consOddWaiting > 0) {
        --consOddWaiting;
        sem_post(&mutexConsOdd);
    }else sem_post(&mutex);
}

void* consEven(void *arg) {
    sem_wait(&mutex);
    if (!consEvenCanConsume()) {
        printf("Can't\n");
        ++consEvenWaiting;
        sem_post(&mutex);
        //sem_wait(&mutexConsEven);
    }
    consumeEvenNum();
    if (consOddCanConsume() && consOddWaiting > 0) {
        --consOddWaiting;
        sem_post(&mutexConsOdd);
    } else if (prodEvenCanProduce() && prodEvenWaiting > 0) {
        --prodEvenWaiting;
        sem_post(&mutexProdEven);
    } else if (prodOddCanProduce() && prodOddWaiting > 0) {
        --prodOddWaiting;
        sem_post(&mutexProdOdd);
    } else sem_post(&mutex);
}

void* consOdd(void *arg) {
    sem_wait(&mutex);
    if (!consOddCanConsume()) {
        printf("Can't\n");
        ++consOddWaiting;
        sem_post(&mutex);
        //sem_wait(&mutexConsOdd);
    }
    consumeOddNum();
    if (consEvenCanConsume() && consEvenWaiting > 0) {
        --consEvenWaiting;
        sem_post(&mutexConsEven);
    } else if (prodEvenCanProduce() && prodEvenWaiting > 0) {
        --prodEvenWaiting;
        sem_post(&mutexProdEven);
    } else if (prodOddCanProduce() && prodOddWaiting > 0) {
        --prodOddWaiting;
        sem_post(&mutexProdOdd);
    } else sem_post(&mutex);
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

pthread_t tid1;
pthread_t tid2;
pthread_t tid3;
pthread_t tid4;

int main() {
    int i = 0;
    int err;
    int x = random(1,4);

    sem_init(&mutex, 0, 1);
    sem_init(&mutexConsEven, 0, 1);
    sem_init(&mutexConsOdd, 0, 1);
    sem_init(&mutexProdEven, 0, 1);
    sem_init(&mutexProdOdd, 0, 1);

    while(1)
    {
        printf("\n---- %d ----\n\n", i++);
        switch (x) {

            case 1:
                printf("1 ");
                pthread_create(&(tid1), NULL, &prodEven, NULL);
                pthread_join(tid1, NULL);
                break;
            case 2:
                printf("2 ");
                pthread_create(&(tid2), NULL, &prodOdd, NULL);
                pthread_join(tid2, NULL);
                break;
            case 3:
                printf("3 ");
                pthread_create(&(tid3), NULL, &consEven, NULL);
                pthread_join(tid3, NULL);
                break;
            case 4:
                printf("4 ");
                pthread_create(&(tid4), NULL, &consOdd, NULL);
                pthread_join(tid4, NULL);
                break;
        }
        showStatus();
        x = random(1,4);
    }
}
