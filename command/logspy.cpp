#include "clara.hpp"
#include "fmt/format.h"
#include <algorithm>
#include <vector>

#include "stream.hpp"
#include "utils.hpp"
#include "params.hpp"
#include "policies.hpp"
#include "report.hpp"
#include "utils/timer.hpp"

int main(int argc, char *argv[]) {
    auto params = scribe::parse_input_arguments(argc, argv);
	utils::ElapsedTime<utils::SECOND> timer("Total runtime: ", params.timer());
	if (params.report()) {
		scribe::strip_scribe_headers<scribe::ReportPolicy>(params);
	} else if (params.table()) {
		scribe::strip_scribe_headers<scribe::CSVPolicy>(params);
	} else if (params.raw()) {
		scribe::strip_scribe_headers<scribe::RawPolicy>(params);
	} else if (params.json_output()) {
		scribe::strip_scribe_headers<scribe::CompactJsonPolicy>(params);
	} else if (params.json_compact_output()) {
		scribe::strip_scribe_headers<scribe::CompactJsonPolicy>(params);
	} else if (params.json_pretty_output()) {
		scribe::strip_scribe_headers<scribe::PrettyJsonPolicy>(params);
	} else {					// Generate the report by default.
		scribe::strip_scribe_headers<scribe::ReportPolicy>(params);
	}
}
