/* transProg.c use ptheard with c
Wanyu Dong
02.21.2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

/*--------------------------------------------------------
Global variables
--------------------------------------------------------*/
int numOfWorkers = 0;   //worker number
int numOfMaxWait = 100; //waitQueue number per woker
int numOfAccount = 0;   //recording added account number
int numOfMaxAcc = 20000; //maximum account number
int addAccountDone = 0;  //worker wait for init
int allDone = 0;  //Producer signal wokers there are no jobs
int over = 0;  //Woker signal Producer that all jobs are done
sem_t sover;
sem_t acc;

/*--------------------------------------------------------
structure define
--------------------------------------------------------*/
typedef struct
{
    int id;
    int blance;
    int F;
} AccountInfo;
AccountInfo accountBuf[20000];

typedef struct
{
    int fromId[100];
    int toId[100];
    int money[100];
    int cnt;
    pthread_mutex_t mtx;
    pthread_cond_t condE;
    pthread_cond_t condF;
} WaitBuf;
WaitBuf waitQueue[100];

/*--------------------------------------------------------
function define
--------------------------------------------------------*/
void init()
{
    sem_init(&sover, 0, 1);
    sem_init(&acc, 0, 1);
    for (int i = 0; i < numOfWorkers; i++)
    {
        waitQueue[i].cnt = 0;
        waitQueue[i].mtx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        waitQueue[i].condE = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        waitQueue[i].condF = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
    }
    // printf("Finish Init! \n");
};

void initAccount(int i)
{
    accountBuf[i].id = 0;
    accountBuf[i].blance = 0;
    accountBuf[i].F = 1;
}

/*Producer
To init account buffer and allocate jobs to each worker in turn.
Add account info to buffer, add Transfer info to each worker queue.*/
void *Producer(void *fpt)
{

    char *line;
    ssize_t read;
    size_t len = 0;

    int i = 0;           // account buffer index
    int j = 0;           // wait queue buffer inner index
    int m = 0;           // wait queue buffer index
    int workerIndex = 0; // woker index
    char *strTemp;

    while (1)
    {

        read = getline(&line, &len, fpt);
        if (read == -1)
        {
            // printf("All Done! \n");
            allDone = 1;
            break;
        }

        strTemp = strtok(line, " \n");
        if (strcmp(strTemp, "Transfer") == 0) // Allocate job to workers.
        {
            // printf("Strat Transfer! \n");
            addAccountDone = 1;

            m = workerIndex % numOfWorkers;
            pthread_mutex_lock(&waitQueue[m].mtx);
            while (waitQueue[m].cnt == numOfMaxWait) // wait if the wait queue reaches maximum.
            {
                pthread_cond_wait(&waitQueue[m].condF, &waitQueue[m].mtx);
            }
            j = waitQueue[m].cnt;
            strTemp = strtok(NULL, " \n");
            waitQueue[m].fromId[j] = atoi(strTemp);
            strTemp = strtok(NULL, " \n");
            waitQueue[m].toId[j] = atoi(strTemp);
            strTemp = strtok(NULL, " \n");
            waitQueue[m].money[j] = atoi(strTemp);
            waitQueue[m].cnt++;
            pthread_cond_broadcast(&waitQueue[m].condE);
            pthread_mutex_unlock(&waitQueue[m].mtx);
            workerIndex++;
        }
        else  // Add account.
        {
            i = numOfAccount;
            initAccount(i);
            accountBuf[i].id = atoi(strTemp);
            strTemp = strtok(NULL, " \n");
            accountBuf[i].blance = atoi(strTemp);
            numOfAccount++;
            i = 0;
        }
    }

    for (int i = 0; i < numOfWorkers; i++)
        pthread_cond_broadcast(&waitQueue[i].condE);

    while(1){
        sem_wait(&sover);
        // printf("over = %d\n", over);
        // If all workers are done
        if(over == numOfWorkers){
            for (int n = 0; n < numOfAccount; n++)
                printf("%d %d\n", accountBuf[n].id, accountBuf[n].blance);
            sem_post(&sover);
            break;
        }
        sem_post(&sover);
    }

    pthread_exit(NULL);
}

