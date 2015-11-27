module mqttd.stream;

import mqttd.server;
import mqttd.message;
import mqttd.broker;
import cerealed.decerealiser;
import std.stdio;
import std.conv;
import std.algorithm;
import std.exception;

version(Win32) {
    alias unsigned = uint;
} else {
    alias unsigned = ulong;
}


enum isMqttInput(T) = is(typeof(() {
    ubyte[] bytes;
    auto t = T.init;
    t.read(bytes);
}));

@safe:


struct MqttStream {

    this(int bufferSize) pure nothrow {
        _buffer = new ubyte[bufferSize];
        _bytes = _buffer[0..0];
    }

    void handleMessages(T)(long numBytes, ref MqttServer!T server, ref T connection) @trusted if(isMqttSubscriber!T) {
        auto slice = _buffer[0 .. _bytesRead + numBytes];
        auto totLen = totalLength(slice);

        while(slice.length >= totLen) {
            const auto msg = slice[0..totLen];
            slice = slice[totLen..$];
            server.newMessage(connection, msg);
            totLen = totalLength(slice);
        }

        //shift everything to the beginning
        //it's okay to overlap in practice
        copy(slice, _buffer);
        _bytesRead = slice.length;
    }

    ubyte[] buffer() {
        return _buffer[_bytesRead .. $];
    }

private:

    ubyte[] _buffer; //the underlying storage
    ubyte[] _bytes; //the current bytes held
    int _lastMessageSize;
    int _bytesStart; //the starting position
    ulong _bytesRead; //what it says

    static int totalLength(in ubyte[] bytes) {
        if(bytes.length < MqttFixedHeader.SIZE) return MqttFixedHeader.SIZE;
        auto dec = Decerealiser(bytes);
        return dec.value!MqttFixedHeader.remaining + MqttFixedHeader.SIZE;
    }

}
