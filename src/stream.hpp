#ifndef STREAM_H_
#define STREAM_H_


#include "dtypes.hpp"
#include "server.hpp"
#include <vector>
#include <memory>


class OldMqttStream {
public:
    explicit OldMqttStream(ulong bufferSize);

    void operator<<(const std::vector<ubyte>& bytes);
    void read(OldMqttServer& server, OldMqttConnection& connection, const std::vector<ubyte>& bytes);
    bool hasMessages() const;
    bool empty() const;
    std::unique_ptr<MqttMessage> createMessage();
    void handleMessage(OldMqttServer& server, OldMqttConnection& connection);

private:

    std::vector<ubyte> _buffer;
    std::vector<ubyte> _bytes;
    int _remaining;
    ulong _bytesRead;
    ulong _bytesStart;

    void allocate(ulong bufferSize = 128);
    void checkRealloc(ulong numBytes);
    void updateRemaining();
    std::vector<ubyte> slice() const;
};


class MqttStream {
public:

    MqttStream(int) {
    }

    void operator<<(gsl::span<const ubyte>) {
    }

    template<typename C>
    void handleMessages(MqttServer<C>&, C&) {
    }
};

#endif // STREAM_H_
