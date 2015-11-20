#ifndef STREAM_H_
#define STREAM_H_


#include "dtypes.hpp"
#include "server.hpp"
#include "Decerealiser.hpp"
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

    MqttStream(int size):
        _buffer(size),
        _scratch(size),
        _begin{_buffer.begin()}
    {
    }

    auto begin() noexcept {
        return _begin;
    }

    template<typename C>
    void handleMessages(int numBytes, MqttServer<C>& server, C& connection) {
        auto slice = gsl::as_span(_buffer.data(), std::distance(_buffer.begin(), _begin) + numBytes);
        auto totLen = totalLength(slice);

        while(slice.length() >= totLen) {
            server.newMessage(connection, slice);
            slice = slice.sub(totLen);
            totLen = totalLength(slice);
        }

        //shift everything to the beginning
        copy(slice.cbegin(), slice.cend(), _scratch.begin());
        copy(_scratch.cbegin(), _scratch.cend(), _buffer.begin());
        _begin = _buffer.begin() + slice.size();
    }

private:

    std::vector<ubyte> _buffer;
    std::vector<ubyte> _scratch;
    decltype(_buffer)::iterator _begin;

    static int remainingLength(gsl::span<ubyte> bytes) noexcept {
        Decerealiser dec{bytes};
        return dec.create<MqttFixedHeader>().remaining;
    }

    static int totalLength(gsl::span<ubyte> bytes) noexcept {
        return bytes.size() > MqttFixedHeader::SIZE
            ? remainingLength(bytes) + MqttFixedHeader::SIZE
            : MqttFixedHeader::SIZE;
    }
};

#endif // STREAM_H_
