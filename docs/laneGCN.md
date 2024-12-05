## 模型简介  
2020年Uber提出的运动预测模型，基于argoverse数据集进行训练、验证和评价。在预测过程中考虑了车道、交通参与者之间的交互，能够输出多模预测轨迹。

## laneGCN整体架构
模型分为4部分：历史轨迹特征提取网络（ActorNet），地图特征提取网络（laneGCN），特征交互融合网络（FusionNet），运动预测网络（preciction header）。
<center> 
<img src="/laneGCN/laneGCN_simplearchi.png"  width="500" />
</center>

<center> 
<img src="/laneGCN/laneGCN_archi.png"  width="500" />
</center>

#### 交通参与物特征提取  
交通参与物的自然表征为历史轨迹，也即过去一段时间内各个时刻的位置信息。这里需要通过神经网络从历史轨迹中学习到特征信息。
这里通过一维CNN来学习时序序列的特征，然后通过FPN（feature pyramid network）将学习到的不同时间范围内的特征进行融合。
<center> 
<img src="/laneGCN/actorNet.png"  width="600" />
</center>
每个交通参与者在过去T个步长的轨迹表示成3xT维矩阵，其中每个步长车辆的状态表示为（x, y, flag），分别为x坐标、y坐标、该时刻历史状态是否存在标志位，若不存在则坐标用0填充。  
然后将这些时序数据用3组一维残差卷积网络进行处理，得到Txn、T/2xn、T/4xn的三组中间特征。  
最后通过升采样逐一将三组特征处理成同一纬度并相加，得到Txn维特征，最终取0时刻的特征作为交通参与者的表征。

#### 地图特征提取
<center> 
<img src="/laneGCN/mapNet.png"  width="600" />
</center>

###### 地图的自然表征
车道可以由车道中心线表征，每条车道中心线可以划分为若干小段，每一段为一个节点。每个节点可以由两个点的坐标表示，节点之间的关系有前驱、后继、左邻、右邻4种，邻接关系用邻接矩阵表示。  
###### 节点特征编码
<center> 
**n**<sub>i</sub> = MLP<sub>shape</sub>(**n**<sub>i,end</sub> - **n**<sub>i,start</sub>) + MLP<sub>loc</sub>**n**<sub>i,start</sub>
</center>
用上式所示方法将两个点坐标的自然表征进行转换，编码所得的信息包括节点的形状信息（方向，长度）和位置信息。最终每个节点输出的特征维度为128。

###### 地图特征提取
节点特征编码只局限于节点自身，没有考虑和周围节点的关系。通过laneGCN学习节点之间的拓扑关系。  
laneGCN基于图卷积神经网络，为了学习单条车道内部节点的长距离作用关系又引入了人空洞卷积。
<center> 
<img src="/laneGCN/laneGcn.png"  width="300" />
</center>
上式中W为带学习权重，X为节点特征，A为邻接矩阵，A<sup>k</sup>为邻接矩阵的k次幂。公式一共有3部分组成，第一部分XW<sub>0</sub>为节点自身特征的加权，第二部分A<sub>i</sub>XW<sub>i</sub>为左邻、右邻对节点自身的影响，第三部分A<sub>i</sub><sup>kc</sup>XW<sub>i,kc</sub>为前驱、后继以及更远范围的前驱、后继对节点自身的影响。
前两项和第三项Kc=1相加实际上可以看作是有多个卷积核的图卷积神经网络，第三项kc！=1的部分属于空洞卷积神经网络。  
下图为CNN中的空洞卷积，通过引入超参数dilate rate，在不需要下采样且不增加计算量的条件下增大卷积的感受野。dilate rate指的是卷积核各点之间的间隔。
<center> 
<img src="/laneGCN/dilatedCNN.png"  width="300" />
</center>
这里在GCN中也应用同样的思路，用邻接矩阵的乘方拓展了节点之间的连接关系，提高了GCN的感受野，同时不需要下采样。
<center> 
<img src="/laneGCN/laneGcn_fig.png"  width="400" />
</center>

#### 特征融合
<center> 
<img src="/laneGCN/fusionNet.png"  width="500" />
</center>
前面通过actorNet和mapNet分别获取了交通参与者和地图的特征信息，实际上交通参与物之间、地图节点之间、交通参与物对于地图、地图对于交通参与物都不是相互独立的，这些特征需要进行融合。这里用4个模块按照A2L、L2L、L2A、A2A的顺序将两部分特征融合。  

###### A2L
首先将交通参与者的特征融合入地图特征（A2L），采用空间注意力机制的思想，将欧氏距离小于给定范围的车辆特征信息融入车道节点特征。特征融合的方式为，
<center> 
<img src="/laneGCN/chara_fusion.png"  width="300" />
</center>
上式中，x<sub>i</sub>为目标节点特征向量，x<sub>j</sub>为待融合节点特征向量，Δ<sub>i,j</sub>为MLP(ecu_dis), 也即由欧氏距离线性变换得到，Φ为层归一化和relu。  

###### L2L
地图节点与节点之间的特征融合仍采用laneGCN。

###### L2A
地图节点特征融合入交通参与物特征，方法与A2L一致。

###### A2A
交通参与物特征的融合，方法与A2L一致。

#### 运动预测
<center> 
<img src="/laneGCN/head.png"  width="500" />
</center>
用一个残差网络和全连接层回归得到M个目标的预测轨迹，每个目标预测K条轨迹，每条轨迹30个点组成。  
将每条轨迹终点与起点的欧氏距离用MLP编码，然后与融合后的特征堆叠，输入由残差网络和全连接层构成的分类器，输出K条轨迹的置信度。  

## loss
<center> 
<img src="/laneGCN/loss.png"  width="150" />
</center>
损失函数由两部分构成，分类损失和回归损失。
<center> 
<img src="/laneGCN/cls_loss.png"  width="300" />
</center>
分类损失采用max-margin loss的形式，以期望最优轨迹的置信度最高。c<sub>m,k</sub>为第m个目标的第k条预测轨迹的置信度，c<sub>m,k<sup>^</sup></sub>为最优轨迹的置信度。
<center> 
<img src="/laneGCN/reg_loss.png"  width="300" />
</center>
<center> 
<img src="/laneGCN/smooth_l1_loss.png"  width="400" />
</center>
回归损失用最优轨迹与真实轨迹点的smooth l1损失之和表示。

## result
<center> 
<img src="/laneGCN/result.png"  width="400" />
</center>

## 参考
[层标准化](https://blog.csdn.net/sinat_34072381/article/details/106173365)