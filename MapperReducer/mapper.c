//test.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef struct { char action; int score; } Rules;
Rules rules[5] = {{'P',50}, {'L',20}, {'D',-10}, {'C',30}, {'S',40}};

int findScore(Rules* rules, char action){
  int i;
  for(i=0; i<5; i++){
    if(rules[i].action == action)
      return rules[i].score;
  }
  fprintf(stderr, "No such action!\n");
  exit(EXIT_FAILURE);

}


int mapAction(char action){
  switch (action)
  {
  case'P':
      return 50;
      break;
  case'L':
      return 20;
      break;
  case'D':
      return -10;
      break;
  case'C':
      return 30;
      break;
  case'S':
      return 40;
      break;
  default:
      fprintf(stderr, "No such action!\n");
      exit(EXIT_FAILURE);
      break;
  }
}

int main(int argc, char *argv[]){
    
    FILE *fptr;
    fptr = fopen("input.txt", "r");
    // char ch;
    // int status = 0;
    // int cnt1 = 0;
    // int score = 0;

    if( fptr == NULL){
        printf("OPEN_FILE_ERROR");
        exit(1);
    }

// -----------------------------------------------
    char * str_temp;
    char * userID_temp;
    char * topic_temp;
    int score_temp;
    char s;
    char * oneTuple;

    char * dataIn;
    ssize_t readDataIn;
    size_t len = 0;
    int cnt = 0;

    readDataIn = getline(&dataIn, &len, fptr);
    printf("Strat!\n");
    printf("Input Data: %s\n", dataIn);

    str_temp = strtok(dataIn, ",()");
    while(str_temp != NULL){
        switch (cnt % 3)
        {
        case 0:
			userID_temp = str_temp;
            break;
        case 1:
			s = *str_temp;
            score_temp = mapAction(s);
            break;
        case 2:
			topic_temp = str_temp;
            sprintf(oneTuple, "(%s,%s,%d)", userID_temp, topic_temp, score_temp);
            printf("%s\n", oneTuple);
            break;
        
        default:
            break;
        }
        str_temp = strtok(NULL, ",()");
        cnt = cnt + 1;
    }

    // printf("(%s,%s,%s)\n", userID_temp, score_temp, topic_temp);
    

// -----------------------------------------------

    // ch = fgetc(fptr);
    // while (ch != EOF){

    //     switch (status)
    //     {
    //     case 0:
    //         if( ch == '('){
    //             printf("%c", ch);
    //             status = 1;
    //             cnt1 = 0;
    //         }
    //         else
    //         {
    //             status = 0;
    //         }
            
    //         break;
    //     case 1:
    //         if(cnt1 >= 5){
    //             score = findScore(rules, ch);
    //             cnt1 = 0;
    //             status = 2;
    //         }
    //         else
    //         {
    //             printf("%c", ch);
    //             cnt1 = cnt1 + 1;
    //             status = 1;
    //         }
            
    //         break;
    //     case 2:
    //         // doing nothing to skip ','
    //         status = 3;            
    //         break;
    //     case 3:
    //         if( ch == ')'){
    //             printf(",%d)\n", score);
    //             score = 0;
    //             status = 0;
    //         }
    //         else
    //         {
    //             printf("%c", ch);
    //             status = 3;
    //         }
            
    //         break;
    //     default:
    //         break;
    //     }

    //     ch = fgetc(fptr);
        
    // }

    fclose(fptr);

    return 0;
}