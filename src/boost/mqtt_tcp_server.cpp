#include "mqtt_tcp_server.hpp"
#include "stream.hpp"
#include <signal.h>

using boost::asio::ip::tcp;

MqttTcpServer::MqttTcpServer(int port):
    _ioService(),
    _signals(_ioService),
    _acceptor(_ioService, tcp::endpoint(tcp::v4(), port)),
    _connectionManager(),
    _socket(_ioService)
{

    _signals.add(SIGINT);
    _signals.add(SIGTERM);

#ifdef SIGQUIT
    _signals.add(SIGQUIT);
#endif

    doAwaitStop();
    doAccept();
}

void MqttTcpServer::run() {
    _ioService.run();
}

void MqttTcpServer::doAwaitStop() {
    _signals.async_wait([this](boost::system::error_code, int) {
            _acceptor.close();
            _connectionManager.stopAll();
        });
}

namespace {
class MqttTcpConnection: public Connection, public MqttConnection {
public:
    MqttTcpConnection(const MqttTcpConnection&) = delete;
    MqttTcpConnection& operator=(const MqttTcpConnection&) = delete;

    enum { BUFFER_SIZE = 16384 };
    explicit MqttTcpConnection(boost::asio::ip::tcp::socket socket,
                               ConnectionManager& manager,
                               MqttServer& server):
        Connection(std::move(socket), manager),
        _connected(true),
        _mqttServer(server),
        _stream(BUFFER_SIZE) {

    }

    virtual void handleRead(std::vector<ubyte> bytes) override {
        if(_connected) _stream.read(_mqttServer, *this, bytes);
    }

    virtual void write(std::vector<ubyte> bytes) override {
        if(_connected) writeBytes(bytes);
    }

    virtual void disconnect() override {
        _connected = false;
        stop();
    }

private:

    bool _connected;
    MqttServer& _mqttServer;
    MqttStream _stream;

};
} //anonymous namespace

void MqttTcpServer::doAccept() {
    _acceptor.async_accept(_socket,
                           [this](boost::system::error_code error) {
                               if(!_acceptor.is_open()) return;
                               if(!error) {
                                   _connectionManager.start(std::make_shared<MqttTcpConnection>(
                                                                std::move(_socket),
                                                                _connectionManager,
                                                                _mqttServer));
                               }

                               doAccept();
                           });
}
