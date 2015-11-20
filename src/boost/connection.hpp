#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "dtypes.hpp"
#include "server.hpp"
#include "stream.hpp"
#include <boost/asio.hpp>


class ConnectionManager;


class Connection : public std::enable_shared_from_this<Connection> {
public:

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(boost::asio::ip::tcp::socket socket,
               ConnectionManager& manager,
               MqttServer<Connection>& server,
               int numStreamBytes);

    void start();
    void stop();

    void newMessage(gsl::span<const ubyte> bytes);
    void disconnect() noexcept { connected = false; }


private:

    boost::asio::ip::tcp::socket _socket;
    ConnectionManager& _connectionManager;
    MqttServer<Connection>& _server;
    bool connected{true};
    MqttStream _stream;
    std::vector<ubyte> _buffer;

    void doRead();
};

typedef std::shared_ptr<Connection> ConnectionPtr;


#endif // CONNECTION_H_
