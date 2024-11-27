## 神经网络是什么  
神经网络是一个描述输入输出关系的非线性模型。  
通过线性和非线性操作对输入信息进行处理映射到输出，
<center> 
![nn](/nn/nn.png)  
</center> 

## 感知机  
<center> 
![mlp](/nn/mlp.png)  
</center> 
感知机是神经网络的基本构成元素，
<center> 
![perceptron](/nn/perceptron.png)  
</center> 
其中y,b为常数，x,w为向量，
<center> 
![perceptron_interp](/nn/perceptron_interp.png)  
</center> 
f是非线性的函数，被称为激活函数，可以用Logistic/Sigmoid函数、tanh函数、ReLU函数等,  比如ReLU,  
<center> 
![relu](/nn/relu.png)  
</center>
<center> 
![relu_img](/nn/relu_img.png)  
</center>  

## 前向传播  
在已知W，b时，通过线性和非线性操作对输入信息进行处理直至得到输出的过程。  
<center> 
![fp](/nn/fp.png)  
</center>  

## 反向传播  
模型训练是一个已知x,y,f，求W，b的问题。
训练过程中通过前向传播结果y'与y的差值，更新W和b, 逐渐减小y'与y的差值的过程。
这个过程的核心是反向传播。   
反向传播的核心是求解损失对模型参数（W,b）的导数，以一个感知机为例，已知，
<center> 
![bp_info](/nn/bp_info.png)  
</center>
那么，
<center> 
![bp](/nn/bp.png)  
</center>
其中，
<center> 
![relu_derive](/nn/relu_derive.png)  
</center>
<center> 
![w_derive](/nn/w_derive.png)  
</center>
<center> 
![b_derive](/nn/b_derive.png)  
</center>
loss的导数根据loss的计算方式决定。  
由于损失对l层参数的偏导依赖l+1层的计算结果，因此从最后一层依次计算损失对参数的偏导。