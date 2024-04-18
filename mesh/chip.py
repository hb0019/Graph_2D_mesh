import numpy as np
import copy
from collections import defaultdict
import torch
import re 
from dataset import create_features, create_adjacency_matrix, label_create, dataset_create
import pandas as pd
from torch_geometric.data import Data
import os
import torch.nn.functional as F
from multiprocessing import Process, Pipe
from threading import Thread
from queue import Queue
import socket
import json

import sys
sys.path.append("/root/git/Graph_2D_mesh/norm")


class Memory:
    def __init__(self, gcn_model_path, graph_data):
        self.Conv_weights = list()

        self.graph_nodes = list()
        self.weight_index = dict()
        self.retrieval_index = defaultdict(list)
        self.count_index = dict()

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
            self.count_index[node_index] = 0
            
            for Adj_node_index in range(graph_data.edge_attr[node_index].shape[0]):
                if graph_data.edge_attr[node_index][Adj_node_index]==1:
                    self.retrieval_index[node_index].append(Adj_node_index)
        self.graph_nodes[0] = torch.stack(list(self.graph_nodes[0].values()), dim=0)

    def initialize_memory(self, gcn_model_path, graph_data):
        self.init_gcn_weight(gcn_model_path)
        self.init_graph(graph_data)
        print(self.graph_nodes)
        print(self.weight_index)
        print(self.retrieval_index)
        print(graph_data.index)
        print("Memory initialization complete!")


class Router:
    def __init__(self, in_graph_index, graph_attr, retrieval_index, num_list_id):
        self.in_graph_index = in_graph_index
        self.graph_attr = copy.deepcopy(graph_attr[self.in_graph_index.reshape(-1)])
        self.retrieval_index = retrieval_index
        self.package = Queue(maxsize=100)
        self.num_list_id = num_list_id
        self.get_routing()

    def get_routing(self):
        # self.routing = np.zeros((self.in_graph_index.shape[0], 2), dtype=bool)
        # for i, retrieval_info in enumerate(self.retrieval_index.items()):
        #     src_id, dst_id_list = retrieval_info
        #     if src_id in dst_id_list:
        #         self.routing[i][0] = True
        #     if any([item!=src_id for item in dst_id_list]):
        #         self.routing[i][1] = True

        # self.routing = np.zeros((sum(sum(self.graph_attr)), len(self.retrieval_index)), dtype=int)
        self.routing = {}
        for key, value in self.retrieval_index.items():
            for item in value:
                if self.routing.get(item, None) is None:
                    self.routing[item] = [key]
                else:
                    self.routing[item].append(key)
        return 
        
    def send_message(self, message):
        udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        dst_addrs = [('0.0.0.0', 20000+i) for i in self.num_list_id]
        json_string = json.dumps(message).encode()
        for dst_addr in dst_addrs:
            udp_socket.sendto(json_string, dst_addr)
        udp_socket.close() 

    def receive_message(self):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind(('0.0.0.0', 20000+self.num_list_id[0]))
        try:
            while True:
                ret, addr = sock.recvfrom(10240)
                if ret:
                    self.package.put(ret)
        except:
            sock.close()

