import numpy as np
import torch 
import torch.optim as optim
import torch.nn.functional as F
import pandas as pd
import os 
from torch_geometric.data import Data
from model import GCN
import matplotlib.pyplot as plt
import networkx as nx

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


def dataset_create(data_path):
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

class Trainer():
    def __init__(self, model, optimizer, loss_fn, epochs):
        self.model = model
        self.optimizer = optimizer
        self.loss_fn = loss_fn
        self.epochs = epochs
        self.pos = {}

    def train(self, graph, train_dataset, test_dataset):
        tot_loss_array = []
        for t in range(self.epochs):
            self.optimizer.zero_grad()
            y_pred=self.model(graph.edge_attr, graph.x)
            # print(y_pred[train_dataset['input']])
            # print(train_dataset['label'], torch.stack(train_dataset['label'], 0))
            
            one_hot = torch.zeros(y_pred[train_dataset['input']].size(0), y_pred[train_dataset['input']].size(1)).long()
            one_hot.scatter_(1, torch.stack(train_dataset['label'], 0), torch.ones(one_hot.size(0), one_hot.size(1)).long())
            loss = self.loss_fn(y_pred[train_dataset['input']], torch.tensor(one_hot, dtype=torch.float32))
            acc = (torch.argmax(y_pred[train_dataset['input']], 1)==torch.stack(train_dataset['label'], 1)).sum().item()/len(train_dataset['label'])
            tot_loss_array.append(loss)
            test_acc, test_y_eval = self.eval(graph, test_dataset)
            print(f"epoch: {t}\t epoch_loss: {loss}\t total_loss: {sum(tot_loss_array)/len(tot_loss_array)}\t train_acc: {acc}\t test_acc:{test_acc}")
            loss.backward()
            self.optimizer.step()
            self.pos[t] = test_y_eval.detach().numpy()
        return 
    
    def save(self):
        torch.save(self.model, "model.pt")
        print("="*15, "\tarchitecture\t", "="*15)
        print(self.model)
        print("="*15, "\tparameters\t", "="*15)
        for name, param in self.model.named_parameters():
            print(name)
            print(param)
        return 
    
    def eval(self, graph, test_dataset):
        self.model.eval()   # 不启用batch_normalization和dropout
        y_eval=self.model(graph.edge_attr, graph.x)
        acc = (torch.argmax(y_eval[test_dataset['input']], 1)==torch.stack(test_dataset['label'], 1)).sum().item()/len(test_dataset['label'])
        # print(f"acc: {acc}")
        return acc, y_eval
    
    def draw(self, graph, t, color_flag=True):
        
        cls1color = '#1F78B4'
        cls2color = '#D20000'
        colors = []
        sub_pos = {}
        for i in range(len(self.pos[t])):
            sub_pos[i] = self.pos[t][i]
            cls = sub_pos[i].argmax()
            colors.append(cls1color if cls else cls2color)
        fig = plt.figure(dpi=150)
        fig.clf()
        ax = fig.subplots()
        ax.cla()
        ax.axis('off')
        ax.set_title('Epoch: %d' % t)
        
        if color_flag:
            nx.draw_networkx(graph, pos=pos, with_labels=True, node_color=colors)
        else:
            nx.draw_networkx(graph, pos=pos, with_labels=True)
        plt.show()
        
        return 

if __name__ == "__main__": 
    DATA_PATH = "/root/git/Graph_2D_mesh/data/karate"
    Graph = dataset_create(DATA_PATH)
    model = GCN(Graph['graph'].x.shape[1], 4, 2)
    trainer = Trainer(model=model, optimizer=optim.Adam(model.parameters(), lr=0.01), loss_fn=F.cross_entropy, epochs=500)
    trainer.train(Graph['graph'], Graph['train_dataset'], Graph['test_dataset'])
    trainer.save()
    # trainer.eval(Graph['graph'], Graph['test_dataset'])

    g = nx.Graph()
    src = Graph['graph'].edge_index[0].numpy()
    dst = Graph['graph'].edge_index[1].numpy()
    edgelist = zip(src, dst)
    for i, j in edgelist:
        g.add_edge(i, j)
    # pos = nx.kamada_kawai_layout(g)
    # pos = nx.spring_layout(g)
    pos = nx.spectral_layout(g)
    #使用Kamada-Kawai路径长度代价函数生成布局
    trainer.draw(g, 0)
    trainer.draw(g, 499)