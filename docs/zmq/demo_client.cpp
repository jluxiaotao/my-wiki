#include "zmq_client_interface.h"
#include "zmq_method_customer.h"
#include "zmq_subscriber.hpp"
#include "zmq_publisher.hpp"
#include <thread>


using namespace haomo::com;

// char request[6] = {'H', 'e', 'l', 'l', 'o', 0};

// int roll(std::shared_ptr<ZmqClientInterface> zmq_method_customer)
// {
//   zmq_method_customer->find_service();
//   std::string rsp;
//   while(1)
//   {
//     zmq_method_customer->run(request, 6, rsp);
//     auto reply = rsp.c_str();
//     std::cout << "reply:" << (int)reply[5] << std::endl;
//     std::this_thread::sleep_for(std::chrono::milliseconds(10000));
//     request[5]++;
//   }
//   zmq_method_customer->stop_find_service();
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
  auto m_provider = std::make_shared<ZmqMethodCustomer>(file_path);
  roll(m_provider);
  #else
  std::string put_topic = "Node1CrashResp";
  std::cout << "pub topic:" << put_topic << std::endl;
  std::string sub_topic = "Node1Crash";
  std::cout << "sub topic:" << sub_topic << std::endl;
  // std::string file_path = "/tmp/haomocrash.ipc";
  ZmqSubscriber sub("/tmp/haomocrash.ipc");
  sub.init();

  ZmqPublisher pub("/tmp/haomocrashresp.ipc");
  pub.init();

  sub.subscribe(sub_topic, test_show);
  sub.start();
  uint8_t value = 1;
  std::string content = std::string(reinterpret_cast<const char*>(&value), 1);
  int counter = 0;
  while (1) {
      std::this_thread::sleep_for(std::chrono::seconds(20));
      pub.publish(put_topic, content);
      counter++;
  }

  #endif

  return 0;
}