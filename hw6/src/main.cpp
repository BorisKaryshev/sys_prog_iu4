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

#include <iostream>
#include <unordered_map>

namespace po = boost::program_options;
namespace logging = boost::log;
namespace expr = boost::log::expressions;

const static std::unordered_map<std::string, operation_t_e> OPERATION_MAPPING = {
    {"add",    ADD_PASSWORD   },
    {"delete", REMOVE_PASSWORD},
    {"list",   LIST_PASSWORDS },
    {"find",   FIND_BY_KEY    }
};
const static std::unordered_map<std::string, container_to_use_t_e> CONTAINERS_MAPPING = {
    {"list",  LIST },
    {"stack", STACK}
};

std::pair<bool, po::variables_map> parse_args(int argc, char** argv) {
    po::options_description desc("This is HW5 for system programming course in BMSTU");

    // clang-format off
    desc.add_options()
        ("help,h", po::bool_switch()->default_value(false), "Show help")
        ("quiet,q", po::bool_switch()->default_value(false), "Disable logging")
        ("container,c", po::value<std::string>()->default_value("stack"), "Which container to use internaly (stack - default, list)")
        ("file_path,f", po::value<std::string>()->default_value(".passwords"), "Path to file in which passwords are stored");
    // clang-format on

    po::positional_options_description pos;
    pos.add("command", 1);
    pos.add("key", 1);
    pos.add("value", 1);

    po::variables_map map;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(pos).run(), map);

    bool is_not_valid = (map["help"].as<bool>()) || (!OPERATION_MAPPING.contains(map["command"].as<std::string>())) ||
                        (!CONTAINERS_MAPPING.contains(map["container"].as<std::string>())) ||
                        (OPERATION_MAPPING.at(map["command"].as<std::string>()) != LIST_PASSWORDS && !map.contains("key")) ||
                        (OPERATION_MAPPING.at(map["command"].as<std::string>()) == ADD_PASSWORD && !map.contains("value"));

    if (is_not_valid) {
        std::cout << "Usage: " << argv[0] << " [options] <command> [<key>] [<value>]\n\n";
        std::cout << desc << std::endl;
    }

    return std::make_pair(is_not_valid, map);
}

void init_logging(bool quiet_mode) {
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

    logging::core::get()->set_filter(logging::trivial::severity > logging::trivial::info);

    if (quiet_mode) {
        logging::core::get()->set_logging_enabled(false);
    }
}

int main(int argc, char** argv) {
    auto [is_args_valid, args] = parse_args(argc, argv);
    if (!is_args_valid) {
        return 1;
    }
    init_logging(args["quiet"].as<bool>());

    std::string key = args["key"].as<std::string>();
    std::string value = args["value"].as<std::string>();
    std::string file_path = args["file_path"].as<std::string>();

    pass_manager_options_t opts;
    opts.command = OPERATION_MAPPING.at(args["command"].as<std::string>());
    opts.container_to_use = CONTAINERS_MAPPING.at(args["container"].as<std::string>());
    opts.path_to_file = file_path.c_str();
    opts.key = (key.empty()) ? nullptr : key.c_str();
    opts.value = (value.empty()) ? nullptr : value.c_str();

    return_code_t_e res = run(opts);

    switch (res) {
        case OK:
            BOOST_LOG_TRIVIAL(info) << "Program finished successfully";
            break;
        case OTHER_ERROR:
            BOOST_LOG_TRIVIAL(error) << "Got unexpected error while executing programm";
            break;
    }

    return 0;
}
