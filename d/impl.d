struct Span {
    ubyte* ptr;
    long size;
}

interface DlangSubscriber {
    void newMessage(Span bytes);
    void disconnect();
}

//doesn't have to have C linkage but it's easier for other languages
extern(C) {
    void startMqttServer(bool useCache) {
    }

    Span getWriteableBuffer() {
        return Span();
    }

    void handleMessages(long numBytesRead, DlangSubscriber subscriber) {

    }
}
