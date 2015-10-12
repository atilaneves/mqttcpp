#ifndef STREAM_H_
#define STREAM_H_


#include "dtypes.hpp"
#include "server.hpp"
#include <vector>
#include <memory>


class MqttStream {
public:
    explicit MqttStream(ulong bufferSize);

    void operator<<(const std::vector<ubyte>& bytes);
    void read(MqttServer& server, MqttConnection& connection, const std::vector<ubyte>& bytes);
    bool hasMessages() const;
    bool empty() const;
    std::unique_ptr<MqttMessage> createMessage();

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


#endif // STREAM_H_
