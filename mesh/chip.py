import numpy as np
from collections import defaultdict
import torch
import re 
from dataset import create_features, create_adjacency_matrix, label_create, dataset_create
import pandas as pd
from torch_geometric.data import Data
import os

import sys
sys.path.append("/root/git/Graph_2D_mesh/norm")


class Memory:
    def __init__(self, gcn_model_path, graph_data):
        self.Conv_weights = list()

        self.graph_nodes = list()
        self.weight_index = dict()
        self.retrieval_index = defaultdict(list)

        self.initialize_memory(gcn_model_path, graph_data)
        
    def init_gcn_weight(self, gcn_model_path):
        # load gcn weight
        GCN_model = torch.load(gcn_model_path)
        GCN_parameters = GCN_model.state_dict()
        GCN_architectur = [x for x in GCN_model.named_children()] 
        Conv_lenght = len([layer for layer in GCN_architectur if 'layer' in layer[0]])
        self.Conv_weights = [dict() for _ in range(Conv_lenght)]
        for layer_parameter in GCN_parameters:
            if 'layer' in layer_parameter:
                layer_index = int(re.findall(r"layer(.+?)\.", layer_parameter)[0])
                assert layer_index<=Conv_lenght, "Error: model layer index out of range"
                layer_name = re.findall(r"layer.*\.(.+)$", layer_parameter)
                self.Conv_weights[layer_index-1][layer_name[0]] = GCN_parameters[layer_parameter]
    
    def init_graph(self, graph_data):
        # load graph
        assert len(self.Conv_weights)>0, "Error: Conv_weights is empty. Please initialize Conv_weights first"
        features_backup_num = len(self.Conv_weights)
        self.graph_nodes.append({})
        for _ in range(features_backup_num):
            self.graph_nodes.append(None)
        for i in range(graph_data.index.shape[0]):
            node_index = graph_data.index[i].item()
            self.graph_nodes[0][node_index] = graph_data.x[i] 
            self.weight_index[node_index] = 1/torch.sqrt(graph_data.edge_attr[node_index].sum()).item() # 对index节点的度求根号取倒数
            
            for Adj_node_index in range(graph_data.edge_attr[node_index].shape[0]):
                if graph_data.edge_attr[node_index][Adj_node_index]==1:
                    self.retrieval_index[node_index].append(Adj_node_index)

    def initialize_memory(self, gcn_model_path, graph_data):
        self.init_gcn_weight(gcn_model_path)
        self.init_graph(graph_data)
        # print(self.graph_nodes)
        # print(self.weight_index)
        print(self.retrieval_index)
        print("Memory initialization complete!")


class Router:
    def __init__(self, in_graph_index, retrieval_index):
        self.in_graph_index = in_graph_index
        self.retrieval_index = retrieval_index
        

    def send_message(self, node_index, message):
        pass

    def receive_message(self, node_index, message):
        pass



class Chip:
    def __init__(self, gcn_model_path, graph_data):
        self.memory = Memory(gcn_model_path, graph_data)
        self.router = Router(graph_data.index)

    def message_stage(self):
        # TODO: 当前的状态栏，怎么确定不同层级的下一步
        try:
            layer_stage = self.memory.graph_nodes.index(None)
        except:
            print('stage overflow')
            layer_stage = len(self.memory.graph_nodes)-1
        # TODO: xxx
        graph_tensor = torch.stack(list(self.memory.graph_nodes[layer_stage-1].values()), dim=0)
        self.memory.graph_nodes[layer_stage] = torch.mm(graph_tensor, self.memory.Conv_weights[layer_stage-1]['W'])+self.memory.Conv_weights[layer_stage-1]['bias']
        D_tensor = torch.tensor([[item] for item in self.memory.weight_index.values()])
        self.memory.graph_nodes[layer_stage] = self.memory.graph_nodes[layer_stage] * D_tensor
        # print(self.memory.graph_nodes[layer_stage])
        # message complete!
        return

    def send_stage(self):
        
        return

    def aggregation_stage(self):

        return

    def activate_stage(self):

        return

    def run(self):
        self.message_stage()
        pass 


def graph_data_split(graph_data, num=1):
    def order_split(index, num):
        part = [(i*index.shape[0]//num, (i+1)*index.shape[0]//num) for i in range(num)]
        part_index_list = [index[begin:end] for begin, end in part]
        return part_index_list
    def Data_extra(item_data, index):
        sub_item_data = item_data[index.data]
        return sub_item_data
    
    graph_data_list = list()
    part_index_list = order_split(graph_data.index, num)
    for index in part_index_list:
        sub_graph_data = Data(x=Data_extra(graph_data.x, index), 
                              edge_index=graph_data.edge_index,
                              edge_attr=graph_data.edge_attr,
                              y=Data_extra(graph_data.y, index),
                              index=index.reshape(-1,1))
        graph_data_list.append(sub_graph_data)
    
    return graph_data_list


if __name__ == "__main__":
    DATA_PATH = "/root/git/Graph_2D_mesh/data/karate"
    MODEL_PATH = "/root/git/Graph_2D_mesh/norm/model.pt"
    NUM_CHIP = 2
    dataset = dataset_create(DATA_PATH)
    graph_data = dataset['graph']
    graph_data.index = torch.tensor([i for i in range(dataset['graph'].x.shape[0])])
    graph_data_list = graph_data_split(graph_data, num=NUM_CHIP)

    # for index in range(NUM_CHIP):
    
    chip1 = Chip(gcn_model_path=MODEL_PATH, graph_data=graph_data_list[0])
    # chip2 = Chip(gcn_model_path=MODEL_PATH, graph_data=graph_data_list[1])

    chip1.run()
