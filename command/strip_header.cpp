#include "clara.hpp"
#include "fmt/format.h"
#include <algorithm>
#include <vector>

#include "stream.hpp"
#include "utils.hpp"
#include "params.hpp"
#include "policies.hpp"

int main(int argc, char *argv[]) {
    auto params = scribe::parse_input_arguments(argc, argv);
	if (params.report()) {
		fmt::print("TODO: A report");
	} else if (params.table()) {
		fmt::print("TODO: A table");
	} else if (params.raw()) {
		scribe::strip_scribe_headers<scribe::RawPolicy>(params);
	} else if (params.json_output()) {
		scribe::strip_scribe_headers<scribe::CompactJsonPolicy>(params);
	} else if (params.json_compact_output()) {
		scribe::strip_scribe_headers<scribe::CompactJsonPolicy>(params);
	} else if (params.json_pretty_output()) {
		scribe::strip_scribe_headers<scribe::PrettyJsonPolicy>(params);
	} else {
		// Generate a report.
	}
}
