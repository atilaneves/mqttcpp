mqtt
=============
[![Build Status](https://travis-ci.org/atilaneves/mqttcpp.png?branch=master)](https://travis-ci.org/atilaneves/mqttcpp)

MQTT broker written in C++11, using
[boost::asio](http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio.html)
and [cereal](https://bitbucket.org/atilaneves/cereal), the latter having to be cloned into
the directory after `git clone` in order for this to compile.

Manually translated from [an MQTT broker written in D](https://github.com/atilaneves/mqtt)
since that was the easiest way I could get a broker going in C++.

Doesn't implement all of MQTT, the feature set is on par with the D version. Buggy, one
of the unit tests fails and running it for long enough will cause it to corrupt memory
and crash.
