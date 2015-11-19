#include "stream.hpp"
#include "factory.hpp"
#include "Decerealiser.hpp"
#include <cassert>
#include <iostream>

OldMqttStream::OldMqttStream(ulong bufferSize):
    _buffer(bufferSize),
    _bytes(),
    _remaining(0),
    _bytesRead(0),
    _bytesStart(0)
{
}

bool OldMqttStream::hasMessages() const {
    return _bytes.size() >= static_cast<size_t>(_remaining + MqttFixedHeader::SIZE);
}

bool OldMqttStream::empty() const {
    return _bytes.size() == 0;
}


void OldMqttStream::read(OldMqttServer& server, OldMqttConnection& connection, const std::vector<ubyte>& bytes) {
    *this << bytes;

    while(hasMessages()) {
        handleMessage(server, connection);
    }
}

void OldMqttStream::operator<<(const std::vector<ubyte>& bytes) {
    checkRealloc(bytes.size());
    const auto end = _bytesRead + bytes.size();

    std::copy(bytes.cbegin(), bytes.cend(), _buffer.begin() + _bytesRead);

    _bytes = std::vector<ubyte>(_buffer.cbegin() + _bytesStart, _buffer.cbegin() + end);
    _bytesRead += bytes.size();
    updateRemaining();
}

void OldMqttStream::checkRealloc(ulong numBytes) {
    if(_bytesRead + numBytes > _buffer.size()) {
        copy(_bytes.cbegin(), _bytes.cend(), _buffer.begin());
        _bytesStart = 0;
        _bytesRead = _bytes.size();
        _bytes = std::vector<ubyte>(_buffer.cbegin(), _buffer.cbegin() + _bytesRead);
    }
}

void OldMqttStream::updateRemaining() {
    if(!_remaining && _bytes.size() >= MqttFixedHeader::SIZE) {
        Decerealiser cereal(slice());
        _remaining = cereal.value<MqttFixedHeader>().remaining;
    }
}

std::unique_ptr<MqttMessage> OldMqttStream::createMessage() {
    if(!hasMessages()) return nullptr;

    const auto theSlice = slice();
    _bytesStart += theSlice.size();
    _bytes = std::vector<ubyte>(_buffer.cbegin() + _bytesStart, _buffer.cbegin() + _bytesRead);

    auto msg = MqttFactory::create(theSlice);

    _remaining = 0; //reset
    updateRemaining();

    return msg;
}

void OldMqttStream::handleMessage(OldMqttServer& server, OldMqttConnection& connection) {
    if(!hasMessages()) return;

    const auto msgSize = _remaining + MqttFixedHeader::SIZE;
    _bytesStart += msgSize;
    auto begin = _bytes.cbegin();
    auto end = _bytes.cbegin() + msgSize;
    MqttFactory::handleMessage(begin, end, server, connection);
    _bytes = std::vector<ubyte>(_buffer.cbegin() + _bytesStart, _buffer.cbegin() + _bytesRead);


    _remaining = 0; //reset
    updateRemaining();
}


std::vector<ubyte> OldMqttStream::slice() const {
    const auto msgSize = _remaining + MqttFixedHeader::SIZE;
    return std::vector<ubyte>(_bytes.cbegin(), _bytes.cbegin() + msgSize);
}
