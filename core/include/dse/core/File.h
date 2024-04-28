/*
 * IOTarget.h
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_FILE_H_
#define DSE_CORE_FILE_H_

#include <memory>
#include <string_view>
#include <string>
#include <dse/util/enum_bitwise.h>
#include <dse/util/functional.h>
#include <dse/util/execution.h>
#include "errors.h"
#include "status.h"
#include "ThreadPool.h"
#include "detail/impexp.h"

namespace dse::core {

#ifdef _WIN32

class File_win32;

typedef File_win32 IOTarget_impl;

#endif

enum class OpenMode : unsigned {
	Read = 0x1U,
	Write = 0x2U,
	Append = 0x4U,
	Clear = 0x8U, // clear file contents when opened
	Existing = 0x10U, // open only existing
};

enum class StPoint {
	START,
	CURRENT,
	END,
};

class API_DSE_CORE IOTargetDelete {
public:
	void operator()(IOTarget_impl* obj);
};

class API_DSE_CORE File;

namespace impl {

struct FileOpResult {
	std::size_t transfered;
	Status ecode;
};

enum class FileOp {
	Read,
	Write
};

template <FileOp op>
struct FileOpBuf;

template <>
struct FileOpBuf<FileOp::Read> {
	using Type = std::byte;
};

template <>
struct FileOpBuf<FileOp::Write> {
	using Type = const std::byte;
};

template <FileOp op>
using FileOpBufT = typename FileOpBuf<op>::Type;

template <FileOp op, typename R>
class FileOpstate;

template <FileOp op, typename R>
void dse_TagInvoke(util::TagT<util::Start>, FileOpstate<op, R>& opstate);

template <FileOp op, typename R>
class FileOpstate {
	File *file;
	FileOpBufT<op> *buf;
	std::size_t size;
	std::remove_cvref_t<R> recv;
	void callback(std::size_t size, Status error)
	{
		util::SetValue(std::move(recv), size, error);
	}
public:
	FileOpstate(
		File *file,
		FileOpBufT<op> *buf,
		std::size_t size,
		std::remove_cvref_t<R> recv
	) : file(file), buf(buf), size(size), recv(recv)
	{}
	friend void dse_TagInvoke<op, R>(util::TagT<util::Start>, FileOpstate& opstate);
};

template <FileOp op>
class FileSender {
	File *file;
	FileOpBufT<op> *buf;
	std::size_t size;
public:
	template <template<typename ...> typename T>
	using ValueTypes = T<std::size_t, Status>;
	using ErrorType = std::exception_ptr;
	static constexpr bool SendsDone = false;
	FileSender(File *file, FileOpBufT<op> buf[], std::size_t size) :
		file(file), buf(buf), size(size)
	{}
	template <util::ReceiverOf<std::size_t, Status> R>
	friend auto dse_TagInvoke(util::TagT<util::Connect>, FileSender&& sndr, R&& recv)
	-> FileOpstate<op, R>
	{
		return { sndr.file, sndr.buf, sndr.size, std::forward<R>(recv) };
	}
};

}

class API_DSE_CORE File {
public:
	using FilePos = std::uint_least64_t;
	using FileOff = std::int_least64_t;
	using Callback = util::FunctionPtr<void(std::size_t, Status)>;
private:
	std::unique_ptr<IOTarget_impl, IOTargetDelete> impl;
public:
	File();
	File(ThreadPool& pool, std::u8string_view filepath, OpenMode mode);
	//~File(); // may cause undefined behavior if called while async operation
	auto Read(std::byte buf[], std::size_t size) -> impl::FileOpResult;
	auto Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult;
	auto Resize() -> Status;
	auto ReadAsync(std::byte buf[], std::size_t size, const Callback& cb) -> Status;
	auto WriteAsync(const std::byte buf[], std::size_t size, const Callback& cb) -> Status;
	auto ReadAsync(std::byte buf[], std::size_t size) -> impl::FileSender<impl::FileOp::Read>
	{
		return { this, buf, size };
	}
	auto WriteAsync(const std::byte buf[], std::size_t size) -> impl::FileSender<impl::FileOp::Write>
	{
		return { this, buf, size };
	}
	auto Cancel() -> Status;
	bool IsEOF() const;
	bool IsValid() const;
	auto Seek(FilePos pos) -> Status;
	auto Seek(FileOff offset, StPoint rel) -> Status;
    auto GetStatus() const -> Status;
	auto Tell() const -> FilePos;
};

template <impl::FileOp op, typename R>
void impl::dse_TagInvoke(util::TagT<util::Start>, FileOpstate<op, R>& opstate)
{
	if constexpr (op == FileOp::Read) {
		opstate.file->ReadAsync(opstate.buf, opstate.size, {opstate, util::fnTag<&FileOpstate<op, R>::callback>});
	} else {
		opstate.file->WriteAsync(opstate.buf, opstate.size, {opstate, util::fnTag<&FileOpstate<op, R>::callback>});
	}
}

} // namespace dse::core

namespace dse::util {

template <>
struct enable_enum_bitwise<enum core::OpenMode> : public std::true_type {};

} // namespace dse::util

#endif /* DSE_CORE_FILE_H_ */
