mqtt
=============

MQTT broker written in C++11, using
[boost::asio](http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio.html)
and [cereal](https://bitbucket.org/atilaneves/cereal), the latter having to be cloned into
the directory after `git clone` in order for this to compile.

Manually translated from [an MQTT broker written in D](https://github.com/atilaneves/mqtt)
since that was the easiest way I could get a broker going in C++.

Doesn't yet implement MQTT, work in progress.
