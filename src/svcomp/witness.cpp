#include <ryml_all.hpp>
#include "witness.h"
#include "uuid/uuid.h"
#include <filesystem>
#include <cstdio>
#include <iostream>
#include <fstream>

const uuids::uuid get_uuid(){
    static std::mt19937 generator = []{
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        return std::mt19937(seq);
    }();
    
    static uuids::uuid_random_generator gen{generator};
    return gen();
}

void set_thread_creation_segment(ryml::NodeRef segment,AccessData& access){
    segment.set_seq();
    ryml::NodeRef item = segment.append_child();
    item.set_map();
    ryml::NodeRef waypoint = item["waypoint"];
    waypoint.set_map();
    waypoint["type"].set_val("function_enter");
    waypoint["action"].set_val("follow");
    waypoint["thread_id"].set_serialized(0);
    ryml::NodeRef location = waypoint["location"];
    location.set_map();
    location["file_name"].set_serialized(access.thread_data.create_location.file);
    location["line"].set_serialized(access.thread_data.create_location.line);
    location["column"].set_serialized(access.thread_data.create_location.column);
    location["function"].set_serialized(access.thread_data.func_name);
    
}

void set_access_waypoint(ryml::NodeRef waypoint, AccessData access){
    waypoint.set_map();
    waypoint["type"].set_val("target");
    waypoint["action"].set_val("follow");
    waypoint["thread_id"].set_serialized(access.variable_data.thread);
    
    ryml::NodeRef location = waypoint["location"];
    location.set_map();
    location["file_name"].set_serialized(access.variable_data.location.file);
    location["file_name"].set_val_style(ryml::VAL_DQUO);
    location["line"].set_serialized(access.variable_data.location.line);
    location["column"].set_serialized(access.variable_data.location.column);
    location["function"].set_serialized(access.func_name);
    location["function"].set_val_style(ryml::VAL_DQUO);
}

void set_data_race_segment(ryml::NodeRef segment,AccessData& access1, AccessData& access2){
    segment.set_seq();

    ryml::NodeRef t1_item = segment.append_child();
    t1_item.set_map();
    set_access_waypoint(t1_item["waypoint"],access1);

    ryml::NodeRef t2_item = segment.append_child();
    t2_item.set_map();
    set_access_waypoint(t2_item["waypoint"],access2);
}

void set_content(ryml::NodeRef content,AccessData& access1, AccessData& access2){
    content.set_seq();
    int tid=1;
    if (access1.variable_data.thread != 0)
    {
        ryml::NodeRef t1_item = content.append_child();
        t1_item.set_map();
        set_thread_creation_segment(t1_item["segment"],access1);
        access1.variable_data.thread = tid++;
    }
    

    if (access2.variable_data.thread != 0)
    {
        ryml::NodeRef t2_item = content.append_child();
        t2_item.set_map();
        set_thread_creation_segment(t2_item["segment"],access2);
        access2.variable_data.thread = tid;
    }

    
    ryml::NodeRef race_item = content.append_child();
    race_item.set_map();
    set_data_race_segment(race_item["segment"],access1,access2);
}

void set_metadata(ryml::NodeRef metadata,std::string file_name){
    metadata.set_map();
    metadata["format_version"].set_val("2.2");
    metadata["format_version"].set_val_style(ryml::VAL_DQUO);
    std::string uuid = uuids::to_string(get_uuid());
    metadata["uuid"].set_serialized(uuid);
    metadata["uuid"].set_val_style(ryml::VAL_DQUO);

    metadata["creation_time"].set_val("2025-10-01T13:00:00Z");
    metadata["creation_time"].set_val_style(ryml::VAL_DQUO);
    ryml::NodeRef producer = metadata["producer"];
    producer.set_map();
    producer["name"].set_val("Moirae");
    producer["name"].set_val_style(ryml::VAL_DQUO);
    producer["version"].set_val("1.0");
    producer["version"].set_val_style(ryml::VAL_DQUO);
    ryml::NodeRef task = metadata["task"];
    task.set_map();
    ryml::NodeRef input_files = task["input_files"];
    input_files.set_seq();
    auto tmp= input_files.append_child();
    tmp.set_serialized(file_name);
    tmp.set_val_style(ryml::VAL_DQUO);
    ryml::NodeRef hashes =  task["input_file_hashes"];
    hashes.set_map();
    hashes["__FILE__"].set_val("__HASH__");
    task["specification"].set_val("G ! data-race");
    task["specification"].set_val_style(ryml::VAL_DQUO);
    task["data_model"].set_val("ILP32");
    task["data_model"].set_val_style(ryml::VAL_DQUO);
    task["language"].set_val("C");
    task["language"].set_val_style(ryml::VAL_DQUO);
    
}

static std::mutex witness_mutex;
void create_witness(AccessData access1,AccessData access2){
    std::lock_guard lock(witness_mutex);
    static bool witness_generated = false;    
    if (witness_generated)
    {
        return;
    }

    ryml::Tree witness;
    ryml::NodeRef root = witness.rootref();
    root.set_seq();
    ryml::NodeRef entry =  root.append_child();

    entry.set_map();
    entry["entry_type"].set_val("violation_sequence");
    set_metadata(entry["metadata"], access1.variable_data.location.file);

    set_content(entry["content"],access1,access2);

    const std::filesystem::path witness_path{"witness.yml"};
    std::ofstream witness_file(witness_path);
    witness_file << witness; 
    witness_generated=true;
}