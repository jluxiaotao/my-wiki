### ZMQ
ZMQ通过一系列不同类型的socket实现不同形式的进程内、进程间、TCP、多播通信。    
### zmq socket
ZMQ socket与传统socket存在一定差异：    
1.传统socket传输的是字节流，ZMQ socket传输的是消息；    
2.传统socket是同步的，zmq socket是异步的，会建立消息队列存储不能被及时接收的消息；    
3.可以实现多对多连接。
### zmq socket的基本操作
socket的基本操作有：    
1.创建与销毁    
2.参数配置    
3.建立连接    
4.收发数据  
  
以C++ API cppzmq库为例：    
服务器端创建一个类型为RES的socket，并绑定到5555端口，然后接收消息，并进行回复。    
```
#include <string>
#include <chrono>
#include <thread>
#include <iostream>

#include <zmq.hpp>

int main() 
{
    using namespace std::chrono_literals;

    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REP (reply) socket and bind to interface
    zmq::socket_t socket{context, zmq::socket_type::rep};
    socket.bind("tcp://*:5555");

    // prepare some static data for responses
    const std::string data{"World"};

    for (;;) 
    {
        zmq::message_t request;

        // receive a request from client
        socket.recv(request, zmq::recv_flags::none);
        std::cout << "Received " << request.to_string() << std::endl;

        // simulate work
        std::this_thread::sleep_for(1s);

        // send the reply to the client
        socket.send(zmq::buffer(data), zmq::send_flags::none);
    }

    return 0;
}
```
客户端创建一个类型为REQ的socket，并连接到5555端口，然后发送消息。  
```
#include <string>
#include <iostream>

#include <zmq.hpp>

int main()
{
    // initialize the zmq context with a single IO thread
    zmq::context_t context{1};

    // construct a REQ (request) socket and connect to interface
    zmq::socket_t socket{context, zmq::socket_type::req};
    socket.connect("tcp://localhost:5555");

    // set up some static data to send
    const std::string data{"Hello"};

    for (auto request_num = 0; request_num < 10; ++request_num) 
    {
        // send the request message
        std::cout << "Sending Hello " << request_num << "..." << std::endl;
        socket.send(zmq::buffer(data), zmq::send_flags::none);
        
        // wait for reply from server
        zmq::message_t reply{};
        socket.recv(reply, zmq::recv_flags::none);

        std::cout << "Received " << reply.to_string(); 
        std::cout << " (" << request_num << ")";
        std::cout << std::endl;
    }

    return 0;
}

```
### message queue
ZMQ会为每个连接创建一个消息队列。  
在bind时并不知道将要建立的连接数量，因此一般是在connect的时候创建一个消息队列。
### 高水位线
高水位线是某个ZMQ消息队列缓存在内存中的未处理的消息的上限值, 接收和发送默认为1000。当超过这个上限时，ZMQ会根据socket的类型
进行相应的操作，比如丢弃消息或拒收消息。
```
skt.setsockopt( ZMQ_RCVHWM, 1000 );
skt.setsockopt( ZMQ_SNDHWM, 1000 );
```
### zmq socket类型
常用的zmq socket类型有：  
1.REQ socket  
2.REP socket  
3.DEALER socket  
4.ROUTER socket  
5.PUB socket  
6.SUB socket  
7.PUSH socket  
8.PULL socket  
基于对这些不同类型socket的使用组合可以实现不同的通信模式。  

**REQ socket**  
客户端使用REQ套接字向服务发送请求并从服务接收回复。这种套
接字类型只允许交替的发送和接收消息。REQ套接字可以
连接到任意数量的REP或ROUTER套接字。发送的每个请求都在所有
连接的服务中进行轮询，接收到的每个回复都与最后发出的请求相
匹配。如果没有可用的服务，那么套接字上的任何发送操作都将被阻止，
直到至少有一个服务可用。REQ套接字不会丢弃任何消息。

**REP socket**  
REP套接字由服务用来接收来自客户端的请求并向客户端发送回复。
与REQ socket相同，这种套接字类型只允许交替的发送和接收消息。
接收到的每个请求都是从所有客户端中公平排队的，
发送的每个回复都被路由到发出最后一个请求的客户端。
如果原始请求者已不存在，则会静默地丢弃回复。  