/*Worker
Remove and Handle Transfer info in own wait queue buffer.
Need sync with Proucer on wait queue buffer.
Need sync with Other workers on account buffer.*/
void *Worker(void *threadid)
{

    long tid;
    tid = (long)threadid;

    int i = 0;       // wait queue buffer inner index
    int fromId = 0;
    int toId = 0;
    int money = 0;
    int fromIdi = 0;
    int toIdi = 0;
    int save = 0;

    while (addAccountDone == 0)
        ;

    while (1)
    {

        pthread_mutex_lock(&waitQueue[tid].mtx);
        while (waitQueue[tid].cnt == 0 && allDone == 0)  // wait if the wait queue is empty.
        {
            pthread_cond_wait(&waitQueue[tid].condE, &waitQueue[tid].mtx);
        }
        if ((allDone == 1) && (waitQueue[tid].cnt == 0))
        {
            sem_wait(&sover);
            over = over + 1;
            sem_post(&sover);
            break;
        }

        //
        i = 0;
        while (waitQueue[tid].cnt > 0)
        {
            if(!save)
            {
                fromId = waitQueue[tid].fromId[i];
                toId = waitQueue[tid].toId[i];
                money = waitQueue[tid].money[i];
                // printf("Transfer %d %d %d\n", fromId, toId, money);

                i = i + 1;
                waitQueue[tid].cnt = waitQueue[tid].cnt - 1;


                for (int n = 0; n < numOfAccount; n++)
                    if (fromId == accountBuf[n].id)
                    {
                        fromIdi = n;
                        break;
                    }

                for (int m = 0; m < numOfAccount; m++)
                    if (toId == accountBuf[m].id)
                    {
                        toIdi = m;
                        break;
                    }
            }

            sem_wait(&acc);

            if(accountBuf[fromIdi].F == 0 || accountBuf[toIdi].F == 0)
            {
                save = 1;
                sem_post(&acc);
            }else
            {
                save = 0;
                accountBuf[fromIdi].F = 0;
                accountBuf[toIdi].F = 0;
                sem_post(&acc);

                accountBuf[fromIdi].blance -= money;
                accountBuf[toIdi].blance += money;

                sem_wait(&acc);
                accountBuf[fromIdi].F = 1;
                accountBuf[toIdi].F = 1;
                sem_post(&acc);
            }

        }
        //
        pthread_cond_broadcast(&waitQueue[tid].condF);
        pthread_mutex_unlock(&waitQueue[tid].mtx);
    }

    pthread_exit(NULL);
}

/*--------------------------------------------------------
main function
--------------------------------------------------------*/
int main(int argc, char const *argv[])
{
    // Checking for input arguments
    if (argc != 3)
    {
        if (argc < 3)
            printf("Insufficient arguments passed\n");
        else
            printf("Too many arguments passed\n");
        printf("usage: pass an input text file containing words\n");
        return 1;
    }

    if (atoi(argv[2]) <= 0)
    {
        printf("Err: Number of wokers should greater than 0.\n");
        return 1;
    }

    FILE *fpt;
    fpt = fopen(argv[1], "r");
    if (fpt == NULL)
    {
        printf("Err: can't open input file!\n");
    }

    numOfWorkers = atoi(argv[2]);
    pthread_t tidP;
    pthread_t tidW[numOfWorkers];

    // init wait queue for each worker
    init();

    // Create Producer Thread
    pthread_create(&tidP, NULL, Producer, fpt);

    // Create Worker Thread
    for (long i = 0; i < numOfWorkers; i++)
    {
        pthread_create(&tidW[i], NULL, Worker, (void *)i);
    }

    pthread_exit(NULL);
}
