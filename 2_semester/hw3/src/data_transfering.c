#include "data_transfering.h"
#include "crypto.h"
#include "main.h"
#include "string_util.h"

#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

static const uint32_t POLL_TIMEOUT_MS = 100;
static const uint32_t SOCKET_MAX_QUEUE_SIZE = 10;

typedef struct {
    int socket;
    int fd;
    int is_failed;
} created_socket_t;

int get_socket_type(options_t opts) {
    switch (opts.protocol) {
        case TCP:
            return SOCK_STREAM;
        case UDP:
            return SOCK_DGRAM;
        case ICMP:
            return SOCK_RAW;
        default:
            return 0;
    }
}

void close_sock(options_t opts, created_socket_t socket) {
    close(socket.fd);
    close(socket.socket);
    unlink(opts.socket_path);
    remove(opts.socket_path);
}

created_socket_t create_sock(options_t opts) {
    created_socket_t result = {.is_failed = 0};
    struct sockaddr_un addr;

    result.socket = socket(AF_UNIX, get_socket_type(opts), 0);
    if (result.socket == -1) {
        perror("Error occurred while opening input socket: sock creation failed");
        result.is_failed = 1;
        return result;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, opts.socket_path, sizeof(addr.sun_path) - 1);

    if (connect(result.socket, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        puts("[CLIENT] Socket successfully connected");
        result.fd = result.socket;
    } else {
        int res = bind(result.socket, (struct sockaddr*)&addr, sizeof(addr));
        if (opts.verbose) {
            printf("[VERBOSE] After binding socket got res: %d with errno: %d\n", res, errno);
        }
        if (res == -1 && errno == 98) {
            unlink(opts.socket_path);
            res = bind(result.socket, (struct sockaddr*)&addr, sizeof(addr));
            if (opts.verbose) {
                printf("[VERBOSE] After binding socket got res: %d with errno: %d\n", res, errno);
            }
        }
        listen(result.socket, SOCKET_MAX_QUEUE_SIZE);
        puts("[CLIENT] Waiting for peer to connect");
        if (opts.protocol == TCP) {
            result.fd = accept(result.socket, NULL, NULL);
        } else {
            result.fd = result.socket;
        }
    }

    return result;
}

return_code_t_e run_data_transfer(options_t opts) {
    crypto_options_t crypto_opts = crypto_options_create(opts.crypto_key_and_iv_path);
    created_socket_t sock = create_sock(opts);
    if (sock.is_failed) {
        return ERROR;
    }

    struct pollfd fds_for_read[] = {
        {.fd = sock.fd,      .events = POLLIN, .revents = 0},
        {.fd = STDIN_FILENO, .events = POLLIN, .revents = 0}
    };
    const int n_of_fds = sizeof(fds_for_read) / sizeof(fds_for_read[0]);

    char buff[STRING_T_MAX_SIZE];
    memset(buff, '\0', STRING_T_MAX_SIZE);

    puts("[CLIENT] Ready to transfer data");
    while (1) {
        int ret = poll(fds_for_read, n_of_fds, POLL_TIMEOUT_MS);
        if (ret == -1) {
            return OK;
        }
        if (ret == 0) {
            continue;
        }

        if (fds_for_read[0].revents & (POLLHUP | POLLERR)) {
            close_sock(opts, sock);
            return OK;
        }

        if (fds_for_read[0].revents & POLLIN) {
            fds_for_read[0].revents = 0;
            int bytes_read = read(fds_for_read[0].fd, buff, STRING_T_MAX_SIZE - 1);
            fds_for_read[0].revents = 0;
            int offset = 0;
            while (bytes_read > 0) {
                string_t message = string_decerealize(buff + offset);
                if (!opts.skip_crypto) {
                    string_t decrypted_message = decrypt_data(crypto_opts, (uint8_t*)message.data, message.size);
                    string_free(&message);
                    message = decrypted_message;
                }
                printf("[PEER] %s", message.data);
                offset += string_required_buffer_size(message);
                bytes_read -= string_required_buffer_size(message);

                string_free(&message);
                memset(buff, '\0', STRING_T_MAX_SIZE);
            }
        }

        if (fds_for_read[1].revents & POLLIN) {
            fds_for_read[1].revents = 0;
            fflush(stdin);
            read(fds_for_read[1].fd, buff, STRING_T_MAX_SIZE - 1);
            fds_for_read[1].revents = 0;
            string_t message = string_create(buff);
            if (!opts.skip_crypto) {
                string_t encrypted_message = encrypt_data(crypto_opts, (uint8_t*)message.data, message.size);
                string_free(&message);
                message = encrypted_message;
            }
            memset(buff, '\0', STRING_T_MAX_SIZE);
            string_serialize_to_buffer(message, buff);
            if (opts.verbose) {
                puts("[VERBOSE] Printing hexdump of transfered buffer");
                hexdump((uint8_t*)buff, string_required_buffer_size(message));
            }
            write(sock.fd, buff, string_required_buffer_size(message));
            string_free(&message);
            memset(buff, '\0', STRING_T_MAX_SIZE);
        }
    }
    crypto_options_free(&crypto_opts);
    return OK;
}
