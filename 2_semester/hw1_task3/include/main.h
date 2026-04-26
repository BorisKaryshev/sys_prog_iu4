#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    TCP,
    UDP,
    ICMP
} required_socket_protocol_t_e;

typedef struct {
    const char* socket_path;
    const char* crypto_key_and_iv_path;
    required_socket_protocol_t_e protocol;
    int verbose;
    int skip_crypto;
} options_t;

typedef enum {
    OK,
    ERROR
} return_code_t_e;

return_code_t_e run(options_t);

#ifdef __cplusplus
}

#include <istream>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
inline std::istream& operator>>(std::istream& in, required_socket_protocol_t_e& protocol) {
    std::string token;
    in >> token;
    if (token == "tcp")
        protocol = TCP;
    else if (token == "udp")
        protocol = UDP;
    else if (token == "icmp")
        protocol = ICMP;
    else {
        throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value,
                                                       "required_socket_protocol_t_e", 0);
    }
    return in;
}

inline std::ostream& operator<<(std::ostream& out, const required_socket_protocol_t_e& protocol) {
    switch (protocol) {
        case TCP:
            out << "tcp";
            break;
        case UDP:
            out << "udp";
            break;
        case ICMP:
            out << "icmp";
            break;
    }

    return out;
}
#endif
