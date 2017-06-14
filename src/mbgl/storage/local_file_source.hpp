#pragma once

#include <mbgl/storage/file_source.hpp>

namespace mbgl {

namespace util {
template <typename T> class ThreadedObject;
} // namespace util

template <typename T> class ActorRef;

class LocalFileSource : public FileSource {
public:
    LocalFileSource();
    ~LocalFileSource() override;

    std::unique_ptr<AsyncRequest> request(const Resource&, Callback) override;

    static bool acceptsURL(const std::string& url);

private:
    class Impl;

    std::unique_ptr<util::ThreadedObject<Impl>> impl;
    std::unique_ptr<ActorRef<Impl>> thread;
};

} // namespace mbgl
