#include "zmq_service_interface.h"
#include "zmq_method_provider.h"
#include "general_message.h"
#include "zmq_publisher.hpp"
#include "zmq_subscriber.hpp"

using namespace haomo::com;

// static int alive_count = 100;

// int method(const void* data, size_t len, std::shared_ptr<std::string>& rsp)
// {
//   general_message *msg = (general_message*)data;
//   std::cout << "Received: " << len << " Bytes and data:";

//   for(int i=0; i<msg->valid_len; i++)
//   {
//     std::cout << msg->data[i];
//   }
//   std::cout << std::endl;
//   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//   std::string tmp = "alive";
//   if (alive_count == 0) tmp = "done";
//   alive_count--;
//   //*rsp = std::string((const char*)msg->data, msg->valid_len);
//   *rsp = tmp;

// }

// int roll(std::shared_ptr<ZmqServiceInterface> zmq_method_provider)
// {
//   zmq_method_provider->offer_service();
//   while(1)
//   {
//     zmq_method_provider->run();
//   }
//   zmq_method_provider->stop_offer_service();
// }

void test_show(const std::string& content) {
    if(content.size() != 1) {
        std::cout << "length error:" << content.size() << " , we want 1 Bytes" << std::endl;
        return;
    }
    uint8_t value = 0;
    memcpy(&value, content.c_str(), 1);
    std::cout << "value:" << (int)value << std::endl;
}

int main()
{
  #ifdef TEST_METHOD
  std::string file_path = "/tmp/haomo_em_ipc";
  std::function<int(const void*, size_t, std::shared_ptr<std::string>&)> fun = std::bind(method, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
  auto m_provider = std::make_shared<ZmqMethodProvider>(file_path, fun);
  roll(m_provider);
  #else
  std::string put_topic = "Node1Crash";
  std::cout << "pub topic:" << put_topic << std::endl;
  std::string sub_topic = "Node1CrashResp";
  std::cout << "sub topic:" << sub_topic << std::endl;
  // std::string file_path = "/tmp/haomocrash.ipc";
  ZmqPublisher pub("/tmp/haomocrash.ipc");
  pub.init();

  ZmqSubscriber sub("/tmp/haomocrashresp.ipc");
  sub.init();
  sub.subscribe(sub_topic, test_show);
  sub.start();

  uint8_t value = 1;
  std::string content = std::string(reinterpret_cast<const char*>(&value), 1);
  int counter = 0;
  while (1) {
      pub.publish(put_topic, content);

      std::this_thread::sleep_for(std::chrono::seconds(1));
      counter++;
  }
  #endif

  return 0;
}
