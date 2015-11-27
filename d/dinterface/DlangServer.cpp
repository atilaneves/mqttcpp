#include "DlangServer.hpp"
#include "gsl.h"
#include <iostream>


using namespace std;
using namespace gsl;
using boost::asio::ip::tcp;


DlangServer::DlangServer(int port):
    _ioService(),
    _signals(_ioService),
    _acceptor(_ioService, tcp::endpoint(tcp::v4(), port)),
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

void DlangServer::run() {
    _ioService.run();
}

void DlangServer::doAwaitStop() {
    _signals.async_wait([this](boost::system::error_code, int) {
            _acceptor.close();
        });
}


void DlangServer::doAccept() {
    _acceptor.async_accept(_socket,
                           [this](boost::system::error_code error) {
                               if(!_acceptor.is_open()) return;
                               if(!error) {
                                   _connections.emplace_back(make_shared<DlangConnection>(move(_socket)));
                               }

                               doAccept();
                           });
}


DlangConnection::DlangConnection(boost::asio::ip::tcp::socket&& socket):
    _socket{std::move(socket)}
{
}

void DlangConnection::doRead() {
    if(!_connected) return;

    auto self(shared_from_this());
    const auto buffer = getWriteableBuffer();
    _socket.async_read_some(boost::asio::buffer(buffer.ptr, buffer.size),
        [this, self](boost::system::error_code error, std::size_t numBytes) {
            if(!error) {
                handleMessages(numBytes, *this);
                doRead();
            } else if(error != boost::asio::error::operation_aborted) {
                stop();
            } else {
                cerr << "Error: Couldn't read from TCP socket" << endl;
            }
        });
}

void DlangConnection::newMessage(Span bytes) {
    if(!_connected) return;

    auto self(shared_from_this());
    boost::asio::async_write(_socket, boost::asio::buffer(bytes.ptr, bytes.size),
                             [this, self](boost::system::error_code, std::size_t) {
                             });
}


void DlangConnection::disconnect() {
    _connected = false;
    stop();
}
