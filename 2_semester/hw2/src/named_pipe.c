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
    while ((output_fd = open(opts.output_pipe_path, O_WRONLY | O_NONBLOCK)) == -1) {
        usleep(10000);
    };

    puts("Peer connected. You can start type your messages.");

    struct pollfd fds_for_read[] = {
        {.fd = input_fd,     .events = POLLIN, .revents = 0},
        {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0}
    };

    char buff[STRING_T_MAX_SIZE];
    memset(buff, '\0', STRING_T_MAX_SIZE);
    while (1) {
        int ret = poll(fds_for_read, 2, opts.timeout_s);
        if (ret == -1) {
            return OK;
        }
        if (ret == 0) {
            usleep(10000);
            continue;
        }

        if (fds_for_read[0].revents & POLLIN) {
            read(fds_for_read[0].fd, buff, STRING_T_MAX_SIZE - 1);
            fds_for_read[0].revents = 0;
            string_t message = string_decerealize(buff);
            printf("%s", message.data);
            string_free(&message);
            memset(buff, '\0', STRING_T_MAX_SIZE);
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
