#include "dse/util/coroutine.h"
#include <cstring>
#include <dse/core/CachedFile.h>
#include <dse/core/IOContext.h>
#include <iostream>
#include <dse/util/execution.h>

using dse::core::IOContext;
using dse::core::CachedFile;
using dse::core::OpenMode;
using dse::core::status::Code;
using dse::util::eager_task_t;

IOContext ctx;

auto MainTask(eager_task_t, int argc, char* argv[]) -> std::future<int>;

int main(int argc, char* argv[])
{
    try {
        auto future = MainTask({}, argc, argv);
        ctx.Run();
        return future.get();
    } catch (dse::core::status::StatusException& e) {
        std::cerr << "[ERROR] " << e.what() << "\n";
        return 1;
    }
}

auto MainTask(eager_task_t, int argc, char* argv[]) -> std::future<int>
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
        ThrowIfError((co_await out.WriteAsync("[ERROR] ", 9)).ecode);
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
