#include "HttpServer.h"
#include "EventLoop.h"
#include "Log.h"
#include "TcpConnection.h"

std::string kNotFoundHtml =
    "<html>"
    "<head><title>404 Not Found</title></head>"
    "<body>"
    "<h1>Not Found</h1>"
    "<p>The requested URL was not found on this server.</p>"
    "</body>"
    "</html>";

HttpServer::HttpServer(EventLoop* loop, const Address& addr)
    : server_(loop, addr)
    , requestCallback_()
    , timeout_(10)
{
    using std::placeholders::_1, std::placeholders::_2;
    server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2));
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, _1));
}

HttpServer::~HttpServer() {}

void HttpServer::start()
{
    server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
    std::shared_ptr<TimingWheel::Entry> entryPtr =
        std::make_shared<TimingWheel::Entry>(conn);
    std::weak_ptr<TimingWheel::Entry> entryWeakPtr(entryPtr);
    getTimingWheel(conn->getLoop())->enqueue(entryWeakPtr);
    conn->setContext(entryWeakPtr);
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer& buffer)
{
    HttpRequest request;
    bool ok = request.parse(buffer);
    buffer.retrieveAll();
    if (requestCallback_ == nullptr || false == ok) {
        HttpResponse response;
        response.version = HttpResponse::Version::V_HTTP1_1;
        response.statusCode = HttpResponse::StatusCode::S_NOTFOUND;
        response.statusMessage = std::string("Not Found");
        response.headers["Connection"] = "keep-alive";
        response.headers["Keep-Alive"] =
            std::string("timeout=") + std::to_string(timeout_);
        response.headers["Content-Type"] = "text/html";
        response.body = kNotFoundHtml;
        conn->send(response.serialize());
        return;
    }

    HttpResponse response;
    requestCallback_(request, response);

    bool longConn = true;
    if (response.headers.find("Connection") == response.headers.end() ||
        response.headers["Connection"] == "close") {
        longConn = false;
    }
    if (longConn) {
        response.headers["Keep-Alive"] =
            std::string("timeout=") + std::to_string(timeout_);
        getTimingWheel(conn->getLoop())
            ->enqueue(std::any_cast<std::weak_ptr<TimingWheel::Entry>>(
                conn->getContext()));
    }
    conn->send(response.serialize());
    if (!longConn) {
        conn->shutdown();
    }
}

HttpServer::TimingWheel* HttpServer::getTimingWheel(EventLoop* loop) const
{
    static thread_local TimingWheel* t_timingWheel = nullptr;
    static thread_local class TimingWheelDestoryer {
    public:
        TimingWheelDestoryer() = default;
        ~TimingWheelDestoryer()
        {
            if (t_timingWheel) {
                delete t_timingWheel;
                t_timingWheel = nullptr;
            }
        }
        void active() {}
    } destoryer;

    if (t_timingWheel == nullptr) {
        t_timingWheel = new TimingWheel(loop, timeout_);
        destoryer.active();
    }
    return t_timingWheel;
}

HttpServer::TimingWheel::Entry::Entry(const TcpConnectionPtr& conn)
    : conn_(conn)
{
}

HttpServer::TimingWheel::Entry::~Entry()
{
    TcpConnectionPtr conn = conn_.lock();
    if (conn) {
        conn->shutdown();
    }
}

HttpServer::TimingWheel::TimingWheel(EventLoop* loop, size_t timeout)
    : loop_(loop)
    , wheel_(kBucketNum_)
    , tail_(0)
    , timeout_(timeout)
{
    assert(timeout <= kBucketNum_);
    loop_->runEvery(std::chrono::seconds(1),
                    std::bind(&TimingWheel::tick, this));
}

HttpServer::TimingWheel::~TimingWheel() {}

void HttpServer::TimingWheel::tick()
{
    ++tail_;
    wheel_[tail_ & (kBucketNum_ - 1)].clear();
}

void HttpServer::TimingWheel::enqueue(const std::weak_ptr<Entry>& entryWeakPtr)
{
    loop_->assertInOwningThread();
    std::shared_ptr<Entry> entryPtr = entryWeakPtr.lock();
    if (entryPtr == nullptr) {
        return;
    }
    wheel_[(tail_ + timeout_) & (kBucketNum_ - 1)].push_front(
        std::move(entryPtr));
}