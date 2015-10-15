#ifndef CONNECTION_H_
#define CONNECTION_H_

#include "dtypes.hpp"
#include <array>
#include <boost/asio.hpp>

class ConnectionManager;


using ConnectionBytes = std::array<unsigned char, 1024 * 64>;
using ConnectionIterator = ConnectionBytes::const_iterator;

class BytesRange {
public:

    using const_iterator = ConnectionIterator;

    ConnectionIterator begin() const { return _begin; }
    ConnectionIterator end() const { return _end; }

private:

    ConnectionIterator _begin;
    ConnectionIterator _end;
};


class Connection : public std::enable_shared_from_this<Connection> {
public:

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    explicit Connection(boost::asio::ip::tcp::socket socket,
                        ConnectionManager& manager);

    void start();
    void stop();
    void writeBytes(const std::vector<ubyte>& bytes);

protected:

    std::vector<ubyte> getBytes(std::size_t numBytes) const;

private:

    boost::asio::ip::tcp::socket _socket;
    ConnectionManager& _connectionManager;
    ConnectionBytes _buffer;

    void doRead();
    virtual void handleRead(const std::vector<ubyte>& bytes) = 0;
};

typedef std::shared_ptr<Connection> ConnectionPtr;


#endif // CONNECTION_H_
