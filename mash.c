#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define MAX_COMMAND 255
#define MAX_ARGUMENTS 255
#define TEMP_ONE "./file_one-XXXXXX"
#define TEMP_TWO "./file_two-XXXXXX"
#define TEMP_THREE "./file_three-XXXXXX"

void split_string(char *, char **);
void initialize_args(char **, int);
void start_processes(char *, char *, char*, char **, char **, char **, char *);
int get_input_size(char *);
void execute_process(char**, int);
void print_contents_of_file(char *);
void create_temp_files(char *, char *, char *);
void display_results(char *, char *, char *, char *, char *, char *, int, int, int);


int main(void) {
    char cmd_one[MAX_COMMAND], cmd_two[MAX_COMMAND], cmd_three[MAX_COMMAND], file[MAX_COMMAND];
    printf("mash-1>");
    scanf("%[^\n]s", cmd_one);
    printf("mash-2>");
    scanf("\n%[^\n]s", cmd_two);
    printf("mash-3>");
    scanf("\n%[^\n]s", cmd_three);
    printf("file>");
    scanf("\n%[^\n]s", file);

    int cmd_one_size = get_input_size(cmd_one), 
        cmd_two_size = get_input_size(cmd_two), 
        cmd_three_size = get_input_size(cmd_three);

    // Initialize arguments.
    char **arg_one, **arg_two, **arg_three;
    arg_one = malloc(sizeof(char*) * cmd_one_size);
    arg_two = malloc(sizeof(char*) * cmd_two_size);
    arg_three = malloc(sizeof(char*) * cmd_three_size);

    initialize_args(arg_one, cmd_one_size);
    initialize_args(arg_two, cmd_two_size);
    initialize_args(arg_three, cmd_three_size);

    // Have arguments NULL terminated
    arg_one[cmd_one_size] = NULL;
    arg_two[cmd_two_size] = NULL;
    arg_three[cmd_three_size] = NULL;


    split_string(cmd_one, arg_one);
    split_string(cmd_two, arg_two);
    split_string(cmd_three, arg_three);

    start_processes(cmd_one, cmd_two, cmd_three,
            arg_one, arg_two, arg_three, file);

    free(arg_one);
    free(arg_two);
    free(arg_three);

    return 0;
}

/*
 * Function: split_string
 * ----------------------
 *  A function that takes in a String with a command with arguments seperated
 *  by a space and stores each command and argument in an array of NULL terminated strings
 *
 *  args: String with a command and arguments seperated by spaces
 *  split: The array of strings to store results in.
 *
 *  returns: nothing
 */
void split_string(char *args, char **split) {
    int n=0,i=0,j=0;

    while (1) {
        if(args[i] != ' ') {
            split[n][j++]=args[i];
        } else{
            split[n][j++]='\0';
            n++;
            j=0;
        } if(args[i]=='\0') {
            break;
        }
        i++;
    }
}

/*
 * Function: initialize_args
 * -------------------------
 *  Dyanmically allocates memory for the list of character 
 *  arrays provided.
 *
 *  char **args: Array of NULL terminated character arrays 
 *  int size: Size of the array
 *  Returns: none
 */
void initialize_args(char **args, int size) {
    size_t i;

    for (i = 0; i < size; i++) {
        args[i] = malloc(sizeof(char *));
    }
}

/*
 * Function: get_input_size
 * -----------------------
 *  Calculates the total amount of arguments provided.
 *  Ex: "mkdir -s -a -t" = 4
 *
 *  char *input: NULL terminated array of character
 *
 *  returns: The amount of words in the string.
 */
int get_input_size(char *input) {
    int size = 0, index = 0;
    char c = input[index];

    while (c != '\0') {
        if (c == ' ') {
            size++;
        }
        c = input[++index];
    }
    return size + 1;
}

/*
 * Function: start_processes
 * -------------------------
 * Creates three nested child processes, each one to execute a program provided.
 * Waits for all child processes to finish before exiting the function.
 *
 * arg_one, arg_two, arg_three: Array of commands and arguments terminated by
 * file: File to output results.
 *
 * a NULL character. Ex: {"wc", "-l", "t", NULL}
 * returns: nothing
 */
