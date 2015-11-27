#ifndef DLANGSERVER_H_
#define DLANGSERVER_H_


#include <boost/asio.hpp>
#include <vector>
#include <memory>
#include "dtypes.hpp"
#include "gsl.h"
#include "dlang.hpp"


class DlangConnection: public std::enable_shared_from_this<DlangConnection>, public DlangSubscriber {
public:

    DlangConnection(boost::asio::ip::tcp::socket&& socket);
    void newMessage(Span bytes) override ;
    void disconnect() override;

    void stop() { _socket.close(); }

private:

    boost::asio::ip::tcp::socket _socket;
    bool _connected{true};

    void doRead();

};

class DlangServer {
public:

    DlangServer(const DlangServer&) = delete;
    DlangServer& operator=(const DlangServer&) = delete;

    DlangServer(int port = 1883);

    void run();

private:

    boost::asio::io_service _ioService;

    boost::asio::signal_set _signals;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _socket;
    std::vector<std::shared_ptr<DlangConnection>> _connections;

    void doAccept();
    void doAwaitStop();
};





#endif // DLANGSERVER_H_
