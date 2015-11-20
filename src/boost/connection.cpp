#include "connection.hpp"
#include "connection_manager.hpp"

using namespace std;
using namespace gsl;


Connection::Connection(boost::asio::ip::tcp::socket socket,
                       ConnectionManager& manager,
                       MqttServer<Connection>& server,
                       int numStreamBytes):
    _socket(std::move(socket)),
    _connectionManager(manager),
    _server{server},
    _stream{numStreamBytes}
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
    if(!_connected) return;
    _socket.async_read_some(boost::asio::buffer(_stream.readableData(), _stream.readableDataSize()),
        [this, self](boost::system::error_code error, std::size_t numBytes) {
            if(!error) {
                _stream.handleMessages(numBytes, _server, *this);
                doRead();
            } else if(error != boost::asio::error::operation_aborted) {
                _connectionManager.stop(shared_from_this());
            } else {
                cerr << "Error reading from TCP socket" << endl;
            }
        });
}

void Connection::newMessage(span<const ubyte> bytes) {
    auto self(shared_from_this());
    if(!_connected) return;

    boost::asio::async_write(_socket, boost::asio::buffer(bytes.data(), bytes.size()),
                             [this, self](boost::system::error_code, std::size_t) {
                             });
}


void Connection::disconnect() {
    auto self(shared_from_this());

    _connected = false;
    _connectionManager.stop(self);
}
