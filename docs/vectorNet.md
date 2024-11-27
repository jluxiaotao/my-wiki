### 行为预测
行为预测是指根据道路情况（包括车道线、交通标志等）以及待预测目标感知范围内的交通参与者（车辆、行人等）的历史轨迹对预测目标未来一段时间内的行为进行预测。  
传统的行为预测方法是基于规则的，如将交通参与者的运动模型用CV（固定速度）、CA（固定加速度）、CTRV（固定角速度和速度）、CTRA（固定角速度和加速度）模型表征，
然后结合道路的约束条件对交通参与者的行为进行预测。 
近来提出了一些基于学习的行为预测方法，一般是将高度结构化的地图数据和历史轨迹数据（如下图右侧）渲染成图片（如下图左侧），然后用计算机视觉，如卷积神经网络，的方式对图片进行处理。
vectorNet提出了一种直接从这些结构化数据中获取有效信息并最终输出预测轨迹的方式。
<center> 
![representation](/vectorNet/representation.png)
</center> 
### vectorNet整体架构
<center> 
![overview](/vectorNet/overview.png)
</center> 
##### 环境信息的抽象
道路信息和轨迹包括其形位信息和附加属性。
道路元素包括车道线、人行道区域等，每个道路元素都可以由点、线、多边形来描述，同样的轨迹也可以表示成点或线。道路元素可以按照合适的间隔分割成一组线段，
轨迹可以按照固定的时间间隔分割成一组线段。
形位信息可以用这些线段的起点、终点坐标来描述。
道路的附加属性，如交通信号灯颜色、转向信息、限速信息等，和轨迹的附加属性，如时间戳，也可以用数字表示。
最终每个道路元素或轨迹P<sub>j</sub>都可以表示成一组由线段起点坐标、终点坐标、附加属性构成的向量v<sub>i</sub>，
<center> 
![vector](/vectorNet/vector.png)
</center> 
其中**d<sub>i</sub><sup>s</sup>**和**d<sub>i</sub><sup>e</sup>**是线段的起点和终点坐标构成的向量，如果把道路简化成二维平面那么该向量可以表示成（x,y），
在三维空间中可以表示成（x,y,z），最终这些坐标都要转化成以待预测目标上一时刻位置为原点的相对坐标。
**a<sub>i</sub>**是线段的属性构成的向量，如交通信号灯颜色、转向信息、限速信息、时间戳等。
j表示线段所属的道路元素或轨迹的ID。
##### 子图的构建
如上所述，一个场景最终可以由多个道路元素和轨迹构成，每个道路元素或轨迹又由一组向量构成。
这些道路元素和轨迹可以看作是NLP问题中一句话中的单词，接下来需要将构成这个“单词”的一组向量转化成一个向量，
对每个“单词”进行编码，处理后得到的向量中要能够包含之前一组向量中的有效信息。在这里将这个过程称为这构建子图的过程。
###### message passing
消息传递范式或者邻域聚合范式是一种将卷积操作推广到不规则空间的方法。通过消息传递能够将节点信息与其关联节点的信息融合，通过神经网络学习得到更好的节点表征。
假定节点i第k层的特征向量为**x<sub>i</sub><sup>(k)</sup>**，节点j与i的邻接关系为**e<sub>j,i</sub>**（邻接矩阵表示成一维向量），那么消息传递神经网络的范式可以表示为，
<center> 
![messagepassing](/vectorNet/messagepassing.png)
</center> 
其中⊕表示一种可微的、具有排列不变性的函数，如求和、均值、最大值函数。Φ和γ表示可微的函数，比如MLP。
###### vectorNet子图构建
vectorNet中子图的构建过程可以用下面两个式子表示。基本采用了message passing的范式将构成道路元素或轨迹的每个向量与其余向量的信息融合得到新的向量，也即节点新的表征，
然后将这些新的向量中的信息融合成一个向量用于表征这个道路元素或轨迹。
<center> 
![subgraph](/vectorNet/subgraph.png)
</center> 
<center> 
![subgraph_agg](/vectorNet/subgraph_agg.png)
</center> 
式中**g<sub>enc</sub>**为MLP，该MLP包括全连接层、层归一化、ReLU层，**φ<sub>agg</sub>**为最大池化，**φ<sub>rel</sub>**为拼接操作。
具体流程如下图所示，实际应用中采用了多层上述结构的神经网络。
<center> 
![subgraph_overview](/vectorNet/subgraph_overview.png)
</center> 
##### 全局图构建
###### self-attention
自注意力神经网络实际上也是一种结合周围关联节点信息获取节点有效信息新的表征的编码网络。用公式可以表示为，
<center> 
![self-attention](/vectorNet/self-attention.png)
</center> 
式中**Q=XW<sup>Q</sup>**，**K=XW<sup>K</sup>**，**V=XW<sup>V</sup>**，d<sub>k</sub>为Q和K矩阵中每个向量的维度。  
**自注意力编码的理解方式一**：上式中**QK<sup>T</sup>=XW<sup>Q</sup>(XW<sup>K</sup>)<sup>T</sup>=XW<sup>Q</sup>(W<sup>K</sup>)<sup>T</sup>X<sup>T</sup>**,
因此**QK<sup>T</sup>**实际上可以理解为X的加权协方差矩阵，权值**W<sup>Q</sup>(W<sup>K</sup>)<sup>T</sup>**是要经过训练学习的参数，
反映了构成X的各个向量之间的联系。**V**是X经过线性转换得到，**W<sup>V</sup>**要经过训练学习得到。协方差矩阵经过缩放和softmax操作之后与V相乘，
实际上是对所有组成X的节点信息的重新加权，最终获得每个节点的新的表征。用d<sub>k</sub>进行缩放的目的是为了防止数值过大进入softmax梯度很小的区域。  
**自注意力编码的理解方式二**：可以把**K**和**V**矩阵中的节点向量看作键值对，**K**中的向量为一系列**键**，
**V**矩阵中的向量为对应的**值**，**Q**矩阵中的向量为**查询语句**，那么一般情况下的查询操作是要找到与**查询语句**相等的**键**返回其对应的**值**，
这里实际上是根据**查询语句**与所有**键**的吻合度对**值**进行加权相加，最终得到查询结果。
###### ecoding
vectorNet中采用了自注意力网络，但没有对协方差矩阵进行缩放。
<center> 
![encoding](/vectorNet/encoding.png)
</center> 
###### decoding
解码器**φ<sub>traj</sub>**和**φ<sub>node</sub>**为MLP。**φ<sub>traj</sub>**的输出为预测结果，**φ<sub>node</sub>**输出为缺失节点的预测结果。
为了提高学习效果，会随机的屏蔽某个子图向量，尝试根据其它子图向量推断出被屏蔽的子图向量。
<center> 
![decoding](/vectorNet/decoding.png)
</center> 
<center> 
![decoding_masked](/vectorNet/decoding_masked.png)
</center> 
##### 损失函数
###### huber loss
Huber loss是MSE和MAE的结合，超参数δ趋于无穷时等价于MSE，δ趋于0时等价于MAE。
<center> 
![huberloss](/vectorNet/huberloss.png)
</center> 
###### negative Gaussian log-likelihood
假定预测结果中的每个量都为高斯分布，分布中心为真值中的对应位置的量，每个量都给定一个方差，那么可以根据该分布计算预测结果每个量在上述分布中的概率。
模型训练的目的是为了让预测结果中的每个量接近其分布中心，也即预测结果中所有量在其分布中的概率乘积最大。那么也就是概率的对数之和最大，也即负的概率的对数之和最小。
<center> 
![gauss](/vectorNet/gauss.png)
</center>
<center> 
![gauss_log_loss](/vectorNet/gauss_log_loss.png)
</center>
###### 损失函数
损失函数包括两部分**L<sub>traj</sub>**和**L<sub>node</sub>**, 其中**L<sub>traj</sub>**为对目标未来轨迹预测结果的损失，**L<sub>node</sub>**为对缺失节点信息的推断结果损失。
其中**L<sub>traj</sub>**采用negative Gaussian log-likelihood计算，**L<sub>node</sub>**采用huber loss计算，α为加权系数。
<center> 
![loss](/vectorNet/loss.png)
</center> 
##### 训练
###### adam优化器
https://zhuanlan.zhihu.com/p/90169812