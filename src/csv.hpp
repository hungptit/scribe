#pragma once

#include "fmt/format.h"
#include <cstring>

namespace scribe {
    struct CSVPolicy {
        template <typename Params> CSVPolicy(Params &&params) : silent(params.silent()) {}
        void operator()(const char *begin, const size_t len) {
            fmt::print("TODO: Implement this method!");
        }
		
		bool silent = false;
    };
} // namespace scribe
