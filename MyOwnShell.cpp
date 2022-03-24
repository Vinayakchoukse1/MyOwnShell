#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>   // exit()
#include <unistd.h>   // fork(), getpid(), exec()
#include <sys/wait.h> // wait()
#include <signal.h>   // signal()
#include <fcntl.h>    // close(), open()
// #define LSH_RL_BUFSIZE 1024

// Function for seperating input on basis of delimiter "space"

char **get_input(char *input)
{
    // removing extra white spaces in the command

    char *end;
    // Trim leading space

    while (isspace((unsigned char)*input))
    {
        input++;
    }
    // All spaces

    if (*input == 0)
    {
    }

    else
    {
        end = input + strlen(input) - 1;
        while (end > input && isspace((unsigned char)*end))
            end--;

        // Write new null terminator character
        end[1] = '\0';
    }

    // seperating differnt string of commands on basis of "space"

    char **command = malloc(8 * sizeof(char *));
    char *token;
    int i = 0;
    /*  eg : input="mkdir Vinayak"
        sor here ,
        token =mkdir;
        command[0]=mkdir;
        token=Vinayak;
        command[1]=vinayak
    */

    // Splits input according to given delimiters.

    token = strtok(input, " ");
    while (token != NULL)
    {
        command[i] = token;
        i++;

        token = strtok(NULL, " ");
    }

    command[i] = NULL;
    return command;
}

// Function for Parse input with 'strsep()' for differXent symbols (&&, ##, >) and for spaces.

int parseInput(char **command)
{

    size_t ln = strlen(command[0]) - 1;
    if (*command[0] && command[0][ln] == '\n')
        command[0][ln] = '\0';

    // checking if the command given is "exit"
    if (strcmp(command[0], "exit") == 0)
    {

        free(command);
        return 1;
    }

    // Checking if there is parallel commands given as input

    int i = 0, dflag = 0;
    while (command[i])
    {
        if (strcmp(command[i], "&&") == 0)
        {
            dflag = 1;
            break;
        }
        i++;
    }

    if (dflag == 1)
    {
        return 2;
    }

    // Checking if there is sequential commands are given as input

    i = 0;
    dflag = 0;
    while (command[i])
    {
        if (strcmp(command[i], "##") == 0)
        {
            dflag = 1;
            break;
        }
        i++;
    }

    if (dflag == 1)
    {
        return 3;
    }

    // Checking if there is redirection of output of command

    i = 0;
    dflag = 0;

    while (command[i])
    {
        if (strcmp(command[i], ">") == 0)
        {
            dflag = 1;
            break;
        }
        i++;
    }

    if (dflag == 1)
    {
        return 4;
    }

    // This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
}

// Function for changing the current directory to given path

int cd(char *path)
{
    return chdir(path);
}

// Function for executing  the single command

void executeCommand(char **command)
{
    // This function will fork a new process to execute a command
    if (strcmp(command[0], "cd") == 0)
    {
        //Error handling
        if (cd(command[1]) < 0)
        {

            printf("Shell: Incorrect command\n");
        }
        /* Skips the fork, as child process is no longer required */
        return;
    }

    else
    {
        int rc = fork();
        if (rc == 0)
        {
            // Interrupt the process. When the user types the Ctrl + C the SIGINT signal is sent.
            signal(SIGINT, SIG_IGN);
            // The SIGTSTP signal is sent to a process by its controlling terminal to request it to stop. It is commonly initiated by the user pressing Ctrl + Z
            signal(SIGTSTP, SIG_IGN);
            // SIG_IGN specifies that the signal should be ignored.

            // the argv list first argument should point to
            // filename associated with file being executed
            // the array pointer must be terminated by NULL
            // pointer

            // first argument points to the file name associated with the file being executed.
            // second argument is a null terminated array of character pointers.x`
            if (execvp(command[0], command) == -1)
            {
                // the execv() only return if error occured.
                // The return value is -1
                printf("Shell: Incorrect command\n");
            }
            exit(0);
        }

        else if (rc > 0)
        {
            //A call to wait() blocks the calling process until one of its child processes exits or a signal is received
            wait(0);
        }

        else
        {
            printf("There's an error while forking\n");
        }
    }
}

// Function for executing parallel commands i.e seperated by "&&""

