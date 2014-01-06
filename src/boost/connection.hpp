#ifndef CONNECTION_H_
#define CONNECTION_H_

#include <array>
#include <boost/asio.hpp>


class ConnectionManager;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    explicit Connection(boost::asio::ip::tcp::socket socket,
                        ConnectionManager& manager);

    void start();
    void stop();

private:

    boost::asio::ip::tcp::socket _socket;
    ConnectionManager& _connectionManager;
    std::array<char, 16384> _buffer;

    void doRead();
    void doWrite(std::size_t numBytes);
};

typedef std::shared_ptr<Connection> ConnectionPtr;


#endif // CONNECTION_H_
