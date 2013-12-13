mqtt
=============

MQTT broker written in C++11, using boost::asio and
[cereal](https://bitbucket.org/atilaneves/cereal), the latter having to be cloned into
the directory after `git clone` in order for this to compile.

Manually translated from [an MQTT broker written in D](https://github.com/atilaneves/mqtt)
since that was the easiest way I could get a broker going in C++.

Doesn't yet implement MQTT, work in progress.

Running the executable makes the server listen on port 1883.