#include "mqtt_tcp_server.hpp"
#include "stream.hpp"
#include <signal.h>

using boost::asio::ip::tcp;

using namespace gsl;

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


void MqttTcpServer::doAccept() {
    _acceptor.async_accept(_socket,
                           [this](boost::system::error_code error) {
                               if(!_acceptor.is_open()) return;
                               if(!error) {
                                   _connectionManager.start(
                                       std::make_shared<Connection>(
                                           std::move(_socket),
                                           _connectionManager,
                                           128 * 1024));
                               }

                               doAccept();
                           });
}
