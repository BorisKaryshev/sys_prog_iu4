#include <main.h>
#include <algorithm>
#include <boost/algorithm/string/classification.hpp>
#include <cctype>
#include "list.h"
#include "pair.h"
#include "stack.h"

#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>  // для TimeStamp
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/regex.hpp>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>

#include <iostream>
#include <string>
#include <unordered_map>

namespace po = boost::program_options;
namespace logging = boost::log;
namespace expr = boost::log::expressions;

const static std::unordered_map<std::string, operation_t_e> OPERATION_MAPPING = {
    {"add",  ADD_PASSWORD   },
    {"rm",   REMOVE_PASSWORD},
    {"list", LIST_PASSWORDS },
    {"find", FIND_BY_KEY    }
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
        ("container,c", po::value<std::string>()->default_value(std::string("stack")), "Which container to use internaly (stack - default, list)");
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

    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);

    if (quiet_mode) {
        logging::core::get()->set_logging_enabled(false);
    }
}

int run_stack() {
    BOOST_LOG_TRIVIAL(info) << "Running stack";
    custom_stack_t stack = create_stack();
    std::string line;
    while (std::getline(std::cin, line)) {
        for (uint64_t i = 0; i < line.size(); i++) {
            line[i] = std::tolower(line[i]);
        }
        if (OPERATION_MAPPING.find(line) == OPERATION_MAPPING.end()) {
            BOOST_LOG_TRIVIAL(error) << "Got unsupported command";
            continue;
        }
        auto cmd = OPERATION_MAPPING.at(line);
        switch (cmd) {
            case ADD_PASSWORD: {
                std::string key, value;
                std::getline(std::cin, line);
                std::vector<std::string> parts;
                boost::algorithm::split(parts, line, boost::algorithm::is_any_of(" "));
                key = parts[0];
                value = parts[1];

                pair_of_str_t pair = {strdup(key.c_str()), strdup(value.c_str())};
                push_stack(&stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                break;
            }
            case LIST_PASSWORDS: {
                custom_stack_t tmp_stack = create_stack();
                while (!is_stack_empty(&stack)) {
                    pair_of_str_t pair;
                    pop_stack(&stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                    std::cout << "{\"key\" : \"" << pair.first << "\", \"value\" : \"" << pair.second << "\"}" << std::endl;
                    push_stack(&tmp_stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                }
                while (!is_stack_empty(&tmp_stack)) {
                    pair_of_str_t pair;
                    pop_stack(&tmp_stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                    push_stack(&stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                }
                break;
            }
            case REMOVE_PASSWORD: {
                std::getline(std::cin, line);

                custom_stack_t tmp_stack = create_stack();

                while (!is_stack_empty(&stack)) {
                    pair_of_str_t pair;
                    pop_stack(&stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                    if (line == pair.first) {
                        std::cout << "Removed with key: " << pair.first << std::endl;
                    } else {
                        push_stack(&tmp_stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                    }
                }
                while (!is_stack_empty(&tmp_stack)) {
                    pair_of_str_t pair;
                    pop_stack(&tmp_stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                    push_stack(&stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                }
                break;
            }

            case FIND_BY_KEY: {
                std::getline(std::cin, line);

                custom_stack_t tmp_stack = create_stack();

                while (!is_stack_empty(&stack)) {
                    pair_of_str_t pair;
                    pop_stack(&stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                    if (line == pair.first) {
                        std::cout << "Contains value: " << pair.second << std::endl;
                    }
                    push_stack(&tmp_stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                }
                while (!is_stack_empty(&tmp_stack)) {
                    pair_of_str_t pair;
                    pop_stack(&tmp_stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                    push_stack(&stack, reinterpret_cast<char*>(&pair), sizeof(pair));
                }
                break;
            }
            case DEFAULT_OPERATION: {
                break;
            }
        }
    }
    return 0;
}

int run_list() {
    BOOST_LOG_TRIVIAL(info) << "Running list";
    custom_list_t list = create_list();
    std::string line;
    while (std::getline(std::cin, line)) {
        for (uint64_t i = 0; i < line.size(); i++) {
            line[i] = std::tolower(line[i]);
        }
        if (OPERATION_MAPPING.find(line) == OPERATION_MAPPING.end()) {
            BOOST_LOG_TRIVIAL(error) << "Got unsupported command";
            continue;
        }
        auto cmd = OPERATION_MAPPING.at(line);
        switch (cmd) {
            case ADD_PASSWORD: {
                std::string key, value;
                std::getline(std::cin, line);
                std::vector<std::string> parts;
                boost::algorithm::split(parts, line, boost::algorithm::is_any_of(" "));
                key = parts[0];
                value = parts[1];

                pair_of_str_t pair = {strdup(key.c_str()), strdup(value.c_str())};
                push_back_list(&list, reinterpret_cast<char*>(&pair), sizeof(pair));
                break;
            }
            case LIST_PASSWORDS: {
                for (custom_list_node_t* node = list.first_node; node != NULL; node = node->next_node) {
                    pair_of_str_t* pair = reinterpret_cast<pair_of_str_t*>(node->data);
                    std::cout << "{\"key\" : \"" << pair->first << "\", \"value\" : \"" << pair->second << "\"}" << std::endl;
                }
                break;
            }
            case REMOVE_PASSWORD: {
                std::getline(std::cin, line);
                std::vector<custom_list_node_t*> nodes;
                for (custom_list_node_t* node = list.first_node; node != NULL; node = node->next_node) {
                    pair_of_str_t* pair = reinterpret_cast<pair_of_str_t*>(node->data);
                    if (line == pair->first) {
                        if (list.first_node == node) {
                            list.first_node = node->next_node;
                        }
                        if (list.last_node == node) {
                            list.last_node = node->prev_node;
                        }
                        nodes.push_back(node);
                        std::cout << "Removed with key: " << pair->first << std::endl;
                    }
                }
                std::ranges::for_each(nodes, delete_node);
                break;
            }

            case FIND_BY_KEY: {
                std::getline(std::cin, line);

                for (custom_list_node_t* node = list.first_node; node != NULL; node = node->next_node) {
                    pair_of_str_t* pair = reinterpret_cast<pair_of_str_t*>(node->data);
                    if (line == pair->first) {
                        std::cout << "Contains value: " << pair->second << std::endl;
                    }
                }
                break;
            }
            case DEFAULT_OPERATION: {
                break;
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    auto [is_args_valid, args] = parse_args(argc, argv);
    if (is_args_valid) {
        return 1;
    }
    init_logging(args["quiet"].as<bool>());

    pass_manager_options_t opts;
    opts.container_to_use = CONTAINERS_MAPPING.at(args["container"].as<std::string>());

    switch (opts.container_to_use) {
        case LIST:
            run_list();
            break;
        case STACK:
            run_stack();
            break;
        default:
            run_stack();
            break;
    }
    return 0;
}
