#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define CHILD_COUNT 3
#define TIMEOUT_SECONDS 5

int main(void)
{
    pid_t child_pids[CHILD_COUNT];
    time_t start_times[CHILD_COUNT];
    int finished[CHILD_COUNT] = {0};
    int remaining = CHILD_COUNT;

    /*
     * Disable output buffering so messages from the parent
     * and child processes appear immediately.
     */
    setbuf(stdout, NULL);

    printf("Parent process started. PID: %d\n", getpid());

    for (int i = 0; i < CHILD_COUNT; i++)
    {
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork failed");
            return EXIT_FAILURE;
        }

        if (pid == 0)
        {
            printf("Child %d started. PID: %d\n", i + 1, getpid());

            if (i == 2)
            {
                printf("Child %d is simulating an unresponsive process.\n",
                       i + 1);

                sleep(20);
            }
            else
            {
                sleep(i + 2);

                printf("Child %d completed normally.\n", i + 1);
            }

            exit(EXIT_SUCCESS);
        }

        child_pids[i] = pid;
        start_times[i] = time(NULL);
    }

    while (remaining > 0)
    {
        for (int i = 0; i < CHILD_COUNT; i++)
        {
            if (finished[i])
            {
                continue;
            }

            int status;
            pid_t result = waitpid(child_pids[i], &status, WNOHANG);

            if (result > 0)
            {
                finished[i] = 1;
                remaining--;

                if (WIFEXITED(status))
                {
                    printf(
                        "Parent collected child PID %d with exit status %d.\n",
                        child_pids[i],
                        WEXITSTATUS(status)
                    );
                }
                else if (WIFSIGNALED(status))
                {
                    printf(
                        "Child PID %d was terminated by signal %d.\n",
                        child_pids[i],
                        WTERMSIG(status)
                    );
                }
            }
            else if (result == 0)
            {
                time_t elapsed =
                    time(NULL) - start_times[i];

                if (elapsed >= TIMEOUT_SECONDS)
                {
                    printf(
                        "Child PID %d exceeded the %d-second timeout.\n",
                        child_pids[i],
                        TIMEOUT_SECONDS
                    );

                    printf(
                        "Sending SIGTERM to child PID %d.\n",
                        child_pids[i]
                    );

                    if (kill(child_pids[i], SIGTERM) == -1)
                    {
                        perror("kill SIGTERM failed");
                    }

                    sleep(1);

                    result = waitpid(
                        child_pids[i],
                        &status,
                        WNOHANG
                    );

                    if (result == 0)
                    {
                        printf(
                            "Child PID %d did not stop. Sending SIGKILL.\n",
                            child_pids[i]
                        );

                        if (kill(child_pids[i], SIGKILL) == -1)
                        {
                            perror("kill SIGKILL failed");
                        }

                        waitpid(child_pids[i], &status, 0);
                    }

                    finished[i] = 1;
                    remaining--;

                    printf(
                        "Unresponsive child PID %d was terminated and collected.\n",
                        child_pids[i]
                    );
                }
            }
            else
            {
                if (errno != EINTR)
                {
                    perror("waitpid failed");
                    finished[i] = 1;
                    remaining--;
                }
            }
        }

        sleep(1);
    }

    printf("All child processes have been collected.\n");
    printf("No zombie processes remain.\n");

    return EXIT_SUCCESS;
}

