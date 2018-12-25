#pragma once

#include "fmt/format.h"
#include <cstring>
#include <string>

#include "csv.hpp"
#include "json.hpp"
#include "report.hpp"
#include "sqlite.hpp"

namespace scribe {
    class RawPolicy {
      public:
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

      private:
        bool silent = false;
        bool color = false;
    };

	struct StorePolicy {
        template <typename Params>
        StorePolicy(Params &&params) : silent(params.silent()){}
        void operator()(const char *begin, const size_t len) {
			if (!silent) {
				results.emplace_back(std::string(begin, len));
			}
        }
		bool silent = 0;
		std::vector<std::string> results;
	};
} // namespace scribe