void start_processes(char *cmd_one, char *cmd_two, char *cmd_three,
        char **arg_one, char **arg_two, char** arg_three, char *file) {

    // Create temp files to store results in.
    char file_one[32], file_two[32], file_three[32];
    create_temp_files(file_one, file_two, file_three);

    // Create some clocks to measure time elapse
    clock_t start, end;
    int cmd_one_time, cmd_two_time, cmd_three_time;

    int p1, p2, p3;
    p1 = fork();

    if (p1 < 0) {
        fprintf(stderr, "fork failed\n");
    } else {
        // Close standard output and store results in file
        //close(STDOUT_FILENO);
        if (p1 == 0) {
            start = clock();
            open(file_one, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            execvp(arg_one[0], arg_one);
        } else {
            // wait for process 1 to finish
            end = clock();
            cmd_one_time = ((double) (end - start));
            int return_status;
            waitpid(p1, &return_status, 0);
            fprintf(stdout, "First process finished...\n");
        }
        if (p1 > 0) {
            p2 = fork();
            if (p2 == 0) {
                start = clock();
                open(file_two, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                execvp(arg_two[0], arg_two);
            }
            else {
                // wait for process 2 to finish
                end = clock();
                cmd_two_time = ((double) (end - start));
                int return_status;
                waitpid(p1, &return_status, 0);
                fprintf(stdout, "Second process finished...\n");
            }
            if (p2 > 0) {
                p3 = fork();
                if (p3 == 0) {
                    start = clock();
                    open(file_three, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                    execvp(arg_three[0], arg_three);
                } else {
                    // wait for process 3 to finish
                    end = clock();
                    cmd_three_time = ((double) (end - start));
                    int return_status;
                    waitpid(p1, &return_status, 0);
                    fprintf(stdout, "Third process finished...\n");
                }
                if (p3 > 0) {
                    wait(NULL);
                }
            }
        }
    }
    wait(NULL);
    open(file, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
    display_results(cmd_one, cmd_two, cmd_three, 
            file_one, file_two, file_three,
            cmd_one_time,cmd_two_time,cmd_three_time);

    // Delete temp files.
    unlink(file_one);
    unlink(file_two);
    unlink(file_three);
}

/*
 * Function: display_results
 * -------------------------
 *  Displays the results of all three commands in order that the user entered them.
 *
 *  temp_one, temp_two, temp_three: The files with the results in them
 *
 *  returns: nothing
 *
 */
void display_results(char *cmd_one, char *cmd_two, char *cmd_three,
        char *temp_one, char *temp_two, char *temp_three,
        int t1, int t2, int t3) {

    printf("-----CMD 1: %s---------------------------------------------------------\n", cmd_one);
    print_contents_of_file(temp_one);
    printf("Result took: %dms\n", t1);
    printf("-----CMD 2: %s---------------------------------------------------------\n", cmd_two);
    print_contents_of_file(temp_two);
    printf("Result took: %dms\n", t2);
    printf("-----CMD 3: %s---------------------------------------------------------\n", cmd_three);
    print_contents_of_file(temp_three);
    printf("Result took: %dms\n", t3);
    puts("--------------------------------------------------------------------------------\n");
}


/**
 * Function: print_contents_of_file
 * -------------------------------
 *  Prints the content of a file
 *
 *  file: File with content to print out
 *  returns: nothing
 */
void print_contents_of_file(char *file) {
    FILE *f;
    char s;
    f=fopen(file,"r");
    while((s=fgetc(f))!=EOF) {
        printf("%c",s);
    }
    fclose(f);
}

/*
 * Function: create_temp_files
 * --------------------------
 *  Create three temp files used to store results of executing each command
 *
 *  file_one, file_two, file_three: Temp names given
 *  returns: nothing
 */
void create_temp_files(char *file_one, char *file_two, char *file_three) {
    memset(file_one,0,sizeof(file_one));
    memset(file_two,0,sizeof(file_two));
    memset(file_three,0,sizeof(file_three));
    strncpy(file_one,TEMP_ONE,strlen(TEMP_ONE));
    strncpy(file_two,TEMP_TWO,strlen(TEMP_TWO));
    strncpy(file_three,TEMP_THREE, strlen(TEMP_THREE));
    mkstemp(file_one);
    mkstemp(file_two);
    mkstemp(file_three);
}

