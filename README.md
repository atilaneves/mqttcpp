mqtt
=============

MQTT broker written in C++11, using boost::asio and
[cereal](https://bitbucket.org/atilaneves/cereal), the latter having to be cloned into
the directory after `git clone` in order for this to compile.

Doesn't yet implement all of MQTT. There is no authentication nor QOS levels other than 0.
It can be used for testing however and does correctly subscribe, unsubscribe and
dispatches messages.

Running the executable makes the server listen on port 1883.