#pragma once

#include <cstring>

namespace scribe {
	struct ReportPolicy {
		void operator()(const char *begin, const size_t len) {
			fmt::print("TODO: Implement this method!");
		}
	};
}
