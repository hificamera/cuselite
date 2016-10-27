#ifndef MYTYPES_H_
#define MYTYPES_H_
#include <atomic>
struct count_s {
	std::atomic_ulong count_request;
	std::atomic_ulong count_response;
	std::atomic_ulong count_response_by_other;
	std::atomic_ulong count_empty_response;
	std::atomic_ulong count_error_request;
	std::atomic_ulong count_error_rtb;
};
#endif
