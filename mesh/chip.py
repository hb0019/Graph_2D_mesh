import numpy as np
from collections import defaultdict
import torch

class Roter:
    def __init__(self) -> None:
        pass


class Memory:
    def __init__(self) -> None:
        self.graph_nodes = defaultdict(list)
        self.Conv_weights = list(defaultdict(list))
        self.retrieval_index = defaultdict(list)
        self.weight_index = defaultdict(list)
    
    def init_gcn_weight(self, gcn_model_path):
        # load gcn weight
        GCN_model = torch.load(gcn_model_path)
        GCN_parameters = GCN_model.state_dict()
        GCN_architectur = [x for x in GCN_model.named_children()] 
        return 
    
    def init_graph(self):
        return 

    def initialize_memory(self, gcn_model_path):
        self.init_gcn_weight()
        self.init_graph()


class Chip:
    def __init__(self, size):
        pass




if __name__ == "__main__":
    pass

