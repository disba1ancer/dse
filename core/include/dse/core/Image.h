#ifndef DSE_CORE_IMAGE_H
#define DSE_CORE_IMAGE_H

#include <memory>
#include <dse/math/vec.h>
#include "ITextureDataProvider.h"
#include <dse/util/functional.h>
#include "ImageManipulator.h"

namespace dse::core {

struct ImageDataDeleter {
    void operator()(void* ptr) const;
};

class Image
{
public:
    Image();
    Image(math::ivec2 size, bool linear = false);
    auto Size() -> math::ivec2;
    void* Data();
    auto Manipulator() -> ImageManipulator;
    static void LoadByProvider(ITextureDataProvider* provider, util::function_ptr<void(Image&&)> callback);
private:
    std::unique_ptr<void, ImageDataDeleter> data;
    math::ivec2 size;
    bool linear;
};

} // namespace dse::core

#endif // DSE_CORE_IMAGE_H
