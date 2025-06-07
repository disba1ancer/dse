#ifndef DSE_CORE_SCENE2_H
#define DSE_CORE_SCENE2_H

#include <dse/util/evtmgr.h>
#include <dse/util/functional.h>
#include <dse/math/vec.h>
#include <dse/util/handle.h>
#include "status.h"

namespace dse::core {

struct IScene;
struct ISceneObject;
struct ISceneLight;
enum class SceneLightType;
struct ISceneVisitor;
struct IResourceManager;
struct IMesh2;
struct IMaterial;
struct ITexture;
enum class PixelFormat;

enum class SceneEvent {
    InvalidateAll,
    InvalidateObjects,
    ObjectCreated,
    ObjectModified,
    ObjectRemoved,
    InvalidateLights,
    LightCreated,
    LightModified,
    LightRemoved,
};

enum class ResourceEvent {
    MeshModified,
    MaterialModified,
    TextureModified,
};

namespace scene2_impl {

template <auto e>
struct event_traits;

template <>
struct event_traits<SceneEvent::InvalidateAll> {
    using handler = void(IScene&);
};

template <>
struct event_traits<SceneEvent::InvalidateObjects> {
    using handler = void(IScene&);
};

template <>
struct event_traits<SceneEvent::ObjectCreated> {
    using handler = void(IScene&, ISceneObject&);
};

template <>
struct event_traits<SceneEvent::ObjectModified> {
    using handler = void(IScene&, ISceneObject&);
};

template <>
struct event_traits<SceneEvent::ObjectRemoved> {
    using handler = void(IScene&, ISceneObject&);
};

template <>
struct event_traits<SceneEvent::InvalidateLights> {
    using handler = void(IScene&);
};

template <>
struct event_traits<SceneEvent::LightCreated> {
    using handler = void(IScene&, ISceneLight& x);
};

template <>
struct event_traits<SceneEvent::LightModified> {
    using handler = void(IScene&, ISceneLight& x);
};

template <>
struct event_traits<SceneEvent::LightRemoved> {
    using handler = void(IScene&, ISceneLight& x);
};

template <>
struct event_traits<ResourceEvent::MeshModified> {
    using handler = void(IResourceManager&, IMesh2& x);
};

template <>
struct event_traits<ResourceEvent::MaterialModified> {
    using handler = void(IResourceManager&, IMaterial& x);
};

template <>
struct event_traits<ResourceEvent::TextureModified> {
    using handler = void(IResourceManager&, ITexture& x);
};

} // namespace scene2_impl

} // namespace dse::core

namespace dse::util {

template <core::SceneEvent e>
struct event_traits<e> : core::scene2_impl::event_traits<e> {};

template <core::ResourceEvent e>
struct event_traits<e> : core::scene2_impl::event_traits<e> {};

}

namespace dse::core {

struct IScene : util::basic_subscribable<IScene, SceneEvent> {
    virtual void VisitObjects(ISceneVisitor& visitor) = 0;
    virtual void VisitLights(ISceneVisitor& visitor) = 0;
    virtual void VisitAll(ISceneVisitor& visitor) = 0;
    virtual auto SubscribeEvent(SceneEvent type, void* data, void(*handler)()) -> handler_id = 0;
    virtual void UnsubscribeEvent(handler_id id) = 0;
    using util::basic_subscribable<IScene, SceneEvent>::SubscribeEvent;
protected:
    ~IScene() = default;
};

struct ISceneObject {
    virtual auto GetPosition() -> math::vec3 = 0;
    virtual auto GetRotation() -> math::vec4 = 0;
    virtual auto GetScale() -> math::vec3 = 0;
    virtual auto GetMesh() -> IMesh2& = 0;
    virtual auto GetMaterial(int slotNum) -> IMaterial& = 0;
protected:
    ~ISceneObject() = default;
};

struct ISceneLight {
    virtual auto GetPosition() -> math::vec3 = 0;
    virtual auto GetRotation() -> math::vec4 = 0;
    virtual auto GetType() -> SceneLightType = 0;
    virtual auto GetBrightness() -> float = 0;
    virtual auto GetColor() -> math::vec3 = 0;
protected:
    ~ISceneLight() = default;
};

enum class SceneLightType {
    Point,
    Spot,
    Sun
};

struct ISceneVisitor
{
    virtual void Visit(ISceneObject& obj) = 0;
    virtual void Visit(ISceneLight& obj) = 0;
protected:
    ~ISceneVisitor() = default;
};

struct IResourceManager : util::basic_subscribable<IResourceManager, ResourceEvent>
{
    virtual auto SubscribeEvent(ResourceEvent type, void* data, void(*handler)()) -> handler_id = 0;
    virtual void UnsubscribeEvent(handler_id id) = 0;
    using util::basic_subscribable<IResourceManager, ResourceEvent>::SubscribeEvent;
protected:
    ~IResourceManager() = default;
};

class IMesh2 {
public:
    struct Vertex {
        math::vec3 pos;
        math::vec3 norm;
        math::vec3 tang;
        math::vec2 uv;
        float bTangSign;
    };
    enum class Draw {
        Triangles,
        Stripes
    };

    struct MeshParameters {
        std::uint32_t verticesCount;
        std::uint32_t elementsCount;
        std::uint32_t submeshCount;
    };
    struct SubmeshRange {
        std::uint32_t start;
        std::uint32_t end;
        Draw drawType;
    };
    virtual auto LoadMeshParameters(MeshParameters* parameters, util::function_ptr<void(Status)> callback) -> Status = 0;
    virtual auto LoadVertices(Vertex* vertexBuffer, util::function_ptr<void(Status)> callback) -> Status = 0;
    virtual auto LoadElements(std::uint32_t* elementBuffer, util::function_ptr<void(Status)> callback) -> Status = 0;
    virtual auto LoadSubmeshRanges(SubmeshRange* ranges, util::function_ptr<void(Status)> callback) -> Status = 0;
    virtual auto GetResourceManager() -> IResourceManager& = 0;
protected:
    ~IMesh2() = default;
};

struct IMaterial
{
    virtual auto GetColor() -> math::vec4 = 0;
    virtual auto GetDiffuseTexture() -> ITexture& = 0;
    virtual auto GetNormalMapTexture() -> ITexture& = 0;
    virtual auto GetResourceManager() -> IResourceManager& = 0;
protected:
    ~IMaterial() = default;
};

struct ITexture {
    struct TextureParameters {
        int width;
        int height;
        int depth;
        PixelFormat format;
        unsigned lodCount;
    };
    virtual auto LoadParameters(TextureParameters* parameters, util::function_ptr<void(Status)> onReady) -> Status = 0;
    virtual auto LoadData(void* recvBuffer, unsigned lod, util::function_ptr<void(Status)> onReady) -> Status = 0;
    virtual auto GetResourceManager() -> IResourceManager& = 0;
protected:
    ~ITexture() = default;
};

enum class PixelFormat {
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

class StaticResourceManager : public IResourceManager {
public:
    auto SubscribeEvent(ResourceEvent type, void *data, void (*handler)()) -> handler_id
    {
        return 1;
    }
    void UnsubscribeEvent(handler_id id)
    {}
    static StaticResourceManager instance;
};

template <class IBase, class Event>
struct embedded_event_manager : IBase
{
    using handler_id = typename IBase::handler_id;
    auto SubscribeEvent(Event type, void *data, void (*handler)()) -> handler_id override
    {
        return eventManager.subscribe(type, data, handler);
    }
    void UnsubscribeEvent(handler_id id) override
    {
        eventManager.unsubscribe(id);
    }
protected:
    util::event_manager<Event> eventManager;
};

inline StaticResourceManager StaticResourceManager::instance;

} // namespace dse::core

#endif // DSE_CORE_SCENE2_H
