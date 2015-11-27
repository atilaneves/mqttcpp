#ifndef DLANG_H_
#define DLANG_H_

#include "dtypes.hpp"
#include "gsl.h"

struct Span {
    ubyte* ptr;
    long size;
};

class CppConnection {
public:
    virtual void newMessage(Span bytes) = 0;
    virtual void disconnect() = 0;
};

class DlangSubscriber {
public:

    virtual Span getWriteableBuffer() = 0;
    virtual const char* handleMessages(long numBytesRead) = 0;
};

void startMqttServer(bool useCache);
DlangSubscriber* newDlangSubscriber(CppConnection* connection);


#endif // DLANG_H_
