## 数据格式
数据文件由若干block组成，每个block包括头和数据两部分。
<center> 
![structure](/packdata/data_structure.png)
</center> 
### block
**block**的header由block长度(block_size)、时间戳(time_stamp)、数据名称长度(data_name_len)、数据名称(data_name)四部分构成。  

1. block_size: 4 bytes, 表示整个block的字节数，   
2. time_stamp：8 bytes, 表示这个block中数据的时间戳, 时间戳UTC时间, 单位为微秒， 
3. data_name_len: 2 bytes, 表示data_name部分所占用的字节数，   
4. data_name：data_name_len bytes, 表示这个block中数据的名称, 可以根据该名称在第一个block的data中查找解析本block所需的描述文件。

**block**的data为protobuf message序列化之后的字符串。
### proto block
**proto block**是组成数据文件的第一个block, 该block的结构与其他block相同。时间戳为数据打包时的时间, data_name为'_proto_file_des'。
data是ProtoFileDes消息序列化后的字符串，该消息为用户定义，这里命名为proto_file.proto，其内容如下, 
```
syntax = "proto3";
package xx.xx;

message ProtoFile {
    string name = 1;  // *.proto 的文件名
    bytes content = 2;  // *.proto 的中间描述(FileDescriptorProto)对应的序列化结果
}

message DataType {
    string name = 1;  // 数据的名字或者topic
    string type = 2;  // 数据对应proto类型，eg: xx.xx.TestDataPb
}

message ProtoFileDes {
    repeated ProtoFile file = 1;  // 文件名和文件中间结果的映射
    repeated DataType data  = 2;  // 数据名和类型映射
}
```
上述消息中存储类型为FileDescriptorProto的消息序列化后的数据, FileDescriptorProto类型的消息是由protobuf自身'descriptor.proto'文件定义的, 
FileDescriptorProto消息用于存储'.proto'文件信息。proto block本质上存储的就是整个数据文件中涉及的protobuf消息的定义文件。  
ProtoFile中content字段为FileDescriptorProto的消息序列化后的数据，解析时这些数据被构造成protobuf message类，然后根据DataType中name找到对应的type(type为类型的全名)，
然后根据type获取protobuf message类对数据进行操作。因此为了保证解析是代码的复用，proto_file.proto要与毫末保持一致。
#### FileDescriptorProto生成

用以下C++接口获取编译产生的所有message类的descriptor的指针。
```
static const DescriptorPool * google::protobuf::DescriptorPool::generated_pool()
```
使用DescriptorPool类的以下接口获取对应message的descriptor，这里name为消息的全名即package后增加message的类型名称。
```
const Descriptor* FindMessageTypeByName(ConstStringParam name) const
```
利用Descriptor类的file接口获取FileDescriptor指针, 再用FileDescriptor的Copyto接口获得FileDescriptorProto。
```
const FileDescriptor * file() const
void CopyTo(FileDescriptorProto* proto) const
```
### event block
**event block**是组成数据文件的第二个block，该block存储触发事件的描述信息，其结构与其他block相同。
时间戳为事件的触发时间, data_name为'trigger_des'。
data是描述触发事件类型、触发时间的消息序列化后的字符串。简单示例如下，
```
syntax = "proto3";
package xx.data_collect;

message TriggerDesPb{
  TriggerType trigger_type = 1; // 触发类型
  uint64 trigger_stamp = 2;
}
enum TriggerType{
  TRI_NONE = 0;  // 未知触发
  TRI_CUSTOM = 1; // 自定义触发
}
```
