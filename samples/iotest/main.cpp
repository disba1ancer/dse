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
    CachedFile file(ctx, u8"test.bin", OpenMode::Read | OpenMode::Write);
    unsigned char byte[1];
    for (int i = 0; i < 4096; ++i) {
        auto [bytesRead, st] = file.Read((std::byte*)byte, 1);
        if (st != Code::Success) {
            std::cout << "\n" << bytesRead << " ["
                      << (const char*)SourceName(st) << ":" << (const char*)Message(st)
                      << "]\n";
            break;
        }
        std::cout << int(*byte) << " ";
    }
    const char8_t str[] = u8"Hello, World!!!";
    file.Seek(0, dse::core::StPoint::Current);
    auto [bytesRead, st] = file.Write((std::byte*)str, sizeof(str));
    if (st != Code::Success) {
        std::cout << "\n" << bytesRead << " ["
                  << (const char*)SourceName(st) << ":" << (const char*)Message(st)
                  << "]\n";
    }
    st = file.Flush();
    if (st != Code::Success) {
        std::cout << "\n" << bytesRead << " ["
                  << (const char*)SourceName(st) << ":" << (const char*)Message(st)
                  << "]\n";
    }
    endl(std::cout);
}
