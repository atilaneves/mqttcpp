#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "dtypes.hpp"
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
    void writeBytes(std::vector<ubyte> bytes);

protected:

    std::vector<ubyte> getBytes(std::size_t numBytes);

private:

    boost::asio::ip::tcp::socket _socket;
    ConnectionManager& _connectionManager;
    std::array<char, 16384> _buffer;

    void doRead();
    virtual void handleRead(std::size_t numBytes) = 0;
};

typedef std::shared_ptr<Connection> ConnectionPtr;


#endif // CONNECTION_H_
