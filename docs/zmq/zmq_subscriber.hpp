/**************************************************************************
 *
 * Copyright (c) 2020 HaoMo.com, Inc. All Rights Reserved
 *
 *************************************************************************/

/*
 * @Author: tengruying
 * @Date: 2023-01-04 09:00:00
 * @LastEditors: tengruying
 * @Description:
 */

#ifndef HAOMO_COM_ZMQ_SUBSCRIBER_HPP
#define HAOMO_COM_ZMQ_SUBSCRIBER_HPP

#include <iostream>
#include <string>
#include <zmq.hpp>
#include <map>
#include <atomic>

#include "log_stream.hpp"

namespace haomo {
namespace com {

typedef std::function<void(const std::string &)> ZmqSubCallBack;

class ZmqSubscriber {
public:
  ZmqSubscriber(const std::string &ip, uint16_t port) : _is_topic_empty(false) {
    std::stringstream ss;
    ss << "tcp://" << ip << ":" << port;
    _url = ss.str();
  };
  ZmqSubscriber(const std::string& file_path) : _is_topic_empty(false) {
    _url =  "ipc://" + file_path;
  };
  ~ZmqSubscriber() {
      HLOG_INFO << "ZmqSubscriber deconstruct begin";
      release();
      HLOG_INFO << "ZmqSubscriber deconstruct end";
  }

  int init() {
    _zmq_context = new zmq::context_t(1);
    _subscriber = zmq::socket_t(*_zmq_context, ZMQ_SUB);

    _subscriber.set(zmq::sockopt::sndhwm, 4096);
    _subscriber.set(zmq::sockopt::rcvhwm, 4096);
    _subscriber.set(zmq::sockopt::linger, 0);

    return connect(_url);
  }

  int subscribe(const std::string &topic, ZmqSubCallBack func) {
    if(_is_topic_empty) {
      HLOG_ERROR << _url << " has subscribed a empty topic and can not subscribe a new";
    }
    auto &&iter = _sub_topics.find(topic);
    if (iter == _sub_topics.end()) {
        if(topic.empty()) {
          HLOG_INFO << _url << " topic is empty and will subscribe all data";
          _is_topic_empty = true;
          HLOG_INFO << _url << " remove other topics in _sub_topics";
          _sub_topics.clear();
        } else {
          HLOG_INFO << _url << " subscribe " << topic;
        }
        _sub_topics.insert(std::pair<std::string, ZmqSubCallBack>(topic, func));
        //_subscriber.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.length());
        _subscriber.set(zmq::sockopt::subscribe, zmq::const_buffer(topic.c_str(), topic.length()));
    } else {
      HLOG_ERROR  << _url << " has subscribed " << topic;
    }
    return 0;
  }

  // run after all subscribe fun running and can not run subscribe again after run this.
  int start() {
    _running = true;
    if(_is_topic_empty) {
      _handle_thread = std::thread(&ZmqSubscriber::handle_all_thread, this);
      return 0;
    }
    _handle_thread = std::thread(&ZmqSubscriber::handle_thread, this);

    return 0;
  }

  int unsubscribe(const std::string &topic) {
    auto &&iter = _sub_topics.find(topic);
    if (iter != _sub_topics.end()) {
        //_subscriber.setsockopt(ZMQ_UNSUBSCRIBE, topic.c_str(), topic.length());
        _subscriber.set(zmq::sockopt::unsubscribe, zmq::const_buffer(topic.c_str(), topic.length()));
        _sub_topics.erase(iter);
    }
    return 0;
  }

private:
  int release() {
    _running = false;
    if (_handle_thread.joinable()) {
        _handle_thread.join();
    }
    _subscriber.close();
    if (_zmq_context != nullptr) {
        delete _zmq_context;
        _zmq_context = nullptr;
    }
    return 0;
  }

  int connect(const std::string &url) {
    try {
        _subscriber.connect(url);
    } catch (std::exception &e) {
        HLOG_ERROR << "catch exception: " << e.what();
        return -1;
    }
    return 0;
  }

