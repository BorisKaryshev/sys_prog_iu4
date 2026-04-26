#include "string_util.h"
#include "unnamed_pipe.h"

#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <unistd.h>

static const uint8_t READING_FROM_PIPE = 0;
static const uint8_t WRITING_TO_PIPE = 1;
static const uint32_t POLL_TIMEOUT_MS = 100;

pid_t CHILD_PID;

void handler(int signum) {
    signum *= signum;
    kill(CHILD_PID, SIGTERM);
    exit(0);
}

return_code_t_e child_process(const int* input_pipe_fd, const int* output_pipe_fd) {
    close(input_pipe_fd[WRITING_TO_PIPE]);
    close(output_pipe_fd[READING_FROM_PIPE]);
    char buff[STRING_T_MAX_SIZE];
    memset(buff, '\0', STRING_T_MAX_SIZE);

    struct pollfd fd_for_reading[] = {
        {.fd = input_pipe_fd[READING_FROM_PIPE], .events = POLLIN, .revents = 0}
    };
    int n_of_fds = sizeof(fd_for_reading) / sizeof(fd_for_reading[0]);

    while (1) {
        int ret = poll(fd_for_reading, n_of_fds, POLL_TIMEOUT_MS);
        if (ret == 0) {
            continue;
        }
        if (ret == -1) {
            return OK;
        }

        if (!(fd_for_reading[0].revents & POLLIN)) {
            continue;
        }
        if (read(fd_for_reading[0].fd, buff, STRING_T_MAX_SIZE) == 0) {
            continue;
        };

        string_t message = string_decerealize(buff);
        memset(buff, '\0', STRING_T_MAX_SIZE);

        printf("[CHILD] got from parent message: %s", message.data);
        string_serialize_to_buffer(message, buff);
        int return_code = (int)write(output_pipe_fd[WRITING_TO_PIPE], buff, string_required_buffer_size(message));
        if (return_code == -1) {
            printf("[PARENT] FAILED TO WRITE with errno = %d\n", errno);
        }
        memset(buff, '\0', STRING_T_MAX_SIZE);
        string_free(&message);
    }
    return OK;
}

return_code_t_e parent_process(const int* input_pipe_fd, const int* output_pipe_fd) {
    close(input_pipe_fd[WRITING_TO_PIPE]);
    close(output_pipe_fd[READING_FROM_PIPE]);
    signal(SIGTERM, handler);
    signal(SIGINT, handler);

    char buff[STRING_T_MAX_SIZE];
    memset(buff, '\0', STRING_T_MAX_SIZE);

    struct pollfd fds_for_read[] = {
        {.fd = STDIN_FILENO,                     .events = POLLIN, .revents = 0},
        {.fd = input_pipe_fd[READING_FROM_PIPE], .events = POLLIN, .revents = 0}
    };
    int n_of_fds = sizeof(fds_for_read) / sizeof(fds_for_read[0]);

    puts("[PARENT] Waiting for user input");
    while (1) {
        int ret = poll(fds_for_read, n_of_fds, POLL_TIMEOUT_MS);
        if (ret == 0) {
            continue;
        }
        if (ret == -1) {
            return OK;
        }

        int bytes_read;
        if ((fds_for_read[0].revents & POLLIN) && ((bytes_read = read(fds_for_read[0].fd, buff, STRING_T_MAX_SIZE - 1)) != 0)) {
            fds_for_read[0].revents = 0;
            fflush(stdout);
            if (bytes_read == (int)STRING_T_MAX_SIZE) {
                puts("[PARENT] got more than buffer size in message. This is not supported so fail. :)");
                return ERROR;
            }
            string_t message = string_create(buff);
            memset(buff, '\0', STRING_T_MAX_SIZE);

            string_serialize_to_buffer(message, buff);
            int return_code = (int)write(output_pipe_fd[WRITING_TO_PIPE], buff, string_required_buffer_size(message));
            if (return_code == -1) {
                printf("[PARENT] FAILED TO WRITE with errno = %d\n", errno);
            }
            memset(buff, '\0', STRING_T_MAX_SIZE);
            string_free(&message);
        }

        if ((fds_for_read[1].revents & POLLIN) && ((bytes_read = read(fds_for_read[1].fd, buff, STRING_T_MAX_SIZE - 1)) != 0)) {
            fds_for_read[1].revents = 0;
            if (bytes_read == (int)STRING_T_MAX_SIZE) {
                puts("[PARENT] got more than buffer size in message. This is not supported so fail. :)");
                return ERROR;
            }
            int offset = 0;

            while (bytes_read > 0) {
                string_t message = string_decerealize(buff + offset);

                printf("[PARENT] got from child: %s", message.data);

                offset += string_required_buffer_size(message);
                bytes_read -= string_required_buffer_size(message);

                memset(buff, '\0', STRING_T_MAX_SIZE);
                string_free(&message);
            }
        }
    }
    return OK;
}

return_code_t_e run_unnamed_pipes(void) {
    int parent_write_fd[2];
    int child_write_fd[2];

    if (pipe(parent_write_fd) == -1) {
        return ERROR;
    };
    if (pipe(child_write_fd) == -1) {
        return ERROR;
    };
    {
        int flags = fcntl(parent_write_fd[READING_FROM_PIPE], F_GETFL, 0);
        fcntl(parent_write_fd[READING_FROM_PIPE], F_SETFL, flags | O_NONBLOCK);
    }
    {
        int flags = fcntl(child_write_fd[READING_FROM_PIPE], F_GETFL, 0);
        fcntl(child_write_fd[READING_FROM_PIPE], F_SETFL, flags | O_NONBLOCK);
    }

    CHILD_PID = fork();
    if (CHILD_PID < 0) {
        return ERROR;
    } else if (CHILD_PID == 0) {
        child_process(parent_write_fd, child_write_fd);
        exit(0);
    } else {
        return parent_process(child_write_fd, parent_write_fd);
    }

    return OK;
}
