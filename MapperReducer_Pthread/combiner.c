// combiner.c  ptheard
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>


int numMaxTuples = 1000;  //max number of input tuples 
int numMaxTopic = 30;  //max number of topics per user

// Global variables
int numOfSlots;
int numOfUsers;
char **userMap = {NULL};
int allDone = 0;

typedef struct
{
    char **tuples;
    pthread_mutex_t mtx;
    pthread_cond_t condE;
    pthread_cond_t condF;
    int count;
} Buffer;
Buffer buf[100];

struct outputTuple
{
    char userID[20];
    char topic[20];
    int score;
};

int actionMap(char action)
{
    switch (action)
    {
    case 'P':
        return 50;
        break;
    case 'L':
        return 20;
        break;
    case 'D':
        return -10;
        break;
    case 'C':
        return 30;
        break;
    case 'S':
        return 40;
        break;
    default:
        fprintf(stderr, "No such action!\n");
        exit(EXIT_FAILURE);
        break;
    }
}

void init()
{

    userMap = malloc(numOfUsers * sizeof(char *));

    for (int i = 0; i < numOfUsers; i++)
    {
        buf[i].tuples = malloc(numOfSlots * sizeof(char *));
        buf[i].mtx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
        buf[i].condE = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        buf[i].condF = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
        buf[i].count = 0;
    }
};

void addToBuffer(int bufIndex, char *tuple, int index)
{
    char *copy = NULL;
    copy = malloc(strlen(tuple) * sizeof(char));
    copy = strcpy(copy, tuple);
    buf[bufIndex].tuples[index] = copy;
    buf[bufIndex].count = buf[bufIndex].count + 1;
}

void *Mapper(void *threadid)
{

    char *strTemp;
    char *userIDTemp;
    char *topicTemp;
    int scoreTemp;
    char s;
    char oneTuple[30];

    int tupleSize = 0;
    struct outputTuple mapTotal[numMaxTuples];
    int t = 0;

    int flag = 0;
    int userBufIndex;
    int userSize = 0;
    int tupleIndex = 0;
    int new = 0;

    char *dataIn;
    ssize_t readDataIn;
    size_t len = 0;
    int cnt = 0;

    // Mapper input data an store it in the mapTotal
    readDataIn = getline(&dataIn, &len, stdin);
    strTemp = strtok(dataIn, ",()");
    while (strTemp != NULL)
    {
        switch (cnt % 3)
        {
        case 0:
            // userIDTemp = strTemp;
            strcpy(mapTotal[tupleSize].userID, strTemp);
            break;
        case 1:
            s = *strTemp;
            // scoreTemp = actionMap(s);
            mapTotal[tupleSize].score = actionMap(s);
            break;
        case 2:
            // topicTemp = strTemp;
            strcpy(mapTotal[tupleSize].topic, strTemp);
            // printf("(%s,%s,%d)\n", mapTotal[tupleSize].userID,mapTotal[tupleSize].topic, mapTotal[tupleSize].score);
            tupleSize++;
        default:
            break;
        }
        strTemp = strtok(NULL, ",()");
        cnt++;
    }

    // Mapper add tuples to user buffer
    while (t < tupleSize)
    {

        userIDTemp = mapTotal[t].userID;
        scoreTemp = mapTotal[t].score;
        topicTemp = mapTotal[t].topic;
        sprintf(oneTuple, "(%s,%s,%d)", userIDTemp, topicTemp, scoreTemp);

        for (int i = 0; i < userSize; i++)
            if (strcmp(userIDTemp, userMap[i]) == 0)
            {
                // printf("UserId %s Exist!\n", userIDTemp);
                userBufIndex = i;
                flag = 1;
                break;
            }

        if (flag == 0 && userSize < numOfUsers)
        {

            // printf("Add new User %s!\n", userIDTemp);
            userMap[userSize] = userIDTemp;
            userBufIndex = userSize;
            userSize = userSize + 1;
            new = 1;
        }

        if (flag == 1 || new == 1)
        {
            pthread_mutex_lock(&buf[userBufIndex].mtx);
            // printf("M got mtx\n");
            while (buf[userBufIndex].count == numOfSlots)
            {
                // printf("M got mtx but wait\n");
                pthread_cond_wait(&buf[userBufIndex].condF, &buf[userBufIndex].mtx);
            }

            tupleIndex = buf[userBufIndex].count;
            addToBuffer(userBufIndex, oneTuple, tupleIndex);

            // printf("M add tup = %s\n", oneTuple);
            pthread_cond_broadcast(&buf[userBufIndex].condE);
            pthread_mutex_unlock(&buf[userBufIndex].mtx);
        }
        flag = 0;
        new = 0;
        t++;
    }

    allDone = 1;

    for (int i = 0; i < numOfUsers; i++)
    {
        pthread_cond_broadcast(&buf[i].condE);
    }

    // // check data
    // for(int j = 0; j < numOfUsers; j++){
    //     for(int i = 0; i < numOfSlots; i ++){
    //         printf("buf%d tup%d = %s\n", j, i, buf[j].tuples[i]);
    //     }
    // }

    pthread_exit(NULL);
}

