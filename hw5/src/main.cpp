#include <main.h>

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

namespace po = boost::program_options;
namespace logging = boost::log;
namespace expr = boost::log::expressions;

std::pair<bool, po::variables_map> parse_args(int argc, char** argv) {
    po::options_description desc("This is HW5 for system programming course in BMSTU");

    // clang-format off
    desc.add_options()
        ("help,h", po::bool_switch()->default_value(false), "Show help")
        ("create", po::bool_switch()->default_value(false), "Create file")
        ("delete", po::bool_switch()->default_value(false), "Delete file")
        ("path", po::value<std::string>(), "File path");
    // clang-format on

    po::positional_options_description pos;
    pos.add("path", 1);

    po::variables_map map;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), map);

    bool is_valid = (map["help"].as<bool>()) || (!map["create"].as<bool>() && !map["delete"].as<bool>());

    if (is_valid) {
        std::cout << "Usage: " << argv[0] << " [options] <path>\n\n";
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

    std::filesystem::path path = args["path"].as<std::string>();
    path = std::filesystem::absolute(path);

    operation_t_e operation = DEFAULT_OPERATION;
    if (args["create"].as<bool>()) {
        BOOST_LOG_TRIVIAL(info) << "Running create file command";
        operation = CREATE_FILE;
    } else if (args["delete"].as<bool>()) {
        BOOST_LOG_TRIVIAL(info) << "Running delete file command";
        operation = DELETE_FILE;
    }
    BOOST_LOG_TRIVIAL(info) << "Got path: " << path;
    return_code_t_e res = run(operation, path.c_str());

    switch (res) {
        case OK:
            BOOST_LOG_TRIVIAL(info) << "Operation completed successfully";
            break;
        case CREATE_FILE_EXISTS:
            BOOST_LOG_TRIVIAL(error) << "Failed to create file: already exists";
            break;
        case DELETE_NOT_FOUND:
            BOOST_LOG_TRIVIAL(error) << "Failed to delete file: not found";
            break;
        case UNEXPECTED_OPERATION:
            BOOST_LOG_TRIVIAL(error) << "Got unsupported operation";
            break;
        default:
            BOOST_LOG_TRIVIAL(error) << "Got unexpected error";
            break;
    }

    return 0;
}