void executeParallelCommands(char **command)
{
    int iterator = 0;
    char *temp[100][100];
    int i = 0, j = 0, k = 0;
    pid_t pidChild = 1;
    int status;

    /* eg :- ./add && ./sub*/

    for (iterator = 0; command[i] != NULL; iterator++)
    {
        if (strcmp(command[i], "&&") != 0)
        {

            temp[j][k] = command[i];
            i++;
            k++;
        }

        else
        {
            temp[j][k] = NULL;
            i++;
            j++;
            k = 0;
        }
    }

    temp[j][k] = NULL;
    temp[j + 1][0] = NULL;

    for (i = 0; temp[i][0] != NULL && pidChild != 0; i++)
    {
        pidChild = fork();
        if (pidChild < 0) /* fork a child process           */
        {
            printf("Shell: Incorrect command\n");
            exit(1);
        }

        else if (pidChild == 0) /* for the child process:         */
        {
            signal(SIGINT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            // the argv list first argument should point to
            // filename associated with file being executed
            // the array pointer must be terminated by NULL
            // pointer

            execvp(temp[i][0], temp[i]);
        }
    }

    while (j >= 0)
    {
        //if we want to reap any specific child process, we use waitpid() function.
        waitpid(-1, &status, WUNTRACED); /* for the parent:      */
        j--;
    }
    // This function will run multiple commands in parallel
}

// Function for executing sequential commands i.e seperated by "##"

void executeSequentialCommands(char **command)
{

    int iterator = 0;
    int i = 0, j = 0, k = 0;
    char *temp[30];

    /*  eg : touch vinayak.txt ## ls
        temp[0]=touch;
        temp[2]=Vinayak;
        
        this will execute first 
        then again temp wil get emptied

        temp[0]=ls
        again this command will get executed


    */
    for (iterator = 0; command[i] != NULL; iterator++)
    {
        if (strcmp(command[i], "##") != 0)
        {
            temp[k] = command[i];
            i++;
            k++;
        }

        else
        {
            temp[k] = NULL;
            executeCommand(temp);
            i++;
            k = 0;
        }
    }

    temp[k] = NULL;
    executeCommand(temp);

    // This function will run multiple commands in sequence
}

// Function for executing command which gives its output to another file i.e " > "

void executeCommandRedirection(char **command)
{
    int count = 0;
    char **temp = command;
    while (*temp++)
        count++;

    int i = 0;
    char **tobeexecute = malloc(8 * sizeof(char *));
    char *file;
    /*  eg : wc myshell.c > vinayak.txt
        here the the input given before ">" will be the desired command 
        ie 
        tobeexecuted[0]="wc";
        tobeexecuted[1]="myshell.c";
        and the file name will be 
        file="vinayak.txt";
    */

    while (command[i] != NULL)
    {
        if (strcmp(command[i], ">") == 0)
        {
            file = command[i + 1];
            break;
        }
        tobeexecute[i] = command[i];
        i++;
    }

    int fp = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IXUSR);
    int output = dup(1); //The dup() system call creates a copy of a file descriptor.
    dup2(fp, 1);
    executeCommand(tobeexecute);
    dup2(output, 1);

    // This function will run a single command with output redirected to an output file specificed by user
}

int main()
{
    // Initial declarations

    // Interrupt the process. When the user types the Ctrl + C the SIGINT signal is sent.
    signal(SIGINT, SIG_IGN);
    // The SIGTSTP signal is sent to a process by its controlling terminal to request it to stop. It is commonly initiated by the user pressing Ctrl + Z
    signal(SIGTSTP, SIG_IGN);
    // SIG_IGN specifies that the signal should be ignored.

    while (1) // This loop will keep your shell running until user exits.
    {

        char **command;
        // Print the prompt in format - currentWorkingDirectory$

        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        printf("%s$", cwd);

        // accept input with 'getline()'

        size_t buffer_size = 80;
        char *input = malloc(buffer_size);
        getline(&input, &buffer_size, stdin);
        command = get_input(input);

        // Parse input with 'strsep()' for differXent symbols (&&, ##, >) and for spaces.

        int parse_val = parseInput(command);

        if (parse_val == 1)
        {
            printf("Exiting shell...\n"); // When user uses exit command.
            break;
        }

        if (parse_val == 2)
        {
            executeParallelCommands(command); // This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
        }

        else if (parse_val == 3)
        {
            executeSequentialCommands(command); // This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
        }

        else if (parse_val == 4)
        {
            executeCommandRedirection(command); // This function is invoked when user wants redirect output of a single command to and output file specificed by user
        }

        else
        {
            executeCommand(command); // This function is invoked when user wants to run a single commands
        }

        free(input);
        free(command);
    }

    return 0;
}
