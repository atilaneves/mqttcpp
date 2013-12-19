#include "broker.hpp"
#include "message.hpp"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <unordered_map>


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
} //anonymous namespace


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



struct SubscriptionTree {
public:
    // void addSubscription(Subscription* s, std::vector<std::string> parts) {
    //     assert(parts.size());
    //     if(_useCache) _cache.clear(); //invalidate cache
    //     addSubscriptionImpl(s, parts, null, _nodes);
    // }

    // void addSubscriptionImpl(Subscription s, const(string)[] parts,
    //                          Node* parent, ref Node*[string] nodes) {
    //     auto part = parts[0];
    //     parts = parts[1 .. $];
    //     auto node = addOrFindNode(part, parent, nodes);
    //     if(parts.empty) {
    //         node.leaves ~= s;
    //     } else {
    //         addSubscriptionImpl(s, parts, node, node.branches);
    //     }
    // }

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

    void useCache(bool u) { _useCache = u; }

private:
    struct Node {
        Node(std::string pt, Node* pr):part(pt), parent(pr) {}
        std::string part;
        Node* parent;
        std::unordered_map<std::string, Node*> branches;
        std::vector<Subscription> leaves;
    };

    bool _useCache;
    std::unordered_map<std::string, Subscription*> _cache;
    std::unordered_map<std::string, Node*> _nodes;

    Node* addOrFindNode(std::string part, Node* parent,
                        std::unordered_map<std::string, Node*> nodes) {
        if(nodes.count(part) && part == nodes[part]->part) {
            return nodes[part];
        }
        auto node = new Node(part, parent);
        nodes[part] = node;
        return node;
    }
};



class MqttBroker {
public:

    void subscribe(MqttSubscriber& subscriber, std::vector<std::string> topics) {
        std::vector<MqttSubscribe::Topic> newTopics;
        std::transform(topics.cbegin(), topics.cend(), std::back_inserter(newTopics),
                       [](std::string t) { return MqttSubscribe::Topic(t, 0); });
        subscribe(subscriber, newTopics);
    }

    void subscribe(MqttSubscriber& subscriber, std::vector<MqttSubscribe::Topic> topics) {
        (void)subscriber;
        for(const auto& t: topics) {
            std::vector<std::string> parts;
            boost::split(parts, t.topic, boost::is_any_of("/"));
            //_subscriptions.addSubscription(Subscription(subscriber, t, parts), parts);
        }
    }

    // void unsubscribe(MqttSubscriber& subscriber) {
    //     _subscriptions.removeSubscription(subscriber, _subscriptions._nodes);
    // }

    // void unsubscribe(MqttSubscriber& subscriber, std::vector<std::string> topics) {
    //     _subscriptions.removeSubscription(subscriber, topics, _subscriptions._nodes);
    // }

    // void publish(std::string topic, std::vector<ubyte> payload) {
    //     auto topParts = array(splitter(topic, "/"));
    //     publish(topic, topParts, payload);
    // }

    void useCache(bool u) { _subscriptions.useCache(u); }

private:

    SubscriptionTree _subscriptions;

    // void publish(std::string topic, std::vector<std::string> topParts,
    //              std::vector<ubyte> payload) {
    //     _subscriptions.publish(topic, topParts, payload);
    // }
};
