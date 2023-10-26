#ifndef DSE_CORE_ITEXTUREDATAPROVIDER_H
#define DSE_CORE_ITEXTUREDATAPROVIDER_H

#include <dse/util/functional.h>

namespace dse::core {

struct ITextureDataProvider {
    enum PixelFormat {
        Unsupported,

        LinearStart,
        RGBA8 = LinearStart,
        RGBX8,
        RGB8,
        BGRA8,
        BGRX8,
        BGR8,
        LinearEnd,

        SRGBStart = LinearEnd,
        RGBA8sRGB = SRGBStart,
        RGBX8sRGB,
        RGB8sRGB,
        BGRA8sRGB,
        BGRX8sRGB,
        BGR8sRGB,
        SRGBEnd,

        R11G11B10 = SRGBEnd,

        ToSRGB = SRGBStart - LinearStart
    };

    struct PixelFormatParameters {
        unsigned char pixelSize;
    };
    static const PixelFormatParameters pixelFormatParameters[];
    struct TextureParameters {
        int width;
        int height;
        int depth;
        PixelFormat format;
        unsigned lodCount;
    };
    /**
     * @brief LoadParameters
     * Loads texture parameters asynchronously
     * @param parameters
     * Pointer to a structure to be filled with texture parameters
     * @param onReady
     * Callback to be called after filling in parameters.
     * @note
     * The thread on which the callback will be called is implementation-defined
     */
    virtual void LoadParameters(TextureParameters* parameters, util::FunctionPtr<void()> onReady) = 0;
    /**
     * @brief LoadData
     * Loads texture data for single level of detail asynchronously
     * @param recvBuffer
     * Pointer to a buffer to be filled with texture data
     * @param lod
     * The level of detail to be loaded
     * @param onReady
     * Callback to be called after filling in parameters.
     * @note
     * The thread on which the callback will be called is implementation-defined.
     * Size of buffer to be filled defined by texture parameters.
     */
    virtual void LoadData(void* recvBuffer, unsigned lod, util::FunctionPtr<void()> onReady) = 0;
    virtual auto GetVersion() -> unsigned = 0;
    ~ITextureDataProvider() = default;
};

}

#endif // DSE_CORE_ITEXTUREDATAPROVIDER_H
