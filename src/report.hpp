#pragma once

#include "fmt/format.h"
#include "rapidjson/document.h"
#include <cstring>
#include <limits>
#include <unordered_map>

#include "utils/timestamp.hpp"

// TODO: Need to have a graph for job status.

namespace scribe {
    enum JobStatus : uint32_t {
        NONE = 0,
        PUBLISH = 1 << 1,
        RELAYING = 1 << 2,
        RECEIVED = 1 << 3,
        EXECUTING = 1 << 4,
        FINISHED = 1 << 5,
        ERROR = 1 << 6
    };
    struct JobInfo {
        uint32_t type = 0;
        uint32_t job = 0;
        uint32_t resource = 0;
        uint32_t pool = 0;
        uint32_t schema = 0;
        uint32_t instance = 0;
        uint64_t publish_time = 0;
        double runtime = std::numeric_limits<double>::max();
    };

    class ReportPolicy {
      public:
        template <typename Params>
        ReportPolicy(Params &&params)
            : silent(params.silent()), verbose(params.verbose()), linebuf() {}

        ~ReportPolicy() {
            // Sort results
            std::sort(jobs.begin(), jobs.end());
            std::sort(resources.begin(), resources.end());
            std::sort(pools.begin(), pools.end());
            std::sort(schemas.begin(), schemas.end());
            std::sort(instances.begin(), instances.end());

            // Display results
            print();
        }

        void print() const {
            auto print_obj = [](const std::string &title, auto table, const bool verbose) {
                fmt::print("\033[1;35m{0}\033[0m: {1}\n", title, table.size());
                if (verbose) {
                    for (auto &item : table) {
                        fmt::print("    - \033[1;32m{0}\033[0m\n", item);
                    }
                }
            };

            print_obj("The number of resources", resources, verbose);
            print_obj("The number of jobs", jobs, verbose);
            print_obj("The number of pools", pools, verbose);
            print_obj("The number of schemas", schemas, verbose);
            print_obj("The number of instances", instances, verbose);
        }

        void operator()(const char *begin, const size_t len) {
            if (len == 0) return;
            linebuf.clear();
            linebuf.append(begin, len);
            char *data = const_cast<char *>(linebuf.data());

            rapidjson::Document document;
            if (document.ParseInsitu(data).HasParseError()) {
                fmt::print(stderr, "Cannot parse given string: \033[1;32m{0}\033[0m\n",
                           std::string(begin, len));
                return;
            }

            // Process parsed JSON data
            if (document.HasMember("PREFIX")) {
                std::string prefix = document["PREFIX"].GetString();
                std::string level = document["LEVEL"].GetString();
                JobInfo info;

                if (prefix.empty()) return; // Skip if we cannot get the message id.

                if (document.HasMember("MESSAGE")) {
                    // Below are possible types of messages
                    // 1. RELAYING -> "Relaying message"
                    // 2. RECEIVED -> "received"
                    // 3. STARTING_EXECUTION -> "Starting execution"
                    // 4. FINISHED -> "finished in"

                    // std::string message = document["MESSAGE"].GetString();
                    // fmt::print("PREFIX: {0}, LEVEL: {1}, MESSAGE: {2}\n", prefix, level,
                    //            message);

                } else if (document.HasMember("REQUEST")) {
                    if (document.HasMember("RESOURCENAME")) {
                        std::string resource_name = document["RESOURCENAME"].GetString();
                        auto iter = resource_lookup_table.find(resource_name);
                        if (iter == resource_lookup_table.end()) {
                            info.resource = resources.size();
                            resources.push_back(resource_name);
                            resource_lookup_table.emplace(resource_name, info.resource);
                        } else {
                            info.resource = iter->second;
                        }
                    }

                    // Extract job information.
                    if (document["REQUEST"].HasMember("JOB")) {
                        std::string job = document["REQUEST"]["JOB"].GetString();
                        auto iter = job_lookup_table.find(job);
                        if (iter == job_lookup_table.end()) {
                            info.job = jobs.size();
                            jobs.push_back(job);
                            job_lookup_table.emplace(job, info.job);
                        } else {
                            info.job = iter->second;
                        }
                    }

                    // Extrace DB schema
                    if (document["REQUEST"].HasMember("SCHEMA")) {
                        std::string key = document["REQUEST"]["SCHEMA"].GetString();
                        auto iter = schema_lookup_table.find(key);
                        if (iter == schema_lookup_table.end()) {
                            info.schema = schemas.size();
                            schemas.push_back(key);
                            schema_lookup_table.emplace(key, info.schema);
                        } else {
                            info.schema = iter->second;
                        }
                    }

                    // Extract pool information.
                    if (document["REQUEST"].HasMember("POOL")) {
                        std::string key = document["REQUEST"]["POOL"].GetString();
                        auto iter = pool_lookup_table.find(key);
                        if (iter == pool_lookup_table.end()) {
                            info.pool = pools.size();
                            pools.push_back(key);
                            pool_lookup_table.emplace(key, info.pool);
                        } else {
                            info.pool = iter->second;
                        }
                    }

                    // Extract instance information.
                    if (document["REQUEST"].HasMember("INSTANCE")) {
                        std::string key = document["REQUEST"]["INSTANCE"].GetString();
                        auto iter = instance_lookup_table.find(key);
                        if (iter == instance_lookup_table.end()) {
                            info.instance = instances.size();
                            instances.push_back(key);
                            instance_lookup_table.emplace(key, info.instance);
                        } else {
                            info.instance = iter->second;
                        }
                    }

                } else if (document.HasMember("RAW_ERROR")) {
                    rapidjson::StringBuffer sb;
                    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
                    document.Accept(writer);
                    fmt::print("{0}\n", sb.GetString());
                } else {

                    rapidjson::StringBuffer sb;
                    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
                    document.Accept(writer);
                    fmt::print("Unrecognized JSON structure: {0}\n", sb.GetString());
                }
            } else {
                rapidjson::StringBuffer sb;
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
                document.Accept(writer);
                fmt::print("Invalid JSON structure: {0}\n", sb.GetString());
            }
        }

      private:
        bool silent = false;
        bool verbose = false;
        std::string linebuf;

        std::unordered_map<std::string, JobInfo> status; // Hold status of a current job

        std::unordered_map<std::string, unsigned int> job_lookup_table;
        std::vector<std::string> jobs;

        std::unordered_map<std::string, unsigned int> resource_lookup_table;
        std::vector<std::string> resources;

        std::unordered_map<std::string, unsigned int> pool_lookup_table;
        std::vector<std::string> pools;

        std::unordered_map<std::string, unsigned int> schema_lookup_table;
        std::vector<std::string> schemas;

        std::unordered_map<std::string, unsigned int> instance_lookup_table;
        std::vector<std::string> instances;
    };
} // namespace scribe
