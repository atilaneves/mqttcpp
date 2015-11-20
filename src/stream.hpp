#ifndef STREAM_H_
#define STREAM_H_


#include "dtypes.hpp"
#include "server.hpp"
#include "Decerealiser.hpp"
#include <vector>
#include <memory>
#include <cassert>


class MqttStream {
public:

    MqttStream(int size):
        _buffer(size),
        _scratch(size),
        _begin{_buffer.begin()}
    {
    }

    auto begin() noexcept { return _begin; }
    auto readableData() noexcept { return _buffer.data() + std::distance(_buffer.begin(), _begin); }
    auto readableDataSize() noexcept { return std::distance(_begin, _buffer.end()); }

    template<typename C>
    void handleMessages(int numBytes, MqttServer<C>& server, C& connection) {

        auto slice = gsl::as_span(_buffer.data(), std::distance(_buffer.begin(), _begin) + numBytes);
        auto totLen = totalLength(slice);

        assert(totLen > 0);

        while(slice.size() >= totLen) {
            const auto msg = slice.sub(0, totLen);

            slice = slice.sub(totLen);

            server.newMessage(connection, msg);
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
        return bytes.size() >= MqttFixedHeader::SIZE
            ? remainingLength(bytes) + MqttFixedHeader::SIZE
            : MqttFixedHeader::SIZE;
    }
};

#endif // STREAM_H_
