#pragma once

#include "fmt/format.h"
#include <cstring>
#include <string>

namespace scribe {
    struct ReportPolicy {};

    struct RawPolicy {
        template <typename Params>
        RawPolicy(Params &&params) : silent(params.silent()), color(params.color()) {}
        void operator()(const char *begin, const size_t len) {
            if (!silent) {
                if (!color) {
                    fmt::print("{0}", std::string(begin, len));
                } else {
                    fmt::print("\033[1;32m{0}\033[0m", std::string(begin, len));
                }
            }
        }

        bool silent = false;
        bool color = false;
    };

    struct CompactJsonPolicy {
        template <typename Params>
        CompactJsonPolicy(Params &&params) : silent(params.silent()), document(), linebuf() {}

        void operator()(const char *begin, const size_t len) {
            linebuf.clear();
            linebuf.append(begin, len);
            char *data = const_cast<char *>(linebuf.data());

            // Parse given JSON string
            if (document.ParseInsitu(data).HasParseError()) {
                fmt::print(stderr, "Cannot parse given string: \033[1;32m{0}\033[0m\n",
                           std::string(begin, len));
                return;
            }

            // Print out results in the compact JSON format.
            if (!silent) {
                rapidjson::StringBuffer sb;
                rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
                document.Accept(writer);
                fmt::print("{0}\n", sb.GetString());
            }
        }

        bool silent = false;
        rapidjson::Document document;
        std::string linebuf;
    };

    struct PrettyJsonPolicy {
        template <typename Params>
        PrettyJsonPolicy(Params &&params) : silent(params.silent()), document(), linebuf() {}

        void operator()(const char *begin, const size_t len) {
            linebuf.clear();
            linebuf.append(begin, len);
            char *data = const_cast<char *>(linebuf.data());

            // Parse given JSON string
            if (document.ParseInsitu(data).HasParseError()) {
                fmt::print(stderr, "Cannot parse given string: \033[1;32m{0}\033[0m\n",
                           std::string(begin, len));
                return;
            }

            // Print out results in the compact JSON format.
            if (!silent) {
                rapidjson::StringBuffer sb;
                rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
                document.Accept(writer);
                fmt::print("{0}\n", sb.GetString());
            }
        }

        bool silent = false;
        rapidjson::Document document;
        std::string linebuf;
    };

    struct SQLitePolicy {};

    struct CSVPolicy {};
} // namespace scribe
