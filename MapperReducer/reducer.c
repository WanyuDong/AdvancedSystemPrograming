//reducer.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct outputTuple{
    char userID[20];
    char topic[20];
    int score;
};

int main(int argc, char *argv[]){
 
    int maxOutTotal = 60;
    struct outputTuple outTotal[maxOutTotal];

    char inTuple[30];
    int flag = 0;
    int size = 0;

    char * userID_temp;
    char * topic_temp;
    char * score_temp;

    FILE * dataSource;
    FILE * fptr;
    fptr = fopen(argv[1], "r");
    
    if( argc > 1){
        dataSource = fptr;// reducer get data from argv[1].
    }
    else
    {
        dataSource = stdin;// reducer get data from stdin(pipes).
    }
    

    while( fgets(inTuple, sizeof(inTuple), dataSource) != NULL){        

        userID_temp = strtok(inTuple, ",()");
        topic_temp = strtok(NULL, ",()");
        score_temp = strtok(NULL, ",()");

        if(size != 0){
            for (int i = 0; i < size; i++){  

                if((strcmp(userID_temp, outTotal[i].userID) == 0)&&(strcmp(topic_temp, outTotal[i].topic) == 0)){
                    //printf("Update score of %s\n", userID_temp);
                    outTotal[i].score += atoi(score_temp);
                    flag = 1;
                    break;
                }
            }
        }

        if (flag == 0){
            strcpy(outTotal[size].userID, userID_temp);
            strcpy(outTotal[size].topic, topic_temp);
            outTotal[size].score = atoi(score_temp);
            //printf("New data(%s,%s,%d)\n", outTotal[size].userID,outTotal[size].topic,outTotal[size].score);
            size += 1;
        }else
            flag = 0;
    }

    //printf("The size = %d\n", size);
    for (int j = 0; j < size; j++)
        printf("(%s,%s,%d)\n", outTotal[j].userID,outTotal[j].topic,outTotal[j].score);

    return 0;
}