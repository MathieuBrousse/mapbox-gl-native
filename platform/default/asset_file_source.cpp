#include <mbgl/storage/asset_file_source.hpp>
#include <mbgl/storage/response.hpp>
#include <mbgl/util/string.hpp>
#include <mbgl/util/threaded_object.hpp>
#include <mbgl/util/url.hpp>
#include <mbgl/util/util.hpp>
#include <mbgl/util/io.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace mbgl {

class AssetFileSource::Impl {
public:
    Impl(ActorRef<Impl>, std::string root_)
        : root(std::move(root_)) {
    }

    void request(const std::string& url, FileSource::Callback callback) {
        std::string path;

        if (url.size() <= 8 || url[8] == '/') {
            // This is an empty or absolute path.
            path = mbgl::util::percentDecode(url.substr(8));
        } else {
            // This is a relative path. Prefix with the application root.
            path = root + "/" + mbgl::util::percentDecode(url.substr(8));
        }

        Response response;

        struct stat buf;
        int result = stat(path.c_str(), &buf);

        if (result == 0 && S_ISDIR(buf.st_mode)) {
            response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound);
        } else if (result == -1 && errno == ENOENT) {
            response.error = std::make_unique<Response::Error>(Response::Error::Reason::NotFound);
        } else {
            try {
                response.data = std::make_shared<std::string>(util::read_file(path));
            } catch (...) {
                response.error = std::make_unique<Response::Error>(
                    Response::Error::Reason::Other,
                    util::toString(std::current_exception()));
            }
        }

        callback(response);
    }

private:
    std::string root;
};

AssetFileSource::AssetFileSource(const std::string& root)
    : impl(std::make_unique<util::ThreadedObject<Impl>>("AssetFileSource", root))
    , thread(std::make_unique<ActorRef<Impl>>(impl->actor())) {
}

AssetFileSource::~AssetFileSource() = default;

std::unique_ptr<AsyncRequest> AssetFileSource::request(const Resource& resource, Callback callback) {
    class AssetFileRequest : public AsyncRequest {
    public:
        AssetFileRequest(Resource resource_, FileSource::Callback callback_, ActorRef<AssetFileSource::Impl> fs_)
            : mailbox(std::make_shared<Mailbox>(*util::RunLoop::Get()))
            , fs(fs_) {
            fs.invoke(&AssetFileSource::Impl::request, resource_.url, [callback_, ref = ActorRef<AssetFileRequest>(*this, mailbox)](Response res) mutable {
                ref.invoke(&AssetFileRequest::runCallback, callback_, res);
            });
        }

        void runCallback(FileSource::Callback callback_, Response res) {
            callback_(res);
        }

    private:
        std::shared_ptr<Mailbox> mailbox;
        ActorRef<AssetFileSource::Impl> fs;
    };

    return std::make_unique<AssetFileRequest>(resource, callback, *thread);
}

} // namespace mbgl
