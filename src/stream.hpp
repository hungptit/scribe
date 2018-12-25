#pragma once

#include "constants.hpp"
#include "fmt/format.h"
#include "utils.hpp"
#include "utils/memchr.hpp"
#include <cstring>
#include <string>


#include "policies.hpp"

namespace scribe {
    template <typename Matcher, typename OutputPolicy = RawPolicy> class StreamPolicy {
      public:
        template <typename Params>
        StreamPolicy(Params &&params)
            : matcher(params.pattern, params.regex_mode), lines(1), pos(0), linebuf(),
              output(std::forward<Params>(params)) {
            color = params.color();
            verbose = params.verbose();
        }

        ~StreamPolicy() { process_linebuf(); }

        void process(const char *begin, const size_t len) {
            const char *start = begin;
            const char *end = begin + len;
            const char *ptr = begin;
            while (
                (ptr = static_cast<const char *>(utils::avx2::memchr(ptr, EOL, end - ptr)))) {
                if (linebuf.empty()) {
                    process_line(start, ptr - start + 1);
                } else {
                    linebuf.append(start, ptr - start + 1);
                    process_linebuf();
                    linebuf.clear();
                }

                // Update parameters
                start = ++ptr;
                ++lines;

                // Stop if we reach the end of the buffer.
                if (start == end) break;
            }

            // Update the line buffer with leftover data.
            if (ptr == nullptr) { linebuf.append(start, end - start); }
            pos += len;
        }

      private:
        Matcher matcher;
        size_t lines = 1;
        size_t pos = 0;
        std::string linebuf;
        OutputPolicy output;
        bool verbose = false;
        bool color = false;

      protected:
        void process_line(const char *begin, const size_t len) {
            if ((len > 0) && matcher.is_matched(begin, len)) {
                auto end = begin + len;

                // Extract the scribe header by finding the start of JSON text data.
                const char *ptr = static_cast<const char *>(
                    utils::avx2::memchr(begin, OPEN_CURLY_BRACE, len));
                if (ptr != nullptr) {
                    output(ptr, end - ptr);
                } else {
                    // TODO: What should we do with the invalid log messages?
                    fmt::print(stderr, "Invalid log message: {0} -> {1}\n",
                               std::string(begin, begin + len), len);
                }
            }
        }

        // Process text data in the linebuf.
        void process_linebuf() { process_line(linebuf.data(), linebuf.size()); }
    };

    struct All {
        All(const std::string &, int) {}
        bool is_matched(const char *, const size_t) { return true; }
    };
} // namespace scribe
