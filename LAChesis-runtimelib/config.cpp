
#include "config.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string_view>

#include <ryml.hpp>
#include <ryml_std.hpp>

struct config
{
    std::string analysis;
    std::map<memory_operation_t, noise_config_t> default_noise;
    std::map<memory_operation_t, noise_config_t> static_analysis_noise;
    std::map<memory_operation_t, std::vector<memop_filter>> filters;
    std::vector<std::string> config_strings;
};

static struct config global_config{};

std::string read_file(const char *path)
{
    std::ifstream config_file(path);
    if (!static_cast<bool>(config_file))
    {
        std::cerr << "Failed to load config file " << path << std::endl;
        return std::string{};
    }
    std::stringstream buffer;
    buffer << config_file.rdbuf();
    return std::move(buffer.str());
}

noise_type to_noise_type(std::string_view type)
{
    if (type == "wait")
    {
        return noise_type::WAIT;
    }
    if (type == "sleep")
    {
        return noise_type::SLEEP;
    }
    if (type == "yield")
    {
        return noise_type::YIELD;
    }

    if (type == "inverse")
    {
        return noise_type::INVERSE;
    }
    if (type == "debug")
    {
        return noise_type::DEBUG;
    }

    return noise_type::NONE;
}

void load_noise_config(ryml::NodeRef node, std::map<memory_operation_t, noise_config_t> &noise_config)
{
    static const std::vector noise_op_string = {"write", "read", "update"};
    static const std::vector noise_op_enum = {memory_operation_t::WRITE, memory_operation_t::READ, memory_operation_t::UPDATE};
    for (int i = 0; i < noise_op_string.size(); i++)
    {
        ryml::NodeRef n = node[noise_op_string[i]];
        std::string type_string;
        n["type"] >> type_string;
        noise_type type = to_noise_type(type_string);

        int frequency, strength;
        bool random;
        n["frequency"] >> frequency;
        n["strength"] >> strength;
        n["random"] >> random;
        noise_config_t noise = {type, frequency, strength, random};
        noise_config[noise_op_enum[i]] = std::move(noise);
    }
}

memory_operation_t to_memory_op(std::string_view op)
{
    if (op == "write")
    {
        return memory_operation_t::WRITE;
    }
    if (op == "read")
    {
        return memory_operation_t::READ;
    }

    return memory_operation_t::UPDATE;
}

void load_filter(ryml::NodeRef node, std::map<memory_operation_t, std::vector<memop_filter>>& filters)
{
    std::string name;
    node["base"]["name"]>>name;
    std::string op;
    {
    auto n=node["access1"];

    n["kind"]>>op;
    memop_filter filter{};
    filter.variable_name = name;
    n["location"]["file"]>>filter.file;
    n["location"]["line"]>>filter.line;
    filters[to_memory_op(op)].emplace_back(std::move(filter));
    }

    auto n=node["access2"];
    n["kind"]>>op;
    memop_filter filter{};
    filter.variable_name = name;
    n["location"]["file"]>>filter.file;
    n["location"]["line"]>>filter.line;
    filters[to_memory_op(op)].emplace_back(std::move(filter));
    

}

void load_config()
{
    global_config.config_strings.emplace_back(read_file("config.yaml"));
    std::string &config_string = global_config.config_strings.back();
    if (config_string.length() == 0)
    {
        exit(1);
    }

    ryml::Tree config = ryml::parse_in_place(ryml::to_substr(config_string));
    std::string static_analysis_file;
    config["static_analysis_file"] >> static_analysis_file;
    config["analysis"] >> global_config.analysis;
    load_noise_config(config["noise"]["default"], global_config.default_noise);
    load_noise_config(config["noise"]["static_detections"], global_config.static_analysis_noise);
    
    global_config.config_strings.emplace_back(read_file(static_analysis_file.c_str()));
    std::string &static_results_string = global_config.config_strings.back();
    if (static_results_string.length() == 0)
    {
        return;
    }

    ryml::Tree static_config = ryml::parse_in_place(ryml::to_substr(static_results_string));
    
    int detected_races = static_config["data_races"].num_children();
    for (int i = 0; i < detected_races; i++)
    {
        load_filter(static_config["data_races"][i],global_config.filters);
    }
}

std::vector<memop_filter>& get_filters(memory_operation_t op){
    return global_config.filters[op];
}

noise_config_t& get_default_noise(memory_operation_t op){
    return global_config.default_noise[op];
}

noise_config_t& get_targeted_noise(memory_operation_t op){
    return global_config.static_analysis_noise[op];
}