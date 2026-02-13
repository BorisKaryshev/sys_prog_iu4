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
        ("timeout,t", po::value<std::uint32_t>()->default_value(0), "Timeout.")
        ("input_path,i", po::value<std::string>(), "Path for input pipe")
        ("output_path,o", po::value<std::string>(), "Path for output pipe")
        ;
    // clang-format on

    po::positional_options_description pos;
    pos.add("path", 1);

    po::variables_map map;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), map);

    bool is_valid = (map["help"].as<bool>() || (!(map.contains("input_path") == map.contains("output_path"))));

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

    std::filesystem::path input_path, output_path;
    options_t opts = {.input_pipe_path = NULL, .output_pipe_path = NULL, .timeout_s = args["timeout"].as<uint32_t>()};
    if (args.contains("input_path") && args.contains("output_path")) {
        input_path = std::filesystem::absolute(args["input_path"].as<std::string>());
        opts.input_pipe_path = input_path.c_str();

        output_path = std::filesystem::absolute(args["output_path"].as<std::string>());
        opts.output_pipe_path = output_path.c_str();
    }

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
