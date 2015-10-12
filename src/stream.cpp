#include "stream.hpp"
#include "factory.hpp"
#include "Decerealiser.hpp"
#include <cassert>
#include <iostream>

MqttStream::MqttStream(ulong bufferSize):
    _buffer(bufferSize),
    _bytes(),
    _remaining(0),
    _bytesRead(0),
    _bytesStart(0)
{
}

bool MqttStream::hasMessages() const {
    return _bytes.size() >= static_cast<size_t>(_remaining + MqttFixedHeader::SIZE);
}

bool MqttStream::empty() const {
    return _bytes.size() == 0;
}


void MqttStream::read(MqttServer& server, MqttConnection& connection, const std::vector<ubyte>& bytes) {
    *this << bytes;

    while(hasMessages()) {
        auto msg = createMessage();
        if(!msg) {
            std::cerr << "null message after createMessage() even though hasMessages() was true" << std::endl;
        }
        assert(msg);

        msg->handle(server, connection);
    }
}

void MqttStream::operator<<(const std::vector<ubyte>& bytes) {
    checkRealloc(bytes.size());
    const auto end = _bytesRead + bytes.size();

    std::copy(bytes.cbegin(), bytes.cend(), _buffer.begin() + _bytesRead);

    _bytes = std::vector<ubyte>(_buffer.cbegin() + _bytesStart, _buffer.cbegin() + end);
    _bytesRead += bytes.size();
    updateRemaining();
}

void MqttStream::checkRealloc(ulong numBytes) {
    if(_bytesRead + numBytes > _buffer.size()) {
        copy(_bytes.cbegin(), _bytes.cend(), _buffer.begin());
        _bytesStart = 0;
        _bytesRead = _bytes.size();
        _bytes = std::vector<ubyte>(_buffer.cbegin(), _buffer.cbegin() + _bytesRead);
    }
}

void MqttStream::updateRemaining() {
    if(!_remaining && _bytes.size() >= MqttFixedHeader::SIZE) {
        Decerealiser cereal(slice());
        _remaining = cereal.value<MqttFixedHeader>().remaining;
    }
}

std::unique_ptr<MqttMessage> MqttStream::createMessage() {
    if(!hasMessages()) return nullptr;

    const auto theSlice = slice();
    _bytesStart += theSlice.size();
    _bytes = std::vector<ubyte>(_buffer.cbegin() + _bytesStart, _buffer.cbegin() + _bytesRead);

    auto msg = MqttFactory::create(theSlice);

    _remaining = 0; //reset
    updateRemaining();

    return msg;
}

std::vector<ubyte> MqttStream::slice() const {
    const auto msgSize = _remaining + MqttFixedHeader::SIZE;
    return std::vector<ubyte>(_bytes.cbegin(), _bytes.cbegin() + msgSize);
}
