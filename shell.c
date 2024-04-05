#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <wait.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_LINE 80
#define MAX_HISTORY 10

typedef struct
{
    int num;
    char com[MAX_LINE];
} command;

char **split(char *buffer, int *index)
{
    char temp[MAX_LINE];
    strcpy(temp, buffer);
    char **args = malloc((MAX_LINE / 2 + 1) * sizeof(char *));
    char *token = strtok(temp, " \n");
    *index = 0;
    while (token != NULL)
    {
        args[*index] = token;
        (*index)++;
        token = strtok(NULL, " \n");
    }
    args[*index] = NULL;
    return args;
}

void child_handler(int sig)
{
    int status;
    printf("osh>");
    fflush(stdout);
}

int main(void)
{
    signal(SIGCHLD, child_handler);
    int should_run = 1;
    command history[MAX_HISTORY];
    memset(history, 0, sizeof(history));
    int count = 0;
    while (should_run)
    {
        if (count == 0)
            printf("osh>");
        fflush(stdout);
        char buffer[MAX_LINE];
        int index;
        fgets(buffer, MAX_LINE, stdin);
        char **args = split(buffer, &index);
        if (strcmp(args[0], "history") != 0)
        {
            if (count < MAX_HISTORY)
            {
                history[count].num = count + 1;
                strcpy(history[count].com, buffer);
                count++;
            }
            else
            {
                for (int i = 0; i < MAX_HISTORY - 1; i++)
                    history[i] = history[i + 1];
                count++;
                history[MAX_HISTORY - 1].num = count;
                strcpy(history[MAX_HISTORY - 1].com, buffer);
            }
            int num;
            if (strcmp(args[0], "!!") == 0)
                strcpy(history[count > 10 ? MAX_HISTORY - 1 : count - 1].com, history[count > 10 ? MAX_HISTORY - 2 : count - 2].com);
            else if (sscanf(args[0], "!%d", &num) == 1)
                strcpy(history[count > 10 ? MAX_HISTORY - 1 : count - 1].com, history[count > 10 ? (num - count % 10 - 1) : num - 1].com);
        }
        int flag = 0;
        if (strcmp(args[index - 1], "&") == 0)
        {
            flag = 1;
            args[index - 1] = NULL;
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            should_run = 0;
            break;
        }
        pid_t pid = fork();
        if (pid == 0)
        {
            if (strcmp(args[0], "history") == 0)
            {
                for (int i = 0; i < (count > 10 ? 10 : count); i++)
                    printf("%d: %s\n", history[i].num, history[i].com);
            }
            else if (args[0][0] == '!')
            {
                int num, tempIndex;
                if (args[0][1] == '!' && count > 0)
                {
                    char **tempargs = split(history[count > 10 ? MAX_HISTORY - 2 : count - 2].com, &tempIndex);
                    if (strcmp(tempargs[tempIndex - 1], "&") == 0)
                    {
                        flag = 1;
                        tempargs[tempIndex - 1] = NULL;
                    }
                    execvp(tempargs[0], tempargs);
                }
                else if (args[0][1] == '!' && count == 1)
                    printf("No commands in history.\n");
                else if (sscanf(args[0], "!%d", &num) == 1)
                {
                    if ((count > 10 && (num > count || num < count - 9)) || (count <= 10 && num > count))
                        printf("No such command in history.\n");
                    else
                    {
                        char **tempargs = split(history[count > 10 ? (num - count % 10 - 1) : (num - 1)].com, &tempIndex);
                        if (strcmp(tempargs[tempIndex - 1], "&") == 0)
                        {
                            flag = 1;
                            tempargs[tempIndex - 1] = NULL;
                        }
                        execvp(tempargs[0], tempargs);
                    }
                }
            }
            else
                execvp(args[0], args);
            exit(0);
        }
        else if (pid > 0)
        {
            if (flag == 0)
                wait(NULL);
        }
    }
    return 0;
}