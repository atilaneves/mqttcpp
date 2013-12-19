#include "broker.hpp"
#include "message.hpp"
#include <algorithm>


// struct MqttBroker {
// public:

//     void subscribe(MqttSubscriber subscriber, in string[] topics) {
//         subscribe(subscriber, array(map!(a => MqttSubscribe.Topic(a, 0))(topics)));
//     }

//     void subscribe(MqttSubscriber subscriber, in MqttSubscribe.Topic[] topics) {
//         foreach(t; topics) {
//             const parts = array(splitter(t.topic, "/"));
//             _subscriptions.addSubscription(Subscription(subscriber, t, parts), parts);
//         }
//     }

//     void unsubscribe(MqttSubscriber subscriber) {
//         _subscriptions.removeSubscription(subscriber, _subscriptions._nodes);
//     }

//     void unsubscribe(MqttSubscriber subscriber, in string[] topics) {
//         _subscriptions.removeSubscription(subscriber, topics, _subscriptions._nodes);
//     }

//     void publish(in string topic, in string payload) {
//         publish(topic, cast(ubyte[])payload);
//     }

//     void publish(in string topic, in ubyte[] payload) {
//         auto topParts = array(splitter(topic, "/"));
//         publish(topic, topParts, payload);
//     }

//     @property void useCache(bool u) {
//         _subscriptions._useCache = u;
//     }

// private:

//     SubscriptionTree _subscriptions;

//     void publish(in string topic, string[] topParts, in ubyte[] payload) {
//         _subscriptions.publish(topic, topParts, payload);
//     }
// };


namespace {

bool revStrEquals(const std::string& str1, const std::string& str2) { //compare strings in reverse
    if(str1.length() != str2.length()) return false;
    for(int i = str1.length() - 1; i >= 0; --i)
        if(str1[i] != str2[i]) return false;
    return true;
}

bool equalOrPlus(const std::string& pat, const std::string& top) {
    return pat == "+" || revStrEquals(pat, top);
}


// private struct SubscriptionTree {
//     private static struct Node {
//         string part;
//         Node* parent;
//         Node*[string] branches;
//         Subscription[] leaves;
//     }

//     void addSubscription(Subscription s, in string[] parts) {
//         assert(parts.length);
//         if(_useCache) _cache = _cache.init; //invalidate cache
//         addSubscriptionImpl(s, parts, null, _nodes);
//     }

//     void addSubscriptionImpl(Subscription s, const(string)[] parts,
//                              Node* parent, ref Node*[string] nodes) {
//         auto part = parts[0];
//         parts = parts[1 .. $];
//         auto node = addOrFindNode(s, part, parent, nodes);
//         if(parts.empty) {
//             node.leaves ~= s;
//         } else {
//             addSubscriptionImpl(s, parts, node, node.branches);
//         }
//     }

//     Node* addOrFindNode(Subscription subscription, in string part,
//                         Node* parent, ref Node*[string] nodes) {
//         if(part in nodes) {
//             auto n = nodes[part];
//             if(part == n.part) {
//                 return n;
//             }
//         }
//         auto node = new Node(part, parent);
//         nodes[part] = node;
//         return node;
//     }

//     void removeSubscription(MqttSubscriber subscriber, ref Node*[string] nodes) {
//         if(_useCache) _cache = _cache.init; //invalidate cache
//         auto newnodes = nodes.dup;
//         foreach(n; newnodes) {
//             if(n.leaves) {
//                 n.leaves = std.algorithm.remove!(l => l.isSubscriber(subscriber))(n.leaves);
//                 if(n.leaves.empty && !n.branches.length) {
//                     removeNode(n.parent, n);
//                 }
//             } else {
//                 removeSubscription(subscriber, n.branches);
//             }
//         }
//     }

//     void removeSubscription(MqttSubscriber subscriber, in string[] topic, ref Node*[string] nodes) {
//         if(_useCache) _cache = _cache.init; //invalidate cache
//         auto newnodes = nodes.dup;
//         foreach(n; newnodes) {
//             if(n.leaves) {
//                 n.leaves = std.algorithm.remove!(l => l.isSubscription(subscriber, topic))(n.leaves);
//                 if(n.leaves.empty && !n.branches.length) {
//                     removeNode(n.parent, n);
//                 }
//             } else {
//                 removeSubscription(subscriber, topic, n.branches);
//             }
//         }
//     }

//     void removeNode(Node* parent, Node* child) {
//         if(parent) {
//             parent.branches.remove(child.part);
//         } else {
//             _nodes.remove(child.part);
//         }
//         if(parent && !parent.branches.length && parent.leaves.empty)
//             removeNode(parent.parent, parent);
//     }

//     void publish(in string topic, string[] topParts, in const(ubyte)[] payload) {
//         publish(topic, topParts, payload, _nodes);
//     }

//     void publish(in string topic, string[] topParts, in const(ubyte)[] payload,
//                  Node*[string] nodes) {
//         //check the cache first
//         if(_useCache && topic in _cache) {
//             foreach(s; _cache[topic]) s.newMessage(topic, payload);
//             return;
//         }

//         //not in the cache or not using the cache, do it the hard way
//         foreach(part; [topParts[0], "#", "+"]) {
//             if(part in nodes) {
//                 if(topParts.length == 1 && "#" in nodes[part].branches) {
//                     //So that "finance/#" matches finance
//                     publishLeaves(topic, payload, topParts, nodes[part].branches["#"].leaves);
//                 }
//                 publishLeaves(topic, payload, topParts, nodes[part].leaves);
//                 if(topParts.length > 1) {
//                     publish(topic, topParts[1..$], payload, nodes[part].branches);
//                 }
//             }
//         }
//     }

//     void publishLeaves(in string topic, in const(ubyte)[] payload,
//                        in string[] topParts,
//                        Subscription[] subscriptions) {
//         foreach(sub; subscriptions) {
//             if(topParts.length == 1 &&
//                equalOrPlus(sub._part, topParts[0])) {
//                 publishLeaf(sub, topic, payload);
//             }
//             else if(sub._part == "#") {
//                 publishLeaf(sub, topic, payload);
//             }
//         }
//     }

//     void publishLeaf(Subscription sub, in string topic, in const(ubyte)[] payload) {
//         sub.newMessage(topic, payload);
//         if(_useCache) _cache[topic] ~= sub;
//     }


// private:

//     bool _useCache;
//     Subscription[] _cache[string];
//     Node*[string] _nodes;
// }


class Subscription {
public:

    Subscription(MqttSubscriber& subscriber, MqttSubscribe::Topic topic,
                 std::vector<std::string> topicParts):
        _subscriber(subscriber),
        _part(std::move(topicParts[topicParts.size() - 1])),
        _topic(std::move(topic.topic)),
        _qos(topic.qos) {
    }

    void newMessage(std::string topic, std::vector<ubyte> payload) {
        _subscriber.newMessage(topic, payload);
    }

    bool isSubscriber(const MqttSubscriber& subscriber) const {
        return &_subscriber == &subscriber;
    }

    bool isSubscription(const MqttSubscriber& subscriber,
                        std::vector<std::string> topics) const {
        return isSubscriber(subscriber) && isTopic(topics);
    }

    bool isTopic(std::vector<std::string> topics) const {
        return std::find(topics.cbegin(), topics.cend(), _topic) == topics.cend();
    }

private:
    MqttSubscriber& _subscriber;
    std::string _part;
    std::string _topic;
    ubyte _qos;
};

} //anonymous namespace
