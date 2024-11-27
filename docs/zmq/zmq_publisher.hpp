/**************************************************************************
 *
 * Copyright (c) 2020 HaoMo.com, Inc. All Rights Reserved
 *
 *************************************************************************/

/*
 * @Author: tengruying
 * @Date: 2023-01-05 09:00:00
 * @LastEditors: tengruying
 * @Description:
 */

#ifndef HAOMO_COM_ZMQ_PUBLISHER_HPP
#define HAOMO_COM_ZMQ_PUBLISHER_HPP

#include <iostream>
#include <string>
#include <zmq.hpp>
#include <map>

#include "log_stream.hpp"

namespace haomo {
namespace com {

class ZmqPublisher {
public:
  ZmqPublisher(const std::string &ip, uint16_t port) {
    std::stringstream ss;
    ss << "tcp://" << ip << ":" << port;
    _url = ss.str();
  };
  ZmqPublisher(const std::string& file_path) {
    _url =  "ipc://" + file_path;
  };
  ~ZmqPublisher() {  // release();
      HLOG_INFO << "ZmqPublisher deconstruct begin";
      release();
      HLOG_INFO << "ZmqPublisher deconstruct end";
  };

  int init() {
    _zmq_context = new zmq::context_t(1);
    _publisher = zmq::socket_t(*_zmq_context, ZMQ_PUB);

    try {
        _publisher.bind(_url);
    } catch (const std::exception& ex) {
        HLOG_ERROR << "ZmqPublisher bind caught exception:" << ex.what();
        return -1;
    } catch (...) {
        HLOG_ERROR << "Caught unknown exception";
        return -1;
    }

    _publisher.set(zmq::sockopt::sndhwm, 4096);
    _publisher.set(zmq::sockopt::rcvhwm, 4096);
    _publisher.set(zmq::sockopt::linger, 0);
    //_publisher.setsockopt(ZMQ_SNDTIMEO, 0);
    //_publisher.setsockopt(ZMQ_RCVTIMEO, 0);
    return 0;
  }

  int publish(const std::string &topic, const std::string &content) {
    zmq::message_t topic_msg(topic.c_str(), topic.length());
    zmq::message_t content_msg(content.c_str(), content.length());

    int rc = 0;
    try {
        //HLOG_ERROR << "publisher sent topic: " << topic;
        _publisher.send(topic_msg, zmq::send_flags::sndmore);
        //HLOG_ERROR << "publisher sent content: " << content;
        _publisher.send(content_msg, zmq::send_flags::none);
    } catch (const std::exception &e) {
        rc = -1;
        HLOG_ERROR << "publisher catch exception " << e.what();
    }
    return rc;
  }

private:
  int release() {
    _publisher.unbind(_url);
    _publisher.close();
    if (_zmq_context != nullptr) {
        delete _zmq_context;
        _zmq_context = nullptr;
    }
    return 0;
  }

private:
  zmq::context_t* _zmq_context;
  zmq::socket_t _publisher;
  std::string _url{""};
};

}  // namespace com
}  // namespace haomo

#endif  // HAOMO_COM_ZMQ_PUBLISHER_HPP