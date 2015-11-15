#include "catch.hpp"
#include "broker.hpp"
#include "span.h"
#include <vector>

using namespace std;
using namespace gsl;


struct TestMqttSubscriber {
    using Payload = vector<ubyte>;

    void newMessage(span<ubyte>) {
    }
};

TEST_CASE("foo") {
    REQUIRE(3 == 4);
}
