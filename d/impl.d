import mqttd.server;
import mqttd.stream;
import std.typecons;
import std.string;
import core.memory;


struct Span {
    ubyte* ptr;
    long size;
}

extern(C++) {

    interface CppConnection {
        void newMessage(const Span bytes);
        void disconnect();
    }

    interface DlangSubscriber {
        Span getWriteableBuffer();
        const(char)* handleMessages(long numBytesRead);
    }

    void startMqttServer(bool useCache) {
        GC.disable;
        gServer = typeof(gServer)(useCache ? Yes.useCache : No.useCache);
    }

    DlangSubscriber newDlangSubscriber(CppConnection connection) {
        return new Subscriber(connection);
    }
}

private inout(Span) arrayToSpan(inout(ubyte)[] bytes) {
    return inout(Span)(cast(inout(ubyte)*)bytes.ptr, bytes.length);
}

private class Subscriber: DlangSubscriber {
    this(CppConnection connection) {
        _stream = MqttStream(512 * 1024);
        _subscriber = SubscriberImpl(connection);
    }

    static struct SubscriberImpl {

        void newMessage(in ubyte[] bytes) {
            assert(bytes.length > 0);
            _cppConnection.newMessage(arrayToSpan(bytes));
        }

        void disconnect() {
            _cppConnection.disconnect();
        }

        CppConnection _cppConnection;
    }

    extern(C++) {
        Span getWriteableBuffer() {
            return arrayToSpan(_stream.buffer());
        }

        const(char)* handleMessages(long numBytesRead) {
            try {
                _stream.handleMessages(numBytesRead, gServer, _subscriber);
                return null;
            } catch(Throwable t) {
                return t.msg.toStringz();
            }
        }
    }

private:

    MqttStream _stream;
    SubscriberImpl _subscriber;
}


private __gshared MqttServer!(Subscriber.SubscriberImpl) gServer;
