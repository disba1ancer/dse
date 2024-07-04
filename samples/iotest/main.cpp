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
    CachedFile file(ctx, u8"test.bin", OpenMode::Read);
    unsigned char byte;
    while (true) {
        auto [bytesRead, st] = file.Read((std::byte*)&byte, 1);
        if (st != Code::Success) {
            std::cout << "\n" << bytesRead << " ["
                      << (char*)SourceName(st) << ":" << (char*)Message(st)
                      << "]\n";
            break;
        }
        std::cout << int(byte) << " ";
    }
    endl(std::cout);
}
