#include "connection.hpp"
#include "connection_manager.hpp"

Connection::Connection(boost::asio::ip::tcp::socket socket,
                       ConnectionManager& manager):
    _socket(std::move(socket)),
    _connectionManager(manager)
{
}

void Connection::start() {
    doRead();
}

void Connection::stop() {
    _socket.close();
}


void Connection::doRead() {
    auto self(shared_from_this());
    _socket.async_read_some(boost::asio::buffer(_buffer),
        [this, self](boost::system::error_code error, std::size_t numBytes) {
            if(!error) {
                handleRead(numBytes);
            } else if(error != boost::asio::error::operation_aborted) {
                _connectionManager.stop(shared_from_this());
            }
        });
}

void Connection::writeBytes(std::vector<ubyte> bytes) {
    auto self(shared_from_this());
    boost::asio::async_write(_socket, boost::asio::buffer(bytes),
                             [this, self](boost::system::error_code error, std::size_t) {
                                 if(!error) {
                                     doRead();
                                 }
                             });
}

std::vector<ubyte> Connection::getBytes(std::size_t numBytes) {
    std::vector<ubyte> bytes(std::begin(_buffer), std::begin(_buffer) + numBytes);
    return bytes;
}
