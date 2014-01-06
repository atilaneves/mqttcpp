#ifndef MQTTTCPSERVER_H_
#define MQTTTCPSERVER_H_

#include <boost/asio.hpp>
#include "connection.hpp"
#include "connection_manager.hpp"


class MqttTcpServer {
public:

    MqttTcpServer(const MqttTcpServer&) = delete;
    MqttTcpServer& operator=(const MqttTcpServer&) = delete;

    explicit MqttTcpServer(int port1);

    void run();

private:

    boost::asio::io_service _ioService;
    boost::asio::signal_set _signals;
    boost::asio::ip::tcp::acceptor _acceptor;
    ConnectionManager _connectionManager;
    boost::asio::ip::tcp::socket _socket;

    void doAccept();
    void doAwaitStop();
};


#endif // SERVER_H_