class Chip:
    def __init__(self, gcn_model_path, graph_data, num_list_id):
        self.memory = Memory(gcn_model_path, graph_data)
        self.router = Router(graph_data.index, graph_data.edge_attr, self.memory.retrieval_index, num_list_id)
        self.graph_data = graph_data

    def message_stage(self):
        # TODO: 通过当前的状态栏，确定不同层级的下一步状态
        try:
            layer_stage = self.memory.graph_nodes.index(None)
        except:
            print('stage overflow')
            layer_stage = len(self.memory.graph_nodes)-1
        # graph_tensor = torch.stack(list(self.memory.graph_nodes[layer_stage-1].values()), dim=0)
        graph_tensor = self.memory.graph_nodes[layer_stage-1]
        self.memory.graph_nodes[layer_stage] = torch.mm(graph_tensor, self.memory.Conv_weights[layer_stage-1]['W'])+\
            self.memory.Conv_weights[layer_stage-1]['bias']
        D_tensor = torch.tensor([[item] for item in self.memory.weight_index.values()])
        self.memory.graph_nodes[layer_stage] = self.memory.graph_nodes[layer_stage] * D_tensor
        # print(self.memory.graph_nodes[layer_stage])
        # message complete!
        return layer_stage

    def send_stage(self, layer_stage):
        message = {'graph_nodes':self.memory.graph_nodes[layer_stage].tolist(), \
                   'graph_index':self.graph_data.index.tolist(), \
                   'layer_stage':layer_stage}
        # print(message)
        self.router.send_message(message)
        return

    def aggregation_stage(self, layer_stage):
        while True:
            message = self.router.package.get()
            message = json.loads(message.decode())
            # TODO: if message['layer_stage'] != layer_stage:
            for i, scr_id in enumerate(message['graph_index']):
                for dst_id in self.router.routing.get(scr_id[0], []):
                    h_dst_node = self.memory.weight_index[dst_id]*torch.tensor(message['graph_nodes'][i])
                    # print(dst_id)
                    # print(self.graph_data.index.tolist().index([dst_id]))
                    self.memory.graph_nodes[layer_stage][self.graph_data.index.tolist().index([dst_id])] = \
                        self.memory.graph_nodes[layer_stage][self.graph_data.index.tolist().index([dst_id])] + h_dst_node
                    self.memory.count_index[dst_id] += 1
            # 判断当前阶段是否收集完成所有邻接边传来的特征
            if self.complete_agg():
                return
        
    def complete_agg(self):
        for id, count in self.memory.count_index.items():
            if sum(self.graph_data.edge_attr[id]).item() > count:
                return False
        for id in self.memory.count_index.keys(): 
            self.memory.count_index[id] = 0
        return True

    def activate_stage(self, layer_stage):
        if layer_stage != len(self.memory.graph_nodes)-1:
            self.memory.graph_nodes[layer_stage] = F.relu(self.memory.graph_nodes[layer_stage])
        else:
            self.memory.graph_nodes[layer_stage] = F.softmax(self.memory.graph_nodes[layer_stage])
        
    
    def action(self):
        while None in self.memory.graph_nodes:
            layer_stage = self.message_stage()
            self.send_stage(layer_stage)
            self.aggregation_stage(layer_stage)
            self.activate_stage(layer_stage)
        # print(self.memory.graph_nodes[-1])
        # print(self.graph_data.index, self.graph_data.y)
        result = torch.argmax(self.memory.graph_nodes[-1], dim=1)
        index = self.graph_data.index.reshape(-1)
        label = self.graph_data.y.reshape(-1)
        print(f'index: {index}')
        print(f'label: {label}')
        print(f'result: {result}')
        print(label==result)
        print("Action complete!")
        return 

    def run(self):
        t1 = Thread(target=self.router.receive_message, args=())
        t2 = Thread(target=self.action, args=())
        t1.start()
        t2.start()
        print("Chip running...")
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

    chip_matrix = []
    id_list = [i for i in range(NUM_CHIP)]
    for index in range(NUM_CHIP):
        chip_matrix.append(Chip(gcn_model_path=MODEL_PATH, graph_data=graph_data_list[index], num_list_id=id_list[index:]+id_list[:index]))
    
    # chip1 = Chip(gcn_model_path=MODEL_PATH, graph_data=graph_data_list[0], num_list_id=[1,2])
    # chip2 = Chip(gcn_model_path=MODEL_PATH, graph_data=graph_data_list[1], num_list_id=[2,1])

    process_list = []
    
    for handle in chip_matrix:
        p = Process(target=handle.run, args=())
        process_list.append(p)

    for p in process_list:
        p.start()
    
    # p1 = Process(target=chip1.run, args=())
    # p2 = Process(target=chip2.run, args=())
    
    # p1.start()
    # p2.start()

