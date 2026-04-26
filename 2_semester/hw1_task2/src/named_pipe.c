#include "named_pipe.h"
#include "main.h"
#include "string_util.h"

#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/stat.h>
#include <unistd.h>

static const uint32_t POLL_TIMEOUT_MS = 100;

return_code_t_e run_named_pipe(options_t opts) {
    if (mkfifo(opts.input_pipe_path, 0666) == -1 && errno != EEXIST) {
        puts("Failed to create named pipe (fifo)");
        return ERROR;
    }
    if (mkfifo(opts.output_pipe_path, 0666) == -1 && errno != EEXIST) {
        puts("Failed to create named pipe (fifo)");
        return ERROR;
    }

    int input_fd = open(opts.input_pipe_path, O_RDONLY | O_NONBLOCK);

    int output_fd;
    int has_written_about_waiting = 0;
    while ((output_fd = open(opts.output_pipe_path, O_WRONLY | O_NONBLOCK)) == -1) {
        if (!has_written_about_waiting) {
            has_written_about_waiting = 1;
            puts("Waiting for peer to connect");
        }
        usleep(10000);
    };

    puts("Peer connected. You can start type your messages.");

    struct pollfd fds_for_read[] = {
        {.fd = input_fd,     .events = POLLIN, .revents = 0},
        {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0}
    };
    int n_of_fds = sizeof(fds_for_read) / sizeof(fds_for_read[0]);

    char buff[STRING_T_MAX_SIZE];
    memset(buff, '\0', STRING_T_MAX_SIZE);
    while (1) {
        int ret = poll(fds_for_read, n_of_fds, POLL_TIMEOUT_MS);
        if (ret == -1) {
            return OK;
        }
        if (ret == 0) {
            usleep(10000);
            continue;
        }

        if (fds_for_read[0].revents & POLLIN) {
            int bytes_read = read(fds_for_read[0].fd, buff, STRING_T_MAX_SIZE - 1);
            fds_for_read[0].revents = 0;
            int offset = 0;
            while (bytes_read > 0) {
                string_t message = string_decerealize(buff + offset);
                printf("%s", message.data);

                offset += string_required_buffer_size(message);
                bytes_read -= string_required_buffer_size(message);

                string_free(&message);
                memset(buff, '\0', STRING_T_MAX_SIZE);
            }
        }

        if (fds_for_read[1].revents & POLLIN) {
            fflush(stdin);
            read(fds_for_read[1].fd, buff, STRING_T_MAX_SIZE - 1);
            fds_for_read[1].revents = 0;
            string_t message = string_create(buff);
            memset(buff, '\0', STRING_T_MAX_SIZE);
            string_serialize_to_buffer(message, buff);
            write(output_fd, buff, string_required_buffer_size(message));
            string_free(&message);
            memset(buff, '\0', STRING_T_MAX_SIZE);
        }

        if (fds_for_read[0].revents & POLLHUP) {
            close(input_fd);
            close(output_fd);
            return OK;
        }
    }
    return OK;
}
