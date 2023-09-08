#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100

int execvpfun(char* command, char** CommandArray, int* number)
{
    int i = 0, j = 0, value = 0, status;
    char* path = "/bin/";
    char* token = NULL;
    char* argv[100];
    char progpath[BUFFER_SIZE];
    int flag = 0;                           // if flag == 0 we dont want to exit
    int waitf = 1;                           // if wait == 0 we dont want to wait


    token = strtok(command, " ,\n");
    if (token == NULL)                      //here we check if we have space or enter
    {
        *number = *(number)-1;
        free(CommandArray[*number]);
        CommandArray[*number] = NULL;
        waitf = 0;
        flag = 0;
        return flag;
    }
    while (token != NULL)                                   //fill the array
    {
        argv[i] = token;
        token = strtok(NULL, " ,\n");
        i++;
    }

    j = i - 1;
    argv[i] = NULL;                       //we have to have null at the end

    if (strcmp(argv[j], "&") == 0)          //we will not wait
    {
        argv[j] = NULL;
        waitf = 0;           //we will not wait
        flag = 0;           //we dont have to exit
    }


    strcpy(progpath, path);     //here we add the /bin/
    strcat(progpath, argv[0]);
    int pid = fork();

    if (pid < 0)        //invalid
    {
        flag = 1;
        waitf = 1;
        perror("error");
    }
    else if (pid == 0)  //child
    {
        execvp(progpath, argv);
        perror("error");
        waitf = 1;
        flag = 1;
    }
    else    //parent
    {
        if (waitf == 1)
        {
            waitpid(pid, &status, 0);
        }
    }
    return flag;//the exit flag
}

void historyfunc(char** CommandArray, int number)//just to print the history
{
    int i;
    for (i = number - 1; i > 0; i--)
    fprintf(stdout, "%d\t%s\n", i, CommandArray[i]);

    return;
}
int func1(char* command, int number, char** CommandArray)   //here we exctracte the number after the !
{
    int printnum = -1, i, j;
    if (command[1] == '!')
    {
        printnum = number - 2;
        CommandArray[number - 1] = CommandArray[printnum];
    }

    else
    {
        for (i = 1; '0' <= command[i] && command[i] <= '9'; i++);
        i = i - 1;
        if (i == 0)
            return printnum;//its not legal
        else
        {
            printnum = 0;
            for (j = 1; j <= i; j++)
                printnum = printnum * 10 + (command[j] - '0');
            if(0<printnum && printnum<number)
                CommandArray[number - 1] = CommandArray[printnum];
        }
    }
    return printnum;
}
int main(void)
{
    close(2);
    dup(1);
    char command[BUFFER_SIZE];
    int number = 1, i = 0, j = 0;
    int printnum = 0;
    char* CommandArray[BUFFER_SIZE] = {NULL};
    char* temp = NULL;
    char* temp1 = NULL;
    int flag = 0;
    while (1)
    {
        fprintf(stdout, "my-shell>");
        memset(command, 0, BUFFER_SIZE);
        fgets(command, BUFFER_SIZE, stdin);
        if (strncmp(command, "exit", 4) == 0)
        {
            for (i = 0; i < BUFFER_SIZE; i++)
            {
                if (CommandArray[i] != NULL)//free it at the end
                    free(CommandArray[i]);
            }
            break;
        }

        printnum = 0;

        for (i = 0; i < strlen(command); i++)
        {    //delete newline
            if (command[i] == '\n')
                command[i] = '\0';
        }

        temp = (char*)malloc(BUFFER_SIZE * sizeof(char));//new block
        for (i = 0; i <= strlen(command); i++)
            temp[i] = command[i];

        CommandArray[number] = temp;//add it to history
        number++;


        if (!memcmp(command, "history", 6)) //history
            historyfunc(CommandArray, number);
        else if (!memcmp(command, "!", 1)) // !
        {
            printnum = func1(command, number, CommandArray);

            //here we have !printnum
            if (printnum == -1 && number!=1)
            {
                temp1 = (char*)malloc(BUFFER_SIZE * sizeof(char));
                for (i = 0; i <= strlen(CommandArray[number - 1]); i++)
                    temp1[i] = CommandArray[number - 1][i];
                flag = execvpfun(temp1, CommandArray, &number);//its not valid for me
                free(temp1);
            }
            else if (0<printnum && printnum < (number - 1) )
            {
                temp1 = (char*)malloc(BUFFER_SIZE * sizeof(char));
                for (i = 0; i <= strlen(CommandArray[printnum]); i++)
                    temp1[i] = CommandArray[printnum][i];

                if (strcmp(CommandArray[printnum], "history") == 0) 
                    historyfunc(CommandArray, number);

                else flag = execvpfun(temp1, CommandArray, &number);
                free(temp1);
            }
            else
            {
                number--;
                free(CommandArray[number]);//we  want to remove it
                CommandArray[number] = NULL;
                if(printnum!=0 || number==1)
                fprintf(stdout, "No History\n");
            }

        }
        else
        {
            //normal command
            flag = execvpfun(command, CommandArray, &number);
        }

        if (flag == 1) break;
    }

    return 0;
}
