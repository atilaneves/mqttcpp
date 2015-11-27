#ifndef DLANG_H_
#define DLANG_H_

#include "dtypes.hpp"
#include "gsl.h"

struct Span {
    ubyte* ptr;
    long size;
};

class DlangSubscriber {
public:

    virtual void newMessage(Span bytes) = 0;
    virtual void disconnect() = 0;
};

//doesn't have to have C linkage but it's easier for other languages
extern "C" {
    void startMqttServer(bool useCache);
    Span getWriteableBuffer();
    void handleMessages(long numBytesRead, DlangSubscriber& subscriber);
}


#endif // DLANG_H_
