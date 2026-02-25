#include <main.h>
#include <cstdint>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>  // для TimeStamp
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <filesystem>
#include <iostream>
#include <string>

namespace po = boost::program_options;
namespace logging = boost::log;
namespace expr = boost::log::expressions;

std::pair<bool, po::variables_map> parse_args(int argc, char** argv) {
    po::options_description desc("This is HW5 for system programming course in BMSTU");

    // clang-format off
    desc.add_options()
        ("help,h", po::bool_switch()->default_value(false), "Show help")
        ("verbose,v", po::bool_switch()->default_value(false), "Verbose output")
        ("skip_crypto,c", po::bool_switch()->default_value(false), "Should pass data as plain text")
        ("protocol,p", po::value<required_socket_protocol_t_e>()->default_value(TCP), "Protocol. One of: tcp (default), udp, icmp")
        ("crypto_key,k", po::value<std::string>()->default_value("crypto_key_and_iv.bin"))
        ("socket_path,s", po::value<std::string>()->default_value("test.sock"));
    // clang-format on

    po::variables_map map;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), map);

    bool is_valid = (map["help"].as<bool>());

    if (is_valid) {
        std::cout << "Usage: " << argv[0] << " [options]\n\n";
        std::cout << desc << std::endl;
        return std::make_pair(false, map);
    }

    return std::make_pair(true, map);
}

void init_logging() {
    logging::add_common_attributes();
    // clang-format off
    logging::add_console_log(
        std::clog,
        boost::log::keywords::format = (
            expr::stream
            << expr::attr<boost::posix_time::ptime>("TimeStamp")
            << " [" << logging::trivial::severity << "] "
            << expr::smessage
        )
    );
    // clang-format on

    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
}

int main(int argc, char** argv) {
    init_logging();
    auto [is_args_valid, args] = parse_args(argc, argv);
    if (!is_args_valid) {
        return 1;
    }

    std::filesystem::path socket_path = std::filesystem::absolute(args["socket_path"].as<std::string>());
    std::filesystem::path crypto_key_path = std::filesystem::absolute(args["crypto_key"].as<std::string>());
    options_t opts = {.socket_path = socket_path.c_str(),
                      .crypto_key_and_iv_path = crypto_key_path.c_str(),
                      .protocol = args["protocol"].as<required_socket_protocol_t_e>(),
                      .verbose = args["verbose"].as<bool>(),
                      .skip_crypto = args["skip_crypto"].as<bool>()};

    BOOST_LOG_TRIVIAL(info) << "Got protocol " << opts.protocol;

    return_code_t_e res = run(opts);

    switch (res) {
        case OK:
            BOOST_LOG_TRIVIAL(info) << "Operation completed successfully";
            break;
        case ERROR:
            BOOST_LOG_TRIVIAL(error) << "Programm failed";
            break;
        default:
            BOOST_LOG_TRIVIAL(error) << "Got unexpected error";
            break;
    }

    return 0;
}
