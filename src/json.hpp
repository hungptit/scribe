#pragma once

#include "fmt/format.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include <string>

namespace scribe {
    struct CompactJsonPolicy {
        template <typename Params>
        CompactJsonPolicy(Params &&params) : silent(params.silent()), linebuf() {}

        void operator()(const char *begin, const size_t len) {
            linebuf.clear();
            linebuf.append(begin, len);
            char *data = const_cast<char *>(linebuf.data());

            // Parse given JSON string
            rapidjson::Document document;
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
        std::string linebuf;
    };

    struct PrettyJsonPolicy {
        template <typename Params>
        PrettyJsonPolicy(Params &&params) : silent(params.silent()), linebuf() {}

        void operator()(const char *begin, const size_t len) {
            linebuf.clear();
            linebuf.append(begin, len);
            char *data = const_cast<char *>(linebuf.data());

            // Parse given JSON string
            rapidjson::Document document;
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
        std::string linebuf;
    };

} // namespace scribe
