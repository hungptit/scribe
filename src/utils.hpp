#pragma once

#include "fmt/format.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h" // for stringify JSON
#include <cstdio>

namespace scribe {
    void print_color_text(const char *begin, const char *end) {
        fmt::print("\033[1;32m{0}\033[0m", std::string(begin, end - begin));
    }

    void print_plain_text(const char *begin, const char *end) {
        fmt::print("{0}", std::string(begin, end - begin));
    }
} // namespace scribe
