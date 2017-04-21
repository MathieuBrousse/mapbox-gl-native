#pragma once

#include <mbgl/style/style.hpp>
#include <mbgl/style/source.hpp>

#include "source.hpp"

#include <jni/jni.hpp>

namespace mbgl {
namespace android {

    mbgl::android::Source* initializeSourcePeer(mbgl::style::Style&, mbgl::style::Source&);

    jni::jobject* createJavaSourcePeer(jni::JNIEnv&, mbgl::style::Style&, mbgl::style::Source&);

    void registerNativeSources(jni::JNIEnv&);

}
}
