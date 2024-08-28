#include <dse/core/CachedFile.h>
#include <dse/core/IOContext.h>
#include <iostream>

using dse::core::IOContext;
using dse::core::CachedFile;
using dse::core::OpenMode;
using dse::core::status::Code;

int main(int argc, char* argv[])
{
    IOContext ctx;
    CachedFile file(ctx, u8"test.bin", OpenMode::Read | OpenMode::Append);
    unsigned char buf[4096] = {};
    auto cb = [&buf](std::size_t size, dse::core::Status st) {
        if (IsError(st) && st != Code::EndOfStream) {
            return;
        }
        for (int i = 0; i < size; ++i) {
            std::cout << int(buf[i]) << " ";
        }
        endl(std::cout);
    };
    auto st = file.ReadAsync(reinterpret_cast<std::byte*>(buf), sizeof(buf), cb);
    if (st.ecode != Code::PendingOperation) {
        cb(st.transferred, st.ecode);
    }
    ctx.Run();
}
