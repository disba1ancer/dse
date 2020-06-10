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
#include "../../util/enum_bitwise.h"
#include <string>
#include <functional>

namespace dse {
namespace os {
namespace io {

#ifdef _WIN32

class File_win32;

typedef File_win32 IOTarget_impl;

#endif

typedef std::uint_least64_t FilePos;
typedef std::int_least64_t FileOff;

enum class OpenMode : unsigned {
	READ = 0x1U,
	WRITE = 0x2U,
	APPEND = 0x4U,
	CLEAR = 0x8U, // clear file contents when opened
	EXISTING = 0x10U, // open only existing
};

enum class Result {
	SUCCESS,
	UNKNOWN,
	END_OF_FILE,
	ASYNC_OPERATION
};

enum class StPoint {
	START,
	CURRENT,
	END,
};

class IOTargetDelete {
public:
	void operator()(IOTarget_impl* obj);
};

class File {
	std::unique_ptr<IOTarget_impl, IOTargetDelete> impl;
public:
	File();
	File(std::u8string_view filepath, OpenMode mode);
	~File() noexcept; // may cause undefined behavior if called while async operation
	File(const File &other) = default;
	File(File &&other);
	File& operator=(const File &other) = default;
	File& operator=(File &&other);
	bool isValid() const;
	File& read(std::byte buf[], std::size_t size);
	File& write(std::byte buf[], std::size_t size);
	FilePos tell() const;
	File& seek(FilePos pos);
	File& seek(FileOff offset, StPoint rel);
	File& resize();
	bool isEOF() const;
	bool isError() const;
	Result getLastResult() const;
	std::size_t getTransfered() const;
	std::u8string getResultString() const;
	File& read_async(std::byte buf[], std::size_t size, std::function<void()>&& onFinish = nullptr);
	File& write_async(std::byte buf[], std::size_t size, std::function<void()>&& onFinish = nullptr);
	bool isBusy();
	File& cancel();
};

} /* namespace io */
} /* namespace os */

namespace util {

template <>
struct enable_enum_bitwise<os::io::OpenMode> {
	static constexpr bool value = true;
};

} /* namespace util */
} /* namespace dse */

#endif /* OS_IO_FILE_H_ */