**REQ socket和REP socket可以构成一种同步的request-reply通信模式。**  

**DEALER socket**   
DEALER套接字类型与一组匿名对等方进行通信。DEALER作为REQ的异步替代品，
用于与REP或ROUTER服务器通信的客户端。

当DEALER套接字由于达到所有对等方的高水位线而进入静音状态时，
或者如果根本没有对等方，则套接字上的任何发送操作都将被阻止，
直到静音状态结束或至少有一个对等方可用于发送,消息不会被丢弃。

当DEALER套接字连接到REP套接字时，发送的消息必须包含一个空帧作为消息的第一部分（分隔符），
然后是一个或多个正文部分。

**ROUTER socket**   
ROUTER套接字类型使用显式寻址与一组对等方进行通信，
以便将每个传出消息发送到特定的对等端连接。
ROUTER是REP的异步替代品，通常被用作与DEALER客户端通信。

当接收到消息时，ROUTER套接字将在将消息传递给应用程序之前，
会为消息添加一个部分，该消息部分包含消息发送方的地址。
发送消息时，ROUTER套接字将删除消息的第一部分，并使用它来确定接收方的地址。
如果该接收方不再存在或从未存在，则应静默地丢弃该消息。

当ROUTER套接字由于达到某个对等方的高水位线而进入静音状态时，
发送到这个套接字的任何消息都将被丢弃，直到静音状态结束。

当REQ套接字连接到ROUTER套接字时，
除了添加消息发送方的地址，还会增加一个空的帧作为分隔符。
因此，应用程序看到的每个接收消息的整个结构变成：地址、分隔符部分、一个或更多正文部分。
当向REQ套接字发送回复时，应用程序必须包括分隔符部分。

**ROUTER socket和DEALER socket可以构成一种异步的request-reply通信模式。**  

**PUB socket**   
发布者使用PUB套接字来分发数据。
PUB端发布的数据应该以topic名称开头。
此套接字类型无法接收任何消息。

当PUB socket与某个订阅者的消息队列达到高水位线而进入静音状态时，
发送给该订阅者的任何消息都将被丢弃，直到静音状态结束。

**SUB socket**   
订阅者使用SUB套接字来订阅发布者分发的数据。
SUB端通过topic名称对消息进行筛选。
此套接字类型无send函数。
```
#include <iostream>
#include <string>
#include <unistd.h>
#include "zmq/zmq.hpp"


int main( int argc, char *argv[] )
{
    // Create ZMQ Context
    zmq::context_t context ( 1 );
    // Create the Publish socket
    zmq::socket_t publisher ( context, ZMQ_PUB );
    // Bind to a tcp socket
    publisher.bind( "tcp://*:5556" );
    // Sleep for 1 sec, if this sleep is removed, you may lose some initial messages while it is being binded
    usleep( 1000000 );
    // Message to send to the subscribers
    std::string msg = "test";

    // loop 100 times
    for ( int i = 1; i <= 100; i++ ) 
    {
        // Create zmq message
        zmq::message_t request( msg.length() );
        // Copy contents to zmq message
        memcpy( request.data(), msg.c_str(), msg.length() );
        // Publish the message
        publisher.send( request );
        std::cout << "sending: " << i << std::endl;
    }
}
```

```
#include <iostream>
#include <string>
#include "zmq/zmq.hpp"


int main( int argc, char *argv[] )
{
    // Create ZMQ Context
    zmq::context_t context ( 1 );
    // Create the Subscribe socket
    zmq::socket_t subscriber ( context, ZMQ_SUB );
    // Connect to a tcp socket
    subscriber.connect( "tcp://localhost:5556" );
    // Set the socket option to subscribe
    subscriber.setsockopt( ZMQ_SUBSCRIBE, "test", 4 );

    // infinite loop to receive messages
    for ( int i = 1; i > 0; i++ )
    {
        // Receive the message and convert to string
        zmq::message_t update;
        subscriber.recv( &update );
        std::string msg = update.to_string();
        // Print the message
        std::cout << "Num: " << i << ", message: " << msg << std::endl;
    }

}
```