void *Reducer(void *threadid)
{

    long tid;
    tid = (long)threadid;

    struct outputTuple outTotal[numMaxTopic];

    char inTuple[30];
    int flag = 0;
    int size = 0;
    int index = 0;

    char *userID_temp;
    char *topic_temp;
    char *score_temp;

    void *res;

    while (1)
    {

        pthread_mutex_lock(&buf[tid].mtx);
        while (buf[tid].count == 0 && allDone == 0)
        {
            pthread_cond_wait(&buf[tid].condE, &buf[tid].mtx);
        }

        if ((allDone == 1) && (buf[tid].count == 0))
            break;

        // printf("P remove data mtx\n");
        index = 0;
        while (buf[tid].count > 0)
        {
            // printf("P%ld tup= %s\n", tid, buf[tid].tuples[index]);
            strcpy(inTuple, buf[tid].tuples[index++]);
            buf[tid].count = buf[tid].count - 1;

            userID_temp = strtok(inTuple, ",()");
            topic_temp = strtok(NULL, ",()");
            score_temp = strtok(NULL, ",()");

            if (size != 0)
                for (int i = 0; i < size; i++)
                    if ((strcmp(topic_temp, outTotal[i].topic) == 0))
                    {
                        // printf("Update score of %s\n", userID_temp);
                        outTotal[i].score += atoi(score_temp);
                        flag = 1;
                        break;
                    }

            if (flag == 0)
            {
                strcpy(outTotal[size].userID, userID_temp);
                strcpy(outTotal[size].topic, topic_temp);
                outTotal[size].score = atoi(score_temp);
                //printf("New data(%s,%s,%d)\n", outTotal[size].userID,outTotal[size].topic,outTotal[size].score);
                size += 1;
                // printf("Size = %d\n", size);
            }
            else
                flag = 0;
        }

        pthread_cond_broadcast(&buf[tid].condF);
        pthread_mutex_unlock(&buf[tid].mtx);
    }

    for (int j = 0; j < size; j++)
    {
        printf("(%s,%s,%d)\n", outTotal[j].userID, outTotal[j].topic, outTotal[j].score);
    }
    pthread_exit(NULL);
}

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
    numOfSlots = atoi(argv[1]);
    numOfUsers = atoi(argv[2]);
    pthread_t tidP;
    pthread_t tidR[numOfUsers];

    // init shared buffers for users
    init();

    // Create Mapper Thread
    pthread_create(&tidP, NULL, Mapper, NULL);

    // Create Reducer Thread
    for (long i = 0; i < numOfUsers; i++)
    {

        usleep(10000);
        pthread_create(&tidR[i], NULL, Reducer, (void *)i);
    }

    pthread_exit(NULL);
}
