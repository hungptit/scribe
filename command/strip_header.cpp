#include "clara.hpp"
#include "fmt/format.h"
#include <algorithm>
#include <vector>

#include "stream.hpp"
#include "utils.hpp"

#include "utils/matchers.hpp"
#include "utils/matchers_avx2.hpp"
#include "utils/regex_matchers.hpp"

#include "ioutils/reader.hpp"
#include "ioutils/stream.hpp"

namespace scribe {
    enum PARAMS : int32_t {
        VERBOSE = 1,
        COLOR = 1 << 1,
        EXACT_MATCH = 1 << 2,
        INVERSE_MATCH = 1 << 3,
        STDIN = 1 << 4,
        JSON_OUTPUT = 1 << 5,
        JSON_BEAUTIFY_OUTPUT = 1 << 6,
    };

    struct StripParams {
        std::vector<std::string> paths;
        std::string pattern;
        unsigned int info;
        int regex_mode;

        bool verbose() const { return (info & VERBOSE) > 0; }
        bool color() const { return (info & COLOR) > 0; }
        bool exact_match() const { return (info & EXACT_MATCH) > 0; }
        bool inverse_match() const { return (info & INVERSE_MATCH) > 0; }
        bool stdin() const { return (info & STDIN) > 0; }
    };

    template <typename Reader> void extract(const StripParams &params) {
        Reader reader(params);
        for (auto const &afile : params.paths) { reader(afile.data()); }
    }

    void strip_scribe_headers(const StripParams &params) {
        if (params.pattern.empty()) {
			using Policy = scribe::StreamPolicy<scribe::All>;
			using Reader = ioutils::FileReader<Policy>;
			extract<Reader>(params);
        } else {
            if (params.exact_match()) {
				using Policy = scribe::StreamPolicy<utils::ExactMatchAVX2>;
				using Reader = ioutils::FileReader<Policy>;
				extract<Reader>(params);
            } else {
				if (!params.inverse_match()) {
					using Matcher = utils::hyperscan::RegexMatcher;
					using Policy = scribe::StreamPolicy<Matcher>;
					using Reader = ioutils::FileReader<Policy>;
					extract<Reader>(params);
				} else {
					using Matcher = utils::hyperscan::RegexMatcherInv;
					using Policy = scribe::StreamPolicy<Matcher>;
					using Reader = ioutils::FileReader<Policy>;
					extract<Reader>(params);
				}
            }
        }
    }

    StripParams parse_input_arguments(int argc, char *argv[]) {
        StripParams params;

        // Input argument
        bool help = false;

        bool inverse_match =
            false; // Inverse match i.e display lines that do not match given pattern.
        bool exact_match = false; // Use exact matching algorithm.
        bool ignore_case = false; // Ignore case.
        bool stdin = false;       // Read data from STDIN.
        bool color = false;       // Display color text.
        bool verbose = false;     // Display verbose information.

        auto cli = clara::Help(help) |
                   clara::Opt(verbose)["-v"]["--verbose"]("Display verbose information") |
                   clara::Opt(exact_match)["--exact-match"]("Use exact matching algorithms.") |
                   clara::Opt(inverse_match)["--inverse-match"](
                       "Print lines that do not match given pattern.") |
                   clara::Opt(ignore_case)["-i"]["--ignore-case"]("Ignore case") |
                   clara::Opt(color)["-c"]["--color"]("Print out color text.") |
                   clara::Opt(stdin)["-s"]["--stdin"]("Read data from the STDIN.") |
			clara::Opt(params.pattern, "pattern")["-e"]["-p"]["--pattern"]("Search pattern.") |

                   // Required arguments.
                   clara::Arg(params.paths, "paths")("Search paths");

        auto result = cli.parse(clara::Args(argc, argv));
        if (!result) {
            fmt::print(stderr, "Invalid option: {}\n", result.errorMessage());
            exit(EXIT_FAILURE);
        }

        // If users want to print out the help message then display the help message and exit.
        if (help) {
            std::ostringstream oss;
            oss << cli;
            fmt::print("{}", oss.str());
            exit(EXIT_SUCCESS);
        }

        // Update search parameters
        params.regex_mode =
            HS_FLAG_DOTALL | HS_FLAG_SINGLEMATCH | (ignore_case ? HS_FLAG_CASELESS : 0);
        params.info = verbose * scribe::VERBOSE | color * scribe::COLOR |
                      exact_match * scribe::EXACT_MATCH |
                      inverse_match * scribe::INVERSE_MATCH | stdin * scribe::STDIN;

        // Print out input parameters if verbose flag is set.
        if (verbose) { fmt::print("{}", params); };

        return params;
    }
} // namespace scribe

// Define the format template for scribe::StripParams.
namespace fmt {
    template <> struct formatter<scribe::StripParams> {
        template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const scribe::StripParams &p, FormatContext &ctx) {
            return format_to(ctx.begin(),
                             "Search pattern: {0}\nregex_mode: {1}\nverbose: {2}\ncolor: "
                             "{3}\ninverse_match: {4}\nexact_match: {5}\nstdin: {6}\n",
                             p.pattern, p.regex_mode, p.verbose(), p.color(), p.inverse_match(),
                             p.exact_match(), p.stdin());
        }
    };
} // namespace fmt

int main(int argc, char *argv[]) {
    auto params = scribe::parse_input_arguments(argc, argv);
    scribe::strip_scribe_headers(params);
}
