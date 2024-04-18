import torch
import torch.nn as nn
import numpy as np
import networkx as nx
import torch.optim as optim
import torch.nn.functional as F
import pandas as pd
import matplotlib.pyplot as plt


def create_Graphs_with_attributes(edgelist_filepath,attributes_filepath):
    #通过边列表构造一个图
    graph = nx.read_edgelist(edgelist_filepath,nodetype=int)
    #读取节点的属性，并设置属性表的索引列名为node
    attributes = pd.read_csv(attributes_filepath,index_col=['node'])
    #获取属性值
    #attributes_values = a:{b}for a,b in enumerate(attributes.values)
    #         print("a:",a)
    #         print("b:",b )
    #         attributes_values = {a:{'role':b[0],'community':b[1]}}
    attributes_values = {a:{'role':b[0],'community':b[1]} for a,b in enumerate(attributes.values)}
    #print(attributes_values)
    #设置节点的属性

    nx.set_node_attributes(graph,attributes_values)
    return graph

graph= create_Graphs_with_attributes('/root/git/Graph_2D_mesh/data/karate/edges.csv','/root/git/Graph_2D_mesh/data/karate/attributes.csv')

print(graph.edges())

nx.draw(graph,with_labels=True,node_color=(0.5294,0.804,0.9215),node_size=500)

plt.show()


