#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

#define MAX 256
#define WHITESPACE " \t\n\v\f\r"

void tokenize(char *input, char **words2, char *tokens);
void testaccess(char *cmd, char *arg);
void writerror(void);
void executeprocess(char *process);
char **path = (char *[MAX]) {"/bin", NULL};

int main(int argc, char *argv[]) {

    if (argc > 2) {
        writerror();
        exit(1);
    }
    FILE *f;

    if (argc == 2) {
        f = fopen(argv[1], "r");
        if (f == NULL) {
            writerror();
            exit(1);
        }
    } else
    {
        f = stdin;
    }


    while (1) {

        if (argc == 1) {
            printf("hesh> ");
        }
        char *line = NULL;
        size_t len = 0;

        int chars = getline(&line, &len, f);
        if (feof(f) != 0) {
            fclose(f);
            exit(0);
            break;
        }
        if (chars < 0) {continue;}
        line[chars-1] = '\0';
        if(line[0] == '\0') continue;
        char **processes = malloc(sizeof line);
        tokenize(line, processes, "&");

        for (int i = 0; processes[i] != NULL; i++) {
            executeprocess(processes[i]);
        }

        while (wait(NULL) > 0);
        //free(myargv);
        //free(cmd);
    }

    return 0;
}

void tokenize(char *input, char **words2, char *tokens) {
    char *target = strdup(input);
    int sindex = 0;
    char **words = malloc( sizeof(void *)*MAX/2 );
    while ( (*(words+sindex) = strsep(&target, tokens)) != NULL) {
        sindex++;
    }
    int j = 0;
    for (int i = 0; words[i]!=NULL; i++) {
        if (words[i][0] == '\0') continue;
        words2[j] = strdup(words[i]);
        j++;
    }
    words2[j] = NULL;


    free(words);
    return;
}

void testaccess(char *cmd, char *arg) {
    for(int i = 0; path[i] != NULL; i++) {
        char *buffer = malloc(MAX);
        snprintf(buffer, MAX, "%s/%s", path[i], arg);
        if (access(buffer, X_OK) == 0) {
            sprintf(cmd, "%s", buffer);
            free(buffer);
            return;
        }
        free(buffer);
    }
    cmd = NULL;
    return;
}

void writerror(void) {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void executeprocess(char *process) {
    char *order = strdup(process);
            char *pargs = strsep(&order, ">");
            if (*pargs == '\0') {
                if (*order == '\0') return;
                writerror();
                exit(0);
            }

            char **myargv = malloc(sizeof pargs);
            tokenize(pargs, myargv, WHITESPACE);
            if (myargv[0] == NULL) return;
            if (strcmp("exit", myargv[0]) == 0) {
                if (myargv[1] != NULL) {
                    writerror();
                    //free(myargv);
                    return;
                }
                //free(myargv);
                exit(0);
            }

            if(strcmp("cd", myargv[0]) == 0) {
                if(myargv[1] == NULL || myargv[2] != NULL) {
                    writerror();
                    //free(myargv);
                    return;
                }
                chdir(myargv[1]);
                //free(myargv);
                return;
            }

            if (strcmp("path", myargv[0]) == 0) {
                path = NULL;
                path = malloc(2 * sizeof order);
                tokenize(process, path, WHITESPACE); // Causes segfault if use order instead of process
                if (myargv[1] == NULL) path = NULL;
                //free(myargv);
                return;
            }
            char *cmd = malloc(MAX);
            int rc = fork();
            if (rc == 0) { // Child
                if (path == NULL) {
                    writerror();
                }
                if (order != NULL) {
                    char **outfilename = malloc(2 * sizeof order);
                    tokenize(order, outfilename, WHITESPACE);
                    if (outfilename[1] != NULL || outfilename[0] == NULL) {
                        writerror();
                        //free(myargv);
                        exit(0);
                    }

                    close(STDOUT_FILENO);
                    int x = open(outfilename[0], O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                    if (x < 0) {
                        writerror();
                        //free(myargv);
                        exit(0);
                    }
                } 
                testaccess(cmd, myargv[0]);
                execv(cmd, myargv);
                //perror("execv");
                //free(myargv);
                writerror();
            }
}