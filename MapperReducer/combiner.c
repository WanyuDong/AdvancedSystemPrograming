// combiner.c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int
main(int argc, char *argv[])
{
    int pfd[2], pid1, pid2;                                     /* Pipe file descriptors */
    //printf("Welcome!\n");
    if (pipe(pfd) == -1){                            /* Create pipe */
        printf("pipe err");
        exit(1);
    }
    //printf("pip successful!\n");
//=============================================================================================
    switch (pid1 = fork()) {
    case -1:
        {
            printf("fork err");
            exit(1);
        }
    case 0:             /* First child: write to pipe */

        if (close(pfd[0]) == -1)                    /* Read end is unused */
        {
            printf("close 1 err");
            exit(1);
        }
        /* Duplicate stdout on write end of pipe; close duplicated descriptor */

        if (pfd[1] != STDOUT_FILENO) {              /* Defensive check */
            dup2(pfd[1], STDOUT_FILENO);
            close(pfd[1]);
        }

  		execlp("./mapper", "./mapper", argv[1], NULL);          /* Writes to pipe */
        exit(1);

    default:            /* Parent falls through to create next child */
        break;
    }
//==============================================================================================
    switch (pid2 = fork()) {
    case -1:
        {
            printf("fork err");
            exit(1);
        }

    case 0:             /* Second child: read from pipe */
        if (close(pfd[1]) == -1)                    /* Write end is unused */
        {
            printf("close 3 err");
            exit(1);
        }

        /* Duplicate stdin on read end of pipe; close duplicated descriptor */

        if (pfd[0] != STDIN_FILENO) {               /* Defensive check */
            if (dup2(pfd[0], STDIN_FILENO) == -1)
            {
                printf("dup2 2 err");
                exit(1);
            }
            if (close(pfd[0]) == -1)
            {
                printf("close 4 err");
                exit(1);
            }
        }
		execlp("./reducer", "./reducer", NULL);
        printf("execlp reducer err");
        exit(1);

    default: /* Parent falls through */
        break;
    }
//==============================================================================================
    /* Parent closes unused file descriptors for pipe, and waits for children */

    if (close(pfd[0]) == -1)
    {
        printf("close 5 err");
        exit(1);
    }
    if (close(pfd[1]) == -1)
    {
        printf("close 6 err");
        exit(1);
    }
    if (wait(NULL) == -1)
    {
        printf("close 7 err");
        exit(1);
    }
    if (wait(NULL) == -1)
    {
        printf("close 8 err");
        exit(1);
    }

    exit(EXIT_SUCCESS);
}