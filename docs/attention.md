## 注意力机制  
神经网络的核心是从输入中提取特征，然后将提取的特征用于分类和回归任务。注意力机制本质上是一种加权，一般是在更高抽象维度上的加权，能够使模型在特征提取过程中关注对于任务更加重要的信息。 

## RNN与Attention  
<center> 
![rnn](/attention/rnn_equation.png)  
</center>
<center> 
<img src="/attention/rnn.png"  width="500" /> 
</center>
如图所示，RNN以序列数据作为输入，可以产生序列输出。根据输入输出序列长度，有1-to-n、n-to-1、n-to-n、n-to-m几种形式。其中n-to-m的形式，又称为seq2seq，是以Encoder-Decoder的形式实现的。  
<center> 
<img src="/attention/1ton_rnn.png"  width="300" />
<br>
1ton_rnn
</center>
<center> 
<img src="/attention/1ton_rnn_2.png"  width="300" />
<br>
1ton_rnn_2 
</center>
<center> 
<img src="/attention/nto1_rnn.png"  width="300" />
<br>
nto1_rnn  
</center>
Encoder-Decoder的形式如下，encoder模块是一个n-to-1形式的RNN，将输入转化成一个上下文向量c。decoder是一个1-to-n形式的RNN，将c作为输入得到输出。
<center> 
<img src="/attention/ntom_rnn.png"  width="500" />
<br>
ntom_rnn  
</center>
<center> 
<img src="/attention/ntom_rnn_2.png"  width="500" />
<br>
ntom_rnn_2  
</center>
在编码过程中序列中前面的信息逐渐稀释甚至覆盖，因此无法很好的处理长序列问题。
<center> 
<img src="/attention/rnn_memory.gif"  width="500" />
</center>
如果decoder用到的上下文向量c能够根据当前解码需求变化，合理调整encoder输入序列中各元素在c中的比重，那么就能够提高模型的性能。这种根据当前任务状态调整关注信息的机制称为注意力机制。
<center> 
<img src="/attention/rnn_attention.png"  width="500" />
</center>
图中，h'<sub>i-1</sub>实际上可以代表当前解码任务需求，h'<sub>i-1</sub>与编码器各个输入的隐藏层特征h<sub>j</sub>点乘本质上是计算当前任务需求与序列中各个元素的相关性，根据相关性加权h<sub>j</sub>最终得到当前解码所需的上下文向量c<sub>i</sub>.

## CNN与Attention
### channel attention  
<center> 
<img src="/attention/cnn_channel_attention.png"  width="500" />
</center>
CNN经过C个卷积核对图像进行卷积操作，得到HxWxC维的特征矩阵，每一个卷积核也即channel学习到的HxW维特征可以看作是图像中某一种特性的抽象，C个卷积核就学习到C种特性。每个channel的特征在解决不同问题时发挥的作用可能是有差异的，如果能够学习到每个channel对于待解决问题的重要程度，就有可能提高模型的性能。  
在实现中，可以如上图，首先将特征在空间维度上进行压缩，也即每个channel的HxW维特征做平均池化操作，
<center> 
<img src="/attention/cnn_channel_attention_avgpool.png"  width="300" />
</center>
然后，学习每个通道的权重，
<center> 
<img src="/attention/cnn_channel_attention_weight.png"  width="300" />
</center>
其中sigma是sigmod操作，delta是ReLu操作。W<sub>1</sub>∈R<sup>C/rxC</sup>,  W<sub>2</sub>∈R<sup>CxC/r</sup>。
最后将特征（维度HxWxC）与学习到的权重相乘。

[Squeeze-and-Excitation Networks](https://arxiv.org/pdf/1709.01507)
### spatial attention  
<center> 
<img src="/attention/cnn_spatial_attention.png"  width="500" />
</center>
类似的，在空间维度上不同位置特征的重要程度，在解决问题时也是不一样的。
在实现中，可以如上图，首先将特征在channel维度上进行压缩，可以采用maxpooling或avgpooling，图中采用的方式是二者叠加，
<center> 
<img src="/attention/cnn_spatial_attention_equation.png"  width="300" />
</center>
其中sigma是sigmod操作，f<sup>7x7</sup>是卷积核为7x7的卷积操作函数。  
然后，进行卷积操作，再经过激活函数处理得到HxW维的空间权重信息。  
最终将权重信息与特征相乘。  

[CBAM](https://arxiv.org/pdf/1807.06521)
## self-Attention
前面提到，在seq2seq模式编码过程中，输入序列个元素之间不是独立的，RNN可以通过将上一个元素的隐藏状态叠加到当前元素隐藏状态的形式实现对于历史信息的记忆功能，这种记忆会随着序列增长稀释。在seq2seq模式解码过程中考虑当前解码状态与输入序列中元素的相关性能够提高模型的性能。实际上在编码过程中也可以考虑当前元素与历史序列中元素的相关性，这样每个元素的特征对于历史序列中各个元素的记忆变成了一种有选择性的记忆，能够更好的表征元素的特征。  
这种考虑序列内部不同位置元素的相关性进行编码的方式称为自注意力机制。  
在transformer的实现中，自注意力机制的形式如下，  
#### query key value
transformer是encoder-decoder形式的网络架构，以其在NLP问题中的应用为例，首先将输入序列中的词语进行embedding和positional encoding操作，转化成词语向量。每个词语向量经过W<sup>Q</sup>, W<sup>K</sup>, W<sup>V</sup>转化成query, key, value
<center> 
<img src="/attention/transformer_qkv.png"  width="500" />
</center>

#### Scaled Dot-Product Attention
query,key,value均由词语向量线性转化得到，query和序列中所有词语的key**s**的点积进行归一化等处理实际上能够反映某个词语与其他所有词语的相关性，然后将相关性与value**s**相乘就得到了新的词语向量，该向量是考虑了词语之间相关性的表示结果。
<center> 
<img src="/attention/transformer_self_attention.png"  width="500" />
</center>

#### 矩阵表示形式
输入序列的所有词语向量堆叠成X，X与W<sup>Q</sup>, W<sup>K</sup>, W<sup>V</sup>相乘得到queries,keys,values堆叠成的Q,K,V矩阵。
<center> 
<img src="/attention/transformer_qkv_stack.png"  width="400" />
</center>
Q与K点积然后进行归一化处理得到序列中词语的协相关矩阵，然后与V矩阵相乘得到所有词语向量组成的矩阵。  
<center> 
<img src="/attention/transformer_self_attention_stack.png"  width="500" />
</center>
[LSTM](https://arxiv.org/pdf/1601.06733)  
[attention is all you need](https://arxiv.org/pdf/1706.03762)

## MHSA
h个自注意力模块并行计算，将结果拼接起来再进行线性转换得到指定维度的向量表示。
<center> 
<img src="/attention/MHSA.png"  width="300" />
</center>

## Attention的一般形式  
注意力可以理解为根据查询内容(Q)与键(K)的相关性对值(V)进行加权得到查询结果的过程。
这种相关性是动态的，这种动态如果完全由输入引入，则称为自注意力，一般是Q和K由输入变换得到或者Q不变K由输入变换得到。

## cross Attention

## 参考  
[https://zhuanlan.zhihu.com/p/609523552](https://zhuanlan.zhihu.com/p/609523552)  
[Transformers from scratch](https://peterbloem.nl/blog/transformers)  
[Transformer图文详解](https://blog.csdn.net/benzhujie1245com/article/details/117173090)