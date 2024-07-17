import numpy as np
import torch 
import torch.optim as optim
import torch.nn.functional as F
import pandas as pd
from model import GCN
from torch_geometric.data import Data
import os

def create_features(graph_node):
    if type(graph_node) is dict:
        node_size = len(graph_node.keys())
        X_1 = torch.eye(node_size)
        return X_1
    else:
        raise ValueError("Graph_node must be a dict or a Data object")

def create_adjacency_matrix(edges):
    nodel_size = 1+torch.max(edges)
    # I_matrix = torch.eye(nodel_size)
    I_matrix = torch.zeros([nodel_size, nodel_size])
    A_matrix = I_matrix.clone()
    for edge in edges:
        A_matrix[edge[0], edge[1]] = 1
        A_matrix[edge[1], edge[0]] = 1
    return A_matrix

def label_create(graph_node):
    if type(graph_node) is dict:
        node_size = len(graph_node.keys())
        labels = torch.zeros(node_size, 1)
        train_mask = torch.tensor(node_size*[False])
        for i, node in enumerate(graph_node.keys()):
            if graph_node[node]['role'] in ['Administrator', 'Instructor']:
                train_mask[i] = True
            if graph_node[node]['community'] == 'Administrator':
                labels[i] = 1
        return labels, train_mask
    else:
        raise ValueError("Graph_node must be a dict or a Data object")

def dataset_loader(data_path):
    attributes = pd.read_csv(os.path.join(data_path, 'attributes.csv'), index_col=['node'])
    attributes_values = {a:{'role':b[0],'community':b[1]} for a,b in enumerate(attributes.values)}
    nodes_features = create_features(attributes_values)
    labels, train_mask = label_create(attributes_values)
    edges = pd.read_csv(os.path.join(data_path, 'edges.csv'))
    edges_values = torch.tensor(np.concatenate((edges.values, edges.values[:,::-1])))
    A_matrix = create_adjacency_matrix(edges_values)

    graph=Data(x=nodes_features, edge_index=edges_values.t().contiguous(), edge_attr=A_matrix,
                y=labels, train_mask=train_mask)
    
    X_train, Y_train, X_test, Y_test=[], [], [], []
    for index in range(graph.train_mask.shape[0]):
        is_train = graph.train_mask[index]
        if is_train:
            X_train.append(index)
            Y_train.append(graph.y[index].long())
        # else:
        #     X_test.append(index)
        #     Y_test.append(graph.y[index].long())
        X_test.append(index)
        Y_test.append(graph.y[index].long())
    
    return dict(
        graph=graph,
        train_dataset={'input':X_train, 'label':Y_train},
        test_dataset={'input':X_test, 'label':Y_test},
    )

class Inferer():
    def __init__(self, model):
        self.model = model
        self.model.eval()
    
    def infer(self, graph, test_dataset):
        y_eval = self.model(graph.edge_attr, graph.x)
        acc = (torch.argmax(y_eval[test_dataset['input']], 1)==torch.stack(test_dataset['label'], 1)).sum().item()/len(test_dataset['label'])
        return acc, y_eval


if __name__ == '__main__':
    DATA_PATH = '/root/git/Graph_2D_mesh/data/karate'
    Graph = dataset_loader(DATA_PATH)
    # model = GCN(Graph['graph'].x.shape[1], 4, 2)
    model = torch.load('model.pt')
    inferer = Inferer(model=model)
    
    acc, y_eval = inferer.infer(Graph['graph'], Graph['test_dataset'])
    print(Graph['graph'].x)
    print(Graph['graph'].edge_attr)
    # print(Graph['test_dataset'])
    print(acc)
    # print(y_eval)