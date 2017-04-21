#include "layers.hpp"

#include <mbgl/style/layer.hpp>
#include <mbgl/style/layers/background_layer.hpp>
#include <mbgl/style/layers/circle_layer.hpp>
#include <mbgl/style/layers/fill_extrusion_layer.hpp>
#include <mbgl/style/layers/fill_layer.hpp>
#include <mbgl/style/layers/line_layer.hpp>
#include <mbgl/style/layers/raster_layer.hpp>
#include <mbgl/style/layers/symbol_layer.hpp>
#include <mbgl/style/layers/custom_layer.hpp>

#include "background_layer.hpp"
#include "circle_layer.hpp"
#include "custom_layer.hpp"
#include "fill_extrusion_layer.hpp"
#include "fill_layer.hpp"
#include "line_layer.hpp"
#include "raster_layer.hpp"
#include "symbol_layer.hpp"
#include "unknown_layer.hpp"
#include "fill_extrusion_layer.hpp"

namespace mbgl {
namespace android {

// Mapping from style layers to peer classes
template <class> struct PeerType {};
template <> struct PeerType<style::BackgroundLayer> { using Type = android::BackgroundLayer; };
template <> struct PeerType<style::CircleLayer> { using Type = android::CircleLayer; };
template <> struct PeerType<style::FillExtrusionLayer> { using Type = android::FillExtrusionLayer; };
template <> struct PeerType<style::FillLayer> { using Type = android::FillLayer; };
template <> struct PeerType<style::LineLayer> { using Type = android::LineLayer; };
template <> struct PeerType<style::RasterLayer> { using Type = android::RasterLayer; };
template <> struct PeerType<style::SymbolLayer> { using Type = android::SymbolLayer; };
template <> struct PeerType<style::CustomLayer> { using Type = android::CustomLayer; };

// Inititalizes a non-owning peer
struct LayerPeerIntitializer {
    mbgl::style::Style& style;

    template <class LayerType>
    Layer* operator()(LayerType& layer) {
        return new typename PeerType<LayerType>::Type(style, layer);
    }
};

static Layer* initializeLayerPeer(mbgl::style::Style& style, mbgl::style::Layer& coreLayer) {
    Layer* layer = coreLayer.accept(LayerPeerIntitializer {style});
    return layer ? layer : new UnknownLayer(style, coreLayer);
}

// Initializes an owning peer
// Only usable once since it needs to pass on ownership
// of the given layer and thus enforced to be an rvalue
struct UniqueLayerPeerIntitializer {
    mbgl::style::Style& style;
    std::unique_ptr<style::Layer> layer;

    template <class LayerType>
    Layer* operator()(LayerType&) && {
        return new typename PeerType<LayerType>::Type(
                style,
                std::unique_ptr<LayerType>(layer.release()->as<LayerType>())
        );
    }
};

static Layer* initializeLayerPeer(mbgl::style::Style& style, std::unique_ptr<mbgl::style::Layer> coreLayer) {
    Layer* layer = coreLayer->accept(UniqueLayerPeerIntitializer {style, std::move(coreLayer)});
    return layer ? layer : new UnknownLayer(style, std::move(coreLayer));
}

jni::jobject* createJavaLayerPeer(jni::JNIEnv& env, mbgl::style::Style& style, style::Layer& coreLayer) {
    std::unique_ptr<Layer> peerLayer = std::unique_ptr<Layer>(initializeLayerPeer(style, coreLayer));
    jni::jobject* result = peerLayer->createJavaPeer(env);
    peerLayer.release();
    return result;
}

jni::jobject* createJavaLayerPeer(jni::JNIEnv& env, mbgl::style::Style& style, std::unique_ptr<mbgl::style::Layer> coreLayer) {
    std::unique_ptr<Layer> peerLayer = std::unique_ptr<Layer>(initializeLayerPeer(style, std::move(coreLayer)));
    jni::jobject* result = peerLayer->createJavaPeer(env);
    peerLayer.release();
    return result;
}

void registerNativeLayers(jni::JNIEnv& env) {
    Layer::registerNative(env);
    BackgroundLayer::registerNative(env);
    CircleLayer::registerNative(env);
    CustomLayer::registerNative(env);
    FillExtrusionLayer::registerNative(env);
    FillLayer::registerNative(env);
    LineLayer::registerNative(env);
    RasterLayer::registerNative(env);
    SymbolLayer::registerNative(env);
    UnknownLayer::registerNative(env);
}

} // namespace android
} // namespace mbgl
