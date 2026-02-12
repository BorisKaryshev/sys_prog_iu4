#include <main.h>
#include <cstdio>

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

const static std::unordered_map<std::string, sort_ordering_t_e> SORT_ORDERING_MAPPING = {
    {"asc",  ASCENDING },
    {"desc", DESCENDING}
};

std::pair<bool, po::variables_map> parse_args(int argc, char** argv) {
    po::options_description desc("This is HW5 for system programming course in BMSTU");

    // clang-format off
    desc.add_options()
        ("help,h", po::bool_switch()->default_value(false), "Show help")
        ("input_file,i", po::value<std::string>(), "Path to input file. Reads from stdin by default.")
        ("output_file,o", po::value<std::string>(), "Path to output file. Prints to stdin by default.")
        ("sort_ordering,s", po::value<std::string>()->default_value("desc"), "desc - default, asc");
    // clang-format on

    po::variables_map map;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), map);

    bool is_not_valid = map["help"].as<bool>();

    if (is_not_valid) {
        std::cout << "Usage: " << argv[0] << " [options] <command> [<key>] [<value>]\n\n";
        std::cout << desc << std::endl;
    }

    return std::make_pair(is_not_valid, map);
}
int main(int argc, char** argv) {
    auto [is_args_valid, args] = parse_args(argc, argv);
    if (is_args_valid) {
        return 1;
    }

    sort_ordering_t_e order = SORT_ORDERING_MAPPING.at(args["sort_ordering"].as<std::string>());
    FILE* input_file = stdin;
    FILE* output_file = stdout;

    if (args.contains("input_file")) {
        input_file = fopen(args["input_file"].as<std::string>().c_str(), "r");
    }
    if (args.contains("output_file")) {
        output_file = fopen(args["output_file"].as<std::string>().c_str(), "w");
    }

    return_code_t_e res = run(order, input_file, output_file);

    switch (res) {
        case OK:
            break;
        case OTHER_ERROR:
            BOOST_LOG_TRIVIAL(error) << "Got unexpected error while executing programm";
            break;
    }

    fclose(input_file);
    fclose(output_file);
    return 0;
}
