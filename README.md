mqtt
=============

MQTT broker written in C++11, using boost::asio.

Doesn't yet implement all of MQTT. There is no authenticatin nor QOS levels other than 0.
It can be used for testing however and does correctly subscribe, unsubscribe and
dispatches messages.

Running the executable makes the server listen on port 1883.