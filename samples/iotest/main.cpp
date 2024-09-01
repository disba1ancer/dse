#include "dse/util/coroutine.h"
#include <dse/core/CachedFile.h>
#include <dse/core/IOContext.h>
#include <iostream>

using dse::core::IOContext;
using dse::core::CachedFile;
using dse::core::OpenMode;
using dse::core::status::Code;

IOContext ctx;

auto MainTask() -> dse::util::task<int>;

int main(int argc, char* argv[])
{
    try {
        auto task = MainTask();
        task.Resume();
        ctx.Run();
        return task.Result();
    } catch (dse::core::status::StatusException& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}

auto MainTask() -> dse::util::task<int>
{
    CachedFile file(ctx, u8"testsrc.txt", OpenMode::Read);
    CachedFile out(ctx, u8"testout.txt", OpenMode::Write | OpenMode::Clear);
    ThrowIfError(out.OpenStatus());
    char buf[4096] = {};
    while (true) {
        auto [bytesRead, st] = co_await file.ReadAsync(buf, sizeof(buf));
        ThrowIfError((co_await out.WriteAsync(buf, bytesRead)).ecode);
        // std::cout.write(buf, bytesRead);
        if (!IsError(st)) {
            continue;
        }
        ThrowIfError((co_await out.WriteAsync("\n[ERROR] ", 9)).ecode);
        auto msg = reinterpret_cast<const char*>(Message(st));
        ThrowIfError((co_await out.WriteAsync(msg, std::strlen(msg))).ecode);
        ThrowIfError((co_await out.WriteAsync("\n", 1)).ecode);
        // std::cout << "\n[ERROR] " << reinterpret_cast<const char*>(Message(st))
        // << "\n";
        break;
    }
    out.Flush();
    // endl(std::cout);
    co_return 0;
}