  int unconnect(const std::string &url) {
    if (_running) {
        try {
            _subscriber.disconnect(url);
        } catch (std::exception &e) {
            HLOG_ERROR << "catch exception: " << e.what();
            return -1;
        }
    }
    return 0;
  }

  void handle_thread() {
    std::string thread_name = "ZmqSub";
     static_cast<void>(pthread_setname_np(pthread_self(), thread_name.c_str()));

    while (_running) {
        try {
            zmq::pollitem_t zmq_pool_item = {_subscriber, 0, ZMQ_POLLIN, 0};
            int rc = zmq::poll(&zmq_pool_item, 1, 100);
            if (rc < 0) {
                continue;
            }
            if (zmq_pool_item.revents & ZMQ_POLLIN) {
                int32_t recv_more;
                size_t size_more = sizeof(recv_more);

                zmq::message_t topic;
                if (!_subscriber.recv(topic)) {
                    HLOG_ERROR << "topic recv error";
                    continue;
                }

                _subscriber.getsockopt(ZMQ_RCVMORE, &recv_more, &size_more);
                zmq::message_t content;
                if (!recv_more || !_subscriber.recv(content)) {
                    HLOG_ERROR << topic.to_string() << " content recv error";
                    continue;
                }
                std::string content_str;
                content_str.assign(static_cast<char *>(content.data()), content.size());

                ZmqSubCallBack cb = nullptr;
                {
                    //HLOG_INFO  << "find " << topic.to_string() << " in _sub_topics";
                    //HLOG_INFO  << "_sub_topics:" << this->_sub_topics.count(topic.to_string());
                    auto iter = this->_sub_topics.find(topic.to_string()); //why use this pointer?
                    if (iter == this->_sub_topics.end()) {
                        HLOG_ERROR << topic.str() << " wasn't in sub_topics";
                        //_subscriber.set(zmq::sockopt::unsubscribe, zmq::const_buffer(topic.to_string().c_str(), topic.to_string().length()));
                        continue;
                    }
                    cb = iter->second;
                }
                if (cb) {
                    cb(content_str);
                }
            }
        } catch (const std::exception &e) {
            HLOG_ERROR << "ZmqSubscriber recv exception " << e.what();
            //_running = false;
            // if (!_running) {
            break;
            //}
        }
    }
  }

  void handle_all_thread() {
    std::string thread_name = "ZmqSub";
    static_cast<void>(pthread_setname_np(pthread_self(), thread_name.c_str()));

    while (_running) {
        try {
            zmq::pollitem_t zmq_pool_item = {_subscriber, 0, ZMQ_POLLIN, 0};
            int rc = zmq::poll(&zmq_pool_item, 1, 100);
            if (rc < 0) {
                continue;
            }
            if (zmq_pool_item.revents & ZMQ_POLLIN) {
                zmq::message_t content;
                if (!_subscriber.recv(content)) {
                    HLOG_ERROR << " content recv error";
                    continue;
                }
                std::string content_str;
                content_str.assign(static_cast<char *>(content.data()), content.size());

                ZmqSubCallBack cb = nullptr;
                {
                    if (this->_sub_topics.size() != 1) {
                        HLOG_ERROR << " error size in sub_topics:" << this->_sub_topics.size();
                        continue;
                    }
                    cb = _sub_topics.begin()->second;
                }
                if (cb) {
                    cb(content_str);
                }
            }
        } catch (const std::exception &e) {
            HLOG_ERROR << "ZmqSubscriber recv exception " << e.what();
            break;
        }
    }
  }

private:
  zmq::context_t* _zmq_context;
  zmq::socket_t _subscriber;
  std::string _url;

  bool _is_topic_empty;
  std::atomic<bool> _running{false};
  std::thread _handle_thread;

  std::map<std::string, ZmqSubCallBack> _sub_topics;
};

}  // namespace com
}  // namespace haomo

#endif  // HAOMO_COM_ZMQ_SUBSCRIBER_HPP