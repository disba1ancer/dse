/*
 * IOTarget.h
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_IO_FILE_H_
#define OS_IO_FILE_H_

#include <memory>
#include <string_view>
#include <string>
#include "util/enum_bitwise.h"
#include "util/future.h"
#include "errors.h"
#include "ThreadPool.h"

namespace dse::core {

#ifdef _WIN32

class File_win32;

typedef File_win32 IOTarget_impl;

#endif

enum class OpenMode : unsigned {
	READ = 0x1U,
	WRITE = 0x2U,
	APPEND = 0x4U,
	CLEAR = 0x8U, // clear file contents when opened
	EXISTING = 0x10U, // open only existing
};

enum class StPoint {
	START,
	CURRENT,
	END,
};

using namespace util::enum_bitwise;

class IOTargetDelete {
public:
	void operator()(IOTarget_impl* obj);
};

class File {
public:
	struct Result {
		std::size_t transfered;
		std::error_code ecode;
	};
	using FilePos = std::uint_least64_t;
	using FileOff = std::int_least64_t;
private:
	std::unique_ptr<IOTarget_impl, IOTargetDelete> impl;
public:
	File();
	File(ThreadPool& pool, std::u8string_view filepath, OpenMode mode);
	~File() noexcept; // may cause undefined behavior if called while async operation
	File(const File &other) = default;
	File(File &&other);
	auto operator=(const File &other) -> File& = default;
	auto operator=(File &&other) -> File&;
	auto read(std::byte buf[], std::size_t size) -> Result;
	auto write(std::byte buf[], std::size_t size) -> Result;
	auto resize() -> std::error_code;
	auto readAsync(std::byte buf[], std::size_t size) -> util::future<Result>;
	auto writeAsync(std::byte buf[], std::size_t size) -> util::future<Result>;
	auto cancel() -> std::error_code;
	bool isEOF() const;
	bool isValid() const;
	auto seek(FilePos pos) -> std::error_code;
	auto seek(FileOff offset, StPoint rel) -> std::error_code;
	auto status() const -> std::error_code;
	auto tell() const -> FilePos;
};

} // namespace dse::core

namespace dse::util {

template <>
struct enable_enum_bitwise<enum core::OpenMode> : public std::true_type {};

} // namespace dse::util

#endif /* OS_IO_FILE_H_ */
