import torch
import torch.nn as nn
from torch.nn.parameter import Parameter
import numpy as np

class GCN_layer(nn.Module):
    def __init__(self, input_shape,output_shape):
        super(GCN_layer,self).__init__()
        self.W = Parameter(torch.rand(input_shape,output_shape))  #随机初始化权重
        self.bias = Parameter(torch.rand(output_shape))#随机初始化偏置

    def forward(self,Adj_matrix,input_feature):
        if type(Adj_matrix)==np.ndarray:
            A = torch.from_numpy(Adj_matrix).type(torch.LongTensor)
        elif type(Adj_matrix)==torch.Tensor:
            A = Adj_matrix.type(torch.LongTensor)
        else:
            raise ValueError("Adj_matrix must be numpy array or torch.Tensor")
        assert A.shape[0]== A.shape[1]  #相等就正常执行，不相等就报错
        I = torch.eye(A.shape[0])  #单位矩阵
        A_hat = A + I
        D = torch.sum(A_hat,0)
        D = torch.diag(D)  #以D构建对角矩阵
        # print(D.shape)
        D_inv = torch.sqrt(torch.inverse(D))  #求逆矩阵再求根号
        A_hat = torch.mm(torch.mm(D_inv,A_hat),D_inv)#D^(-1/2)*A_hat*D^(-1/2)
        aggregate = torch.mm(A_hat,input_feature) #D^(-1/2)*A_hat*D^(-1/2)*H^(l)
        propagate = torch.mm(aggregate,self.W)+self.bias  #再乘权重，加上权重
        #print("forward_GCN_layer")
        return propagate
    

class GCN(nn.Module):
    def __init__(self, input_shape, output_shape, n_classes, activation='Relu'):
        super(GCN,self).__init__()
        # 第一层
        self.layer1 = GCN_layer(input_shape,output_shape)
        # 第二层
        self.layer2 = GCN_layer(output_shape,n_classes)
        # 激活函数
        if activation =='Tanh':
            self.activation = nn.Tanh()
        elif activation=='Sigmoid':
            self.activation = nn.Sigmoid()
        elif activation=='Softmax':
            self.activation=nn.Softmax()
        elif activation=='Relu':
            self.activation=nn.ReLU()
        self.softmax=nn.Softmax(dim=1)   
    
    def forward(self, Adj_matrix, input_feature):
        x = self.layer1(Adj_matrix, input_feature)     #这里的参数对应与GCN_layer.forward()的参数
        x = self.activation(x) 
        x = self.layer2(Adj_matrix,x)
        x = self.softmax(x)     
        return x
    
    
