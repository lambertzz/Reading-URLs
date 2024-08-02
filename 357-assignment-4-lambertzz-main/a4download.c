#include <sys/types.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int downloads = 0;
char line[256];
int line_number = 0;


void download(int line_number, int timeout, char* output_file, char* url) {
    
    pid_t pid = fork();

    if (pid == 0) {
        
        char timeout_str[10];
        sprintf(timeout_str, "%d", timeout);
        execlp("curl", "curl", "-m", timeout_str, "-o", output_file, "-s", url, (char*)NULL);
        printf("failed");
        exit(1);

    } else if (pid > 0) {
        
        printf("process %d processing line %d\n", pid, line_number);

    } else {
        printf("failed");
        exit(1);
    }
}

void downloading(const char* filename, int max) {

    FILE* file = fopen(filename, "r");

    if (!file) {
        printf("can't open");
        exit(1);
    }

    while (fgets(line, sizeof(line), file)) {

        line_number++;
        char output_file[256];

        char url[256];
        int timeout = 0;

        int input = sscanf(line, "%s %s %d", output_file, url, &timeout);

        if (input < 2) {
            printf("invalid");
            continue;
        }
        if (input == 2) {
            timeout = 0;
        }

        if (downloads >= max) {

            int status;
            pid_t pid = wait(&status);

            if (pid > 0) {
                downloads--;
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);

                    if (exit_status == 0) {
                        printf("process %d exited normally\n", pid);
                    } else {
                        printf("process %d terminated with exit status: %d\n", pid, exit_status);
                    }

                } else {
                    printf("process %d terminated abnormally\n", pid);
                }
            }
        }
        
        download(line_number, timeout, output_file, url);

        downloads++;
    }

    while (downloads > 0) {

        int status;

        pid_t pid = wait(&status);

        if (pid > 0) {

            downloads--;

            if (WIFEXITED(status)) {
                printf("process %d exited normally\n", pid);
            } else {
                printf("process %d terminated abnormally\n", pid);
            }
        }
    }

    fclose(file);
}

int main(int argc, char* argv[]) {

    const char* filename = argv[1];

    int max = atoi(argv[2]);

    if (argc != 3) {
        printf("error");
        exit(1);
    }

    if (max <= 0) {
        printf("not a valid number");
        exit(1);
    }

    downloading(filename, max);

    return 0;
}
