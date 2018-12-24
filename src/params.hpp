#pragma once

#include "constants.hpp"
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
        SILENT = 1 << 5,
        JSON_OUTPUT = 1 << 6,
        JSON_COMPACT_OUTPUT = 1 << 7,
        JSON_PRETTY_OUTPUT = 1 << 8,
        RAW = 1 << 9,
        TABLE = 1 << 10,
        REPORT = 1 << 11,
    };

    struct Params {
        unsigned int info;
        int regex_mode;
        std::vector<std::string> paths;
        std::string pattern;
        std::string output_file;

        bool verbose() const { return (info & VERBOSE) > 0; }
        bool color() const { return (info & COLOR) > 0; }
        bool exact_match() const { return (info & EXACT_MATCH) > 0; }
        bool inverse_match() const { return (info & INVERSE_MATCH) > 0; }
        bool stdin() const { return (info & STDIN) > 0; }
        bool silent() const { return (info & SILENT) > 0; }
        bool json_output() const { return (info & JSON_OUTPUT) > 0; }
        bool json_compact_output() const { return (info & JSON_COMPACT_OUTPUT) > 0; }
        bool json_pretty_output() const { return (info & JSON_PRETTY_OUTPUT) > 0; }
        bool raw() const { return (info & RAW) > 0; }
        bool table() const { return (info & TABLE) > 0; }
        bool report() const { return (info & REPORT) > 0; }
    };

    template <typename Reader> void extract(const Params &params) {
        Reader reader(params);
        for (auto const &afile : params.paths) { reader(afile.data()); }
    }

    template <typename OutputPolicy> void strip_scribe_headers(const Params &params) {
        if (params.pattern.empty()) {
            using Policy = scribe::StreamPolicy<scribe::All, OutputPolicy>;
            using Reader = ioutils::FileReader<Policy>;
            extract<Reader>(params);
        } else {
            if (params.exact_match()) {
                using Policy = scribe::StreamPolicy<utils::ExactMatchAVX2, OutputPolicy>;
                using Reader = ioutils::FileReader<Policy>;
                extract<Reader>(params);
            } else {
                if (!params.inverse_match()) {
                    using Matcher = utils::hyperscan::RegexMatcher;
                    using Policy = scribe::StreamPolicy<Matcher, OutputPolicy>;
                    using Reader = ioutils::FileReader<Policy>;
                    extract<Reader>(params);
                } else {
                    using Matcher = utils::hyperscan::RegexMatcherInv;
                    using Policy = scribe::StreamPolicy<Matcher, OutputPolicy>;
                    using Reader = ioutils::FileReader<Policy>;
                    extract<Reader>(params);
                }
            }
        }
    }

    Params parse_input_arguments(int argc, char *argv[]) {
        Params params;

        // Input argument
        bool help = false;

        bool inverse_match =
            false; // Inverse match i.e display lines that do not match given pattern.
        bool exact_match = false; // Use exact matching algorithm.
        bool ignore_case = false; // Ignore case.
        bool stdin = false;       // Read data from STDIN.
        bool color = false;       // Display color text.
        bool verbose = false;     // Display verbose information.

        // Select output types
        bool silent = false;              // Do not output anything
        bool json_output = false;         // Output in JSON format
        bool json_compact_output = false; // Output in JSON compact format
        bool json_pretty_output = false;  // Output in JSON pretty format i.e with color.
        bool report = false;              // Generate a report.
        bool table = false;               // Output results in tabular format i.e CSV.
        bool raw = false; // Output raw data which is in the orignal JSON string.

        auto cli =
            clara::Help(help) |
            clara::Opt(verbose)["-v"]["--verbose"]("Display verbose information") |
            clara::Opt(exact_match)["--exact-match"]("Use exact matching algorithms.") |
            clara::Opt(inverse_match)["--inverse-match"](
                "Print lines that do not match given pattern.") |
            clara::Opt(ignore_case)["-i"]["--ignore-case"]("Ignore case") |
            clara::Opt(color)["-c"]["--color"]("Print out color text.") |
            clara::Opt(raw)["--raw"]("Output raw data.") |
            clara::Opt(report)["--report"]("Generate a report for all log messages.") |
            clara::Opt(table)["--table"]("Generate a report in a tabular format i.e CSV.") |
            clara::Opt(silent)["--silent"]("Do not output results.") |
            clara::Opt(json_output)["--json"]("Output results in JSON format.") |
            clara::Opt(json_compact_output)["--compact-json"](
                "Output results in JSON compact format.") |
            clara::Opt(json_pretty_output)["--pretty-json"](
                "Output results in JSON pretty format.") |
            clara::Opt(stdin)["-s"]["--stdin"]("Read data from the STDIN.") |
            clara::Opt(params.output_file,
                       "output")["-o"]["--output"]("The output file name.") |
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
                      inverse_match * scribe::INVERSE_MATCH | stdin * scribe::STDIN |
                      silent * scribe::SILENT | json_output * scribe::JSON_OUTPUT |
                      json_compact_output * scribe::JSON_COMPACT_OUTPUT |
                      json_pretty_output * scribe::JSON_PRETTY_OUTPUT | raw * scribe::RAW |
                      table * scribe::TABLE | report * scribe::REPORT;

        // Print out input parameters if verbose flag is set.
        if (verbose) { fmt::print("{}", params); };

        return params;
    }
} // namespace scribe

// Define the format template for scribe::Params.
namespace fmt {
    template <> struct formatter<scribe::Params> {
        template <typename ParseContext> constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const scribe::Params &p, FormatContext &ctx) {
            return format_to(
                ctx.begin(),
                "Search pattern: {0}\nInput options:\n\tregex_mode: "
                "{1}\n\tverbose: {2}\n\tcolor: "
                "{3}\n\tinverse_match: {4}\n\texact_match: {5}\n\tjson: "
                "{6}\n\tcompact-json: {7}\n\tpretty-json: {8}\n\tsilent: {9}\n\tstdin: {10}\n",
                p.pattern, p.regex_mode, p.verbose(), p.color(), p.inverse_match(),
                p.exact_match(), p.json_output(), p.json_compact_output(),
                p.json_pretty_output(), p.silent(), p.stdin());
        }
    };
} // namespace fmt
