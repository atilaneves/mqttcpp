#include "stream.hpp"
#include "factory.hpp"
#include "Decerealiser.hpp"
#include <cassert>

MqttStream::MqttStream(ulong bufferSize) {
    allocate(bufferSize);
}

void MqttStream::operator<<(std::vector<ubyte> bytes) {
    checkRealloc(bytes.size());
    const auto end = _bytesRead + bytes.size();

    std::vector<ubyte> buf;
    std::copy(bytes.cbegin(), bytes.cend(), std::back_inserter(buf));

    _bytes = std::vector<ubyte>(_buffer.cbegin() + _bytesStart, _buffer.cbegin() + end);
    _bytesRead += bytes.size();
    updateRemaining();
}

void MqttStream::read(MqttServer& server, MqttConnection& connection, ulong size) {
    checkRealloc(size);
    const auto end = _bytesRead + size;

    connection.read(std::vector<ubyte>(_buffer.cbegin() + _bytesRead, _buffer.cbegin() + end));

    _bytes = std::vector<ubyte>(_buffer.cbegin() + _bytesStart, _buffer.cbegin() + end);
    _bytesRead += size;
    updateRemaining();

    while(hasMessages()) {
        createMessage()->handle(server, connection);
    }
}

void MqttStream::read(std::vector<ubyte> bytes) {
    (void)bytes;
}

bool MqttStream::hasMessages() const {
    return _bytes.size() >= static_cast<size_t>(_remaining + MqttFixedHeader::SIZE);
}

bool MqttStream::empty() const {
    return _bytes.size() == 0;
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

void MqttStream::allocate(ulong bufferSize) {
    assert(bufferSize > 10);
    _buffer = std::vector<ubyte>(bufferSize);
}

void MqttStream::checkRealloc(ulong numBytes) {
    if(!_buffer.size()) {
        allocate();
    }

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

std::vector<ubyte> MqttStream::slice() const {
    const auto msgSize = _remaining + MqttFixedHeader::SIZE;
    return std::vector<ubyte>(_bytes.cbegin(), _bytes.cbegin() + msgSize);
}
