#!/usr/bin/env python
# -*- coding: utf-8 -*-
# @Author: Yukun Feng
# @Date: 2023-08-18

import copy
import random
from collections import deque
from dataclasses import dataclass, field
from enum import Enum, unique
from pathlib import Path
from time import time

import click
import matplotlib.pyplot as plt
import networkx as nx
import numpy as np
import pandas as pd
import yaml
from Kronecker_Generator.KroneckerGenerator import (InitMatrix,
                                                    generateStochasticKron)
from loguru import logger

BASIC_NODE_NUM = 2
MAX_INT = 32768

MessageBit = 32
BAND_Gbit_2_bit = 1024 * 1024 * 1024
TIME_ns_2_s = 1 / 1000 / 1000 / 1000
DIRECTION_NAME = ('WEST', 'SOUTH', 'EAST', 'NORTH')

FIVE_DIRECTION = range(5)


class DIRECTION:
    WEST = 0
    SOUTH = 1
    EAST = 2
    NORTH = 3
    LOCAL = 4


def reverse_direction(direct: DIRECTION):
    if direct == DIRECTION.WEST:
        return DIRECTION.EAST
    elif direct == DIRECTION.EAST:
        return DIRECTION.WEST
    elif direct == DIRECTION.SOUTH:
        return DIRECTION.NORTH
    elif direct == DIRECTION.NORTH:
        return DIRECTION.SOUTH
    else:
        return direct
    a = a


@dataclass
class Edge:
    dst_v: np.int64
    weight: np.float32


class RandomGraph:
    def __init__(self, numv, nume) -> None:
        self.vertex_num = numv
        self.edge_num = nume
        self.edge_map = {}

    def _add_edge(self, v0, v1, weight):
        if v0 not in self.edge_map:
            self.edge_map[v0] = [Edge(v1, weight)]
        else:
            self.edge_map[v0].append(Edge(v1, weight))

    def add_random_edges(self, nume):
        for _ in range(nume):
            v0 = random.randint(0, self.vertex_num - 1)
            v1 = random.randint(0, self.vertex_num - 1)
            weight = random.random()
            self._add_edge(v0, v1, weight)
            self._add_edge(v1, v0, weight)


class KroneckerGraph():
    def __init__(self, scale, factor, prob_mat=None) -> None:
        self.initial = InitMatrix(BASIC_NODE_NUM)
        self.initial.make()
        if prob_mat == None:
            self.initial.makeStochasticAB(0.7, 0.3)
        else:
            self.initial.makeStochasticCustom(probArr=np.array(prob_mat))
        self.scale = scale
        self.vertex_num = BASIC_NODE_NUM ** scale
        self.edge_num = self.vertex_num * factor

    def make_graph(self):
        self.graph = generateStochasticKron(
            self.initial, self.scale, deleteSelfLoopsForStats=True, customEdges=True, edges=self.edge_num)

    def edge_table(self):
        self.edge_map = {}
        edges = nx.edges(self.graph)
        for v0, v1 in edges:
            weight = random.random()
            if v0 not in self.edge_map:
                self.edge_map[v0] = [Edge(v1, weight)]
            else:
                self.edge_map[v0].append(Edge(v1, weight))

            if v1 not in self.edge_map:
                self.edge_map[v1] = [Edge(v0, weight)]
            else:
                self.edge_map[v1].append(Edge(v0, weight))

        return self.edge_map

    def plot_network(self):
        edge_array = np.array(nx.adjacency_matrix(self.graph).todense())
        plt.imshow(edge_array)
        plt.show()

    def plot_graph(self):
        fig = plt.figure()
        nx.draw(self.graph, pos=nx.circular_layout(self.graph), node_color='green', edge_color='green',
                node_size=8, width=1, alpha=1)
        fig.set_facecolor('black')
        plt.show()


class GraphValidator():
    def bfs(vertex_num, edge_map, root, bfs_tree):
        # bfs algo
        queue = [root]
        std_tree = []
        check = [False] * vertex_num
        check[root] = True
        while len(queue) > 0:
            a = []
            for _ in range(len(queue)):
                src_v = queue.pop(0)
                check[src_v] = True
                a.append(src_v)
                for edge in edge_map[src_v]:
                    if check[src_v]:
                        continue
                    queue.append(edge.dst_v)
            std_tree.append(a)

        # validation
        validation = True
        for level, sets in enumerate(std_tree):
            for v in sets:
                if bfs_tree[v][0] != level:
                    validation = False
                    logger.critical(
                        f'BFS Validation: Vertex {v} level {bfs_tree[v][0]}, std level {level}')

        if validation:
            logger.info(f'BFS Validation: Successful!!!')
        else:
            logger.critical(f'BFS Validation: Failed!!!')

    def sssp(vertex_num, edge_map, root, sssp_tree):
        # sssp algo
        dist = [MAX_INT] * vertex_num
        dist[root] = 0
        book = [False] * vertex_num
        book[root] = 0
        queue = [root]
        while len(queue) > 0:
            src_v = queue.pop(0)
            for edge in edge_map[src_v]:
                if dist[src_v] + edge.weight < dist[edge.dst_v]:
                    dist[edge.dst_v] = dist[src_v] + edge.weight
                    if not book[edge.dst_v]:
                        queue.append(edge.dst_v)
                        book[src_v] = True
            book[src_v] = False

        # # NOTE: bellman-ford (too slow)
        # for _ in range(vertex_num):
        #     for src_v in edge_map.keys():
        #         for edge in edge_map[src_v]:
        #             if dist[edge.dst_v] > dist[src_v] + edge.weight:
        #                 dist[edge.dst_v] = dist[src_v] + edge.weight

        # validation
        validation = True
        for i in range(vertex_num):
            if dist[i] != sssp_tree[i][0]:
                validation = False
                logger.critical(
                    f'SSSP Validation: Vertex {i} min_dist {sssp_tree[i][0]}, std min_dist {dist[i]}')

        if validation:
            logger.info(f'SSSP Validation: Successful!!!')
        else:
            logger.critical(f'SSSP Validation: Failed!!!')


@unique
class MESSAGE_TYPE(Enum):
    BFS = 0
    SSSP = 1


@dataclass
class MessageInChip:
    """_summary_
    type "BFS"
        info: (src_v.hop)
    type "SSSP"
        info: (src_v.dist)
    """
    process_id: int
    src_v: int
    type: MESSAGE_TYPE
    info: None


class OneChip:
    def __init__(self, chip_id, chip_msg_max) -> None:
        self.id = chip_id
        self.msg_max = chip_msg_max
        self.edge_count = 0
        self.vertex_count = 0
        self.vertex = []    # chip vertex
        self.edge_map = {}  # edges with the dst vertex in chip
        self.msg_queue = []
        self.msg_queue_next = [[] for _ in range(5)]
        self.neighbor_chips = {}  # {OneChip * 4}
        self.routing_table = {}  # Dict{routing: {src_v: <W, S, E, N, Local>}}
        self.vertex_to_npus = []
        self.bfs_record = {}  # bfs result recording
        self.sssp_record = {}  # sssp result recording
        """_summary_
        bfs_record = Dict{Dict{(vertex.hop, vertex.from)}}
            Dict{process_id: {vertex: {(hop, from)}}}
        sssp_record = Dict{Dict{(vertex.dist, edge.src_v, edge.weight)}}
            Dict{process_id: {vertex: {(dist, from, weight)}}}
        """

    def add_edge(self, src_v, dst_v, weight):
        if dst_v not in self.vertex:
            logger.error(
                f'Cannot add Edge({src_v}, {dst_v}), Vertex {dst_v} is not in Chip {self.id}')
            exit(1)
        if src_v in self.edge_map:
            self.edge_map[src_v].append(Edge(dst_v, weight))
        else:
            self.edge_map[src_v] = [Edge(dst_v, weight)]

    def add_vertex(self, src_v):
        if src_v not in self.vertex:
            self.vertex.append(src_v)

    def add_routing(self, src_id, routing):
        if src_id not in self.routing_table:
            self.routing_table[src_id] = routing
        else:
            logger.warning(
                f'chip {self.id}: src_id {src_id} routing already exist <{self.routing_table[src_id]}>, new_coming <{routing}>')
            exit(1)

    def msg_forward(self, msg: MessageInChip):
        for direct in FIVE_DIRECTION[:4]:
            if self.routing_table[self.vertex_to_npus[msg.src_v]][direct]:
                self.neighbor_chips[direct].msg_queue_next[reverse_direction(
                    direct)].append(msg)
                # logger.info(
                #     f'Chip {self.id}: Forward to Chip {self.neighbor_chips[direct].id}.')

    def msg_broadcast(self, process_id, src_v, msg_type, msg_info):
        message = MessageInChip(process_id, src_v, msg_type, msg_info)
        self.msg_queue_next[DIRECTION.LOCAL].append(message)

    def bfs_update(self, process_id, src_v, dst_v, src_hop):
        """bfs_record = Dict{process_id: {vertex: {(hop, from)}}}
        """
        if dst_v not in self.vertex:
            logger.warning(
                f'Chip {self.id} Update: vertex {dst_v} not in this chip.')
            exit(1)

        if process_id not in self.bfs_record:
            self.bfs_record[process_id] = {}

        if dst_v not in self.bfs_record[process_id]:
            self.bfs_record[process_id][dst_v] = (
                src_hop, src_v)
            return True, self.bfs_record[process_id][dst_v][0]

        if src_hop < self.bfs_record[process_id][dst_v][0]:
            self.bfs_record[process_id][dst_v] = (
                src_hop, src_v)
            return True, self.bfs_record[process_id][dst_v][0]

        return False, 0

    def sssp_update(self, process_id, src_v, dst_v, weight, src_dist):
        """sssp_record = Dict{process_id: {vertex: {(dist, from, weight)}}}
        """
        if dst_v not in self.vertex:
            logger.warning(
                f'Chip {self.id} Update: vertex {dst_v} not in this chip.')
            exit(1)

        if process_id not in self.sssp_record:
            self.sssp_record[process_id] = {}

        if dst_v not in self.sssp_record[process_id]:
            self.sssp_record[process_id][dst_v] = (
                src_dist + weight, src_v, weight)
            return True, self.sssp_record[process_id][dst_v][0]

        if src_dist + weight < self.sssp_record[process_id][dst_v][0]:
            self.sssp_record[process_id][dst_v] = (
                src_dist + weight, src_v, weight)
            return True, self.sssp_record[process_id][dst_v][0]

        return False, 0

    def msg_parse(self):
        if len(self.msg_queue) == 0:
            return 0
        report_edge_num = 0
        for msg in self.msg_queue:
            if self.routing_table[self.vertex_to_npus[msg.src_v]][DIRECTION.LOCAL]:
                if msg.src_v in self.edge_map:
                    # logger.info(
                    #     f'Chip {self.id}: Received message, src_v {msg.src_v} connected to {len(self.edge_map[msg.src_v])} edges')
                    for edge in self.edge_map[msg.src_v]:
                        # logger.info(
                        #     f'Chip {self.id}: Report edge<{msg.src_v}, {edge.dst_v}> weight {edge.weight}')
                        report_edge_num = report_edge_num + 1
                        # update
                        if msg.type == MESSAGE_TYPE.BFS:
                            update_flag, update_info = self.bfs_update(
                                msg.process_id, msg.src_v, edge.dst_v, msg.info+1)
                        elif msg.type == MESSAGE_TYPE.SSSP:
                            update_flag, update_info = self.sssp_update(
                                msg.process_id, msg.src_v, edge.dst_v, edge.weight, msg.info)
                        else:
                            exit(1)

                        # new broadcas
                        if update_flag:
                            self.msg_broadcast(
                                msg.process_id, edge.dst_v, msg.type, update_info)

            # forward
            self.msg_forward(msg)
        return report_edge_num

    def msg_queue_update(self):
        self.msg_queue = []
        for direct in FIVE_DIRECTION:
            if len(self.msg_queue_next[direct]) > self.msg_max and direct != DIRECTION.LOCAL:
                # logger.warning(
                #     f'Chip {self.id}: Message Queue [{DIRECTION_NAME[direct]}] is full load!')
                self.msg_queue = self.msg_queue + \
                    self.msg_queue_next[direct][:self.msg_max]
                self.msg_queue_next[direct] = self.msg_queue_next[direct][self.msg_max:]
            else:
                self.msg_queue = self.msg_queue + \
                    self.msg_queue_next[direct]
                self.msg_queue_next[direct] = []

        return len(self.msg_queue)


class NPU:
    def __init__(self, config) -> None:
        # NPU Parameters
        # npu standard
        self.config = config
        self.X_NumOfChips = config['X_NumOfChips']
        self.Y_NumOfChips = config['Y_NumOfChips']
        self.NPUNodeNumMax = config['NPUNodeNumMax']
        self.NPUEdgeNumMax = config['NPUEdgeNumMax']

        self.ChipNPUNumMax = config['ChipNPUNumMax']
        self.ChipNodeNumMax = self.ChipNPUNumMax * self. NPUNodeNumMax
        self.ChipEdgeNumMax = self.ChipNPUNumMax * self. NPUEdgeNumMax

        self.InnerDelay = config['InnerDelay(ns)'] * TIME_ns_2_s
        self.InterDelay = config['InterDelay(ns)'] * TIME_ns_2_s
        self.HostDelay = config['ChipHostDelay(ns)'] * TIME_ns_2_s

        self.InnerBandMax = int(config['InnerBandWid(Gbps)'] *
                                self.InnerDelay * BAND_Gbit_2_bit / MessageBit / 2)
        self.InterBandMax = int(config['InterBandWid(Gbps)'] * self.InterDelay *
                                BAND_Gbit_2_bit / MessageBit / 2)
        self.HostBandMax = int(config['ChipHostDelay(Gbps)'] * self.HostDelay *
                               BAND_Gbit_2_bit / MessageBit / 2)

        # init chip matrix
        self.TotalChips = self.X_NumOfChips * self.Y_NumOfChips
        self.chip_matrix = [OneChip(i, self.InterBandMax) for i in range(
            self.TotalChips)]

        # npus into chips
        self.npus_to_chips = []
        for i in range(self.TotalChips):
            self.npus_to_chips.extend([i] * self.ChipNPUNumMax)

        # build chip connection
        for i in range(self.TotalChips):
            x, y = self.convert_id_to_xy(i)
            if x > 0:
                self.chip_matrix[i].neighbor_chips[DIRECTION.NORTH] = self.chip_matrix[i -
                                                                                       self.Y_NumOfChips]
            if x < self.X_NumOfChips-1:
                self.chip_matrix[i].neighbor_chips[DIRECTION.SOUTH] = self.chip_matrix[i +
                                                                                       self.Y_NumOfChips]
            if y > 0:
                self.chip_matrix[i].neighbor_chips[DIRECTION.WEST] = self.chip_matrix[i - 1]
            if y < self.Y_NumOfChips - 1:
                self.chip_matrix[i].neighbor_chips[DIRECTION.EAST] = self.chip_matrix[i + 1]

        # check connection
        # dire = ['W', 'S', 'E', 'N']
        # for i in range(self.TotalChips):
        #     print(f'chip {i}: connection <W, S, E, N>')
        #     for j in FIVE_DIRECTION:
        #         if j in self.chip_matrix[i].neighbor_chips:
        #             print(
        #                 f'    {dire[j]}: {self.chip_matrix[i].neighbor_chips[j].id}')

        ###################################################################
        self.used_chip_num_x = 1
        self.used_chip_num_y = 1

        # dirction [0, 1, 2, 3] = [N, E, S, W]
        self.router_load = np.zeros(
            (self.TotalChips, 5), dtype=np.int64)

    def gen_vertex_layout(self, vertex_num):
        used_chip_num = vertex_num // self.ChipNodeNumMax
        if used_chip_num * self.ChipNodeNumMax < vertex_num:
            used_chip_num = used_chip_num + 1
        if self.TotalChips >= used_chip_num:
            logger.info(
                f'Total chip_num = {self.TotalChips}, used_chip_num = {used_chip_num}')
        else:
            logger.error(
                f'Total chip_num = {self.TotalChips}, used_chip_num = {used_chip_num}')
            exit(1)

        # vertex into npus/chips
        self.vertex_to_npus = []
        self.vertex_to_chips = []
        for i in range(vertex_num):
            npu_id = i // self.NPUNodeNumMax
            # locat = i % used_chip_num
            self.vertex_to_npus.append(npu_id)
            self.vertex_to_chips.append(self.npus_to_chips[npu_id])
            self.chip_matrix[self.npus_to_chips[npu_id]].add_vertex(i)

        # apply to all chips
        for chip in self.chip_matrix:
            chip.vertex_to_npus = self.vertex_to_npus

    def convert_id_to_xy(self, chip_id):
        chip_x = chip_id // self.Y_NumOfChips
        chip_y = chip_id % self.Y_NumOfChips
        return chip_x, chip_y

    def get_routing(self, src_id, dst_id_list):
        """_summary_

        Parameters
        ----------
        src_id : int
            Routing source chip id
        dst_id_list : list [id_0, id_1, ...]
            Routing target chips id collaction

        """
        routing = np.zeros((self.TotalChips, 5))
        routing_record = [routing]
        path_lens = [-1] * self.TotalChips
        src_x, src_y = self.convert_id_to_xy(src_id)
        dst_set = []
        src_set = [src_id]
        path_lens[src_id] = 0

        # straight forward
        for dst_id in dst_id_list:
            dst_x, dst_y = self.convert_id_to_xy(dst_id)

            # get direction
            diff_x = src_x - dst_x
            diff_y = src_y - dst_y

            if diff_x != 0 and diff_y != 0:
                dst_set.append(dst_id)
                continue

            if diff_x < 0:
                direct = DIRECTION.SOUTH    # South
            elif diff_x > 0:
                direct = DIRECTION.NORTH    # North
            elif diff_y < 0:
                direct = DIRECTION.EAST    # East
            elif diff_y > 0:
                direct = DIRECTION.WEST    # West

            diff = abs(diff_x) + abs(diff_y)
            current_id = src_id
            current_dist = 0
            for i in range(diff):
                routing[current_id, direct] = 1
                current_id = self.chip_matrix[current_id].neighbor_chips[direct].id
                src_set.append(current_id)
                current_dist = current_dist + 1
                path_lens[current_id] = current_dist
            routing[current_id, DIRECTION.LOCAL] = 1
            routing_record.append(routing)

        # logger.info(
        #     f'Finish straight forward, routing_record: \n{np.array(routing)}')

        # non-straight forward
        src_set = list(set(src_set))
        loop_cnt = -1
        while len(dst_set) > 0:
            loop_cnt = loop_cnt + 1
            # logger.info(
            #     f'\n\n===================Non-Straight Forward: Loop {loop_cnt}===================')
            # logger.info(f'src_set: {src_set}')
            # logger.info(f'dst_set: {dst_set}')
            src_cands = []
            dst_cands = []
            dist_cands = []

            # l1_dist
            for dst_id in dst_set:
                dst_x, dst_y = self.convert_id_to_xy(dst_id)
                l1_dist = abs(src_x - dst_x) + abs(src_y - dst_y)

                dist = []
                for src in src_set:
                    x, y = self.convert_id_to_xy(src)
                    dist.append(abs(dst_x-x) + abs(dst_y-y))

                find_flag = False
                while not find_flag:
                    min_dist = min(dist)
                    src_cands_tmp = [s
                                     for i, s in enumerate(src_set) if dist[i] == min_dist]
                    dist_cands_tmp = np.array([
                        path_lens[i] for i in src_cands_tmp]) + min_dist
                    # check is l1_dist
                    bypass_filted_idx = []
                    for i, d in enumerate(dist_cands_tmp):
                        if d == l1_dist or 1:
                            bypass_filted_idx.append(i)
                    if len(bypass_filted_idx) > 0:
                        src_cands.extend([src_cands_tmp[i]
                                         for i in bypass_filted_idx])
                        dst_cands.extend(
                            len([src_cands_tmp[i] for i in bypass_filted_idx]) * [dst_id])
                        dist_cands.extend(
                            len([src_cands_tmp[i] for i in bypass_filted_idx]) * [min_dist])
                        find_flag = True

            # logger.info(f'src_cands = {src_cands}')
            # logger.info(f'dst_id = {dst_cands}')

            # choose min cands
            min_dist = min(dist_cands)
            min_src_dist = self.TotalChips
            for s, d, di in zip(src_cands, dst_cands, dist_cands):
                if di == min_dist and path_lens[s] < min_src_dist:
                    filted_src_cand = s
                    filted_dst_cand = d
                    min_src_dist = path_lens[s]

            # logger.info(
            #     f'Filted src_id = {filted_src_cand}, dst_id = {filted_dst_cand}')

            # search path
            src_cand_x, src_cand_y = self.convert_id_to_xy(filted_src_cand)
            dst_cand_x, dst_cand_y = self.convert_id_to_xy(filted_dst_cand)
            diff_x = src_cand_x - dst_cand_x
            diff_y = src_cand_y - dst_cand_y

            if diff_x <= 0:
                direct_x = DIRECTION.SOUTH    # South
            else:
                direct_x = DIRECTION.NORTH    # North

            if diff_y <= 0:
                direct_y = DIRECTION.EAST    # East
            else:
                direct_y = DIRECTION.WEST    # West

            diff_x = abs(diff_x)
            diff_y = abs(diff_y)

            # current location
            current_id = filted_src_cand
            current_dist = path_lens[current_id]

            while diff_x > 0 or diff_y > 0:
                if (self.router_load[current_id, direct_x] > self.router_load[current_id, direct_y] or diff_x <= 0) and diff_y > 0:
                    diff_y = diff_y - 1
                    routing[current_id, direct_y] = 1
                    current_id = self.chip_matrix[current_id].neighbor_chips[direct_y].id
                    src_set.append(current_id)

                    current_dist = current_dist + 1

                    path_lens[current_id] = current_dist
                else:
                    diff_x = diff_x - 1
                    routing[current_id, direct_x] = 1
                    current_id = self.chip_matrix[current_id].neighbor_chips[direct_x].id
                    src_set.append(current_id)
                    current_dist = current_dist + 1
                    path_lens[current_id] = current_dist
            routing[current_id, DIRECTION.LOCAL] = 1
            routing_record.append(routing)
            if filted_dst_cand == current_id:
                dst_set.remove(current_id)
            else:
                logger.error(
                    f'routing final_id {current_id} is not equal to dst_id {filted_dst_cand}')
                exit(1)
            src_set = list(set(src_set))
            # src_set = sorted(src_set)
            # logger.info(f'src_set: {src_set}')
            # logger.info(f'dst_set: {dst_set}')
            # logger.info(f'path_len: {path_lens}')
        # logger.info(
        #     f'\n\n===================Finish Non-Straight Forward===================')
        # logger.info(f'routing_record: \n{np.array(routing)}')
        return routing

    def gen_routing_layout(self, key, routing):
        for chip_id in range(self.TotalChips):
            route_info = routing[chip_id]
            if np.sum(route_info) == 0:
                continue
            self.chip_matrix[chip_id].add_routing(key, route_info)
            for direct in FIVE_DIRECTION:
                self.router_load[chip_id, direct] = self.router_load[chip_id,
                                                                     direct] + routing[chip_id][direct]

    def graph_mapping(self, vertex_num, edge_map):
        # TODO: 先不考虑边不够放的问题
        t1 = time()

        # vertex into npus and chips
        self.gen_vertex_layout(vertex_num)

        # edge into chips
        for src_v in edge_map:
            for edge in edge_map[src_v]:
                dst_id = self.vertex_to_chips[edge.dst_v]
                self.chip_matrix[dst_id].add_edge(
                    src_v, edge.dst_v, edge.weight)

        # connection into npu_flow
        self.npu_flows = {}
        for src_v in edge_map:
            for edge in edge_map[src_v]:
                src_npu = self.vertex_to_npus[src_v]
                dst_chip = self.vertex_to_chips[edge.dst_v]
                if src_npu in self.npu_flows:
                    self.npu_flows[src_npu].append(dst_chip)
                else:
                    self.npu_flows[src_npu] = [dst_chip]

        # npu routing into chips
        for src_npu in self.npu_flows.keys():
            src_id = self.npus_to_chips[src_npu]
            dst_id_list = self.npu_flows[src_npu]
            dst_id_list = list(set(dst_id_list))
            # logger.info(
            #     f'Routing npu {src_npu}: src_id {src_id}, dst_id {dst_id_list}')
            routing = self.get_routing(src_id, dst_id_list)
            self.gen_routing_layout(src_npu, routing)

        # # routing into chips
        # for src_v in edge_map:
        #     src_id = self.vertex_loc[src_v]
        #     dst_id_list = []
        #     for edge in edge_map[src_v]:
        #         dst_id_list.append(self.vertex_loc[edge.dst_v])
        #     dst_id_list = list(set(dst_id_list))
        #     logger.info(
        #         f'Routing start: src_id {src_id}, dst_id {dst_id_list}')
        #     routing = self.get_routing(src_id, dst_id_list)
        #     self.gen_routing_layout(src_v, routing)

        t2 = time()
        logger.info(
            f'Write graph successful! TIME COST = {t2-t1:.2f}s')

    def bfs(self, roots):
        t0 = time()
        if not isinstance(roots, list):
            roots = [roots]
        for i, src_v in enumerate(roots):
            src_id = self.vertex_to_chips[src_v]
            self.chip_matrix[src_id].bfs_update(
                i, src_v, src_v, 0)
            self.chip_matrix[src_id].msg_broadcast(
                i, src_v, MESSAGE_TYPE.BFS, 0)
        msg_num = 0
        for chip in self.chip_matrix:
            msg_num = msg_num + chip.msg_queue_update()
        all_msg_num = msg_num
        time_count = 0
        edge_count = 0
        while msg_num > 0:
            time_count = time_count + 1
            msg_num = 0
            # message process
            for chip in self.chip_matrix:
                edge_count = edge_count + chip.msg_parse()

            # message update
            for chip in self.chip_matrix:
                msg_num = msg_num + chip.msg_queue_update()
            all_msg_num = all_msg_num + msg_num

        finish_time = time_count * self.InterDelay
        GTEPS = edge_count / finish_time / BAND_Gbit_2_bit

        # Gather results
        bfs_trees = []
        for i in range(len(roots)):
            tree = {}
            for chip in self.chip_matrix:
                tree.update(chip.bfs_record[i])
            bfs_trees.append(tree)

        t1 = time()
        # BFS Report
        print(f'\n\n')
        logger.info(f'===================BFS Final Report===================')
        logger.info(
            f'BPU Total Chips Num: {self.X_NumOfChips} * {self.Y_NumOfChips}')
        logger.info(
            f'BPU InnerBandWid(Gbps): {self.config["InterBandWid(Gbps)"]}')
        logger.info(
            f'BPU InnerBandNumMax: {self.InterBandMax}')
        logger.info(f'BPU Chip Vertex Max Num: {self.ChipNodeNumMax}')
        logger.info(f'BPU Chip Edge Max Num: {self.ChipEdgeNumMax}')
        logger.info(f'BFS Message Processing Num: {all_msg_num}')
        logger.info(f'BFS Visit Edge Num = {edge_count}')
        logger.info(f'BFS Running Time(s): {finish_time}')
        logger.info(f'BFS GTEPS = {GTEPS}\n')
        logger.info(f'BFS simulation finished! TIME COST = {t1-t0:.2f}s')

        return bfs_trees

    def sssp(self, roots):
        t0 = time()
        if not isinstance(roots, list):
            roots = [roots]
        for i, src_v in enumerate(roots):
            src_id = self.vertex_to_chips[src_v]
            self.chip_matrix[src_id].sssp_update(i,
                                                 src_v, src_v, 0, 0)
            self.chip_matrix[src_id].msg_broadcast(
                i, src_v, MESSAGE_TYPE.SSSP, 0)
        msg_num = 0
        for chip in self.chip_matrix:
            msg_num = msg_num + chip.msg_queue_update()
        all_msg_num = msg_num
        time_count = 0
        edge_count = 0
        while msg_num > 0:
            time_count = time_count + 1
            msg_num = 0
            # message process
            for chip in self.chip_matrix:
                edge_count = edge_count + chip.msg_parse()

            # message update
            for chip in self.chip_matrix:
                msg_num = msg_num + chip.msg_queue_update()
            all_msg_num = all_msg_num + msg_num

        finish_time = time_count * self.InterDelay
        GTEPS = edge_count / finish_time / BAND_Gbit_2_bit

        # Gather results
        sssp_trees = []
        for i in range(len(roots)):
            tree = {}
            for chip in self.chip_matrix:
                tree.update(chip.sssp_record[i])
            sssp_trees.append(tree)

        t1 = time()
        # BFS Report
        print(f'\n\n')
        logger.info(f'===================SSSP Final Report===================')
        logger.info(
            f'BPU Total Chips Num: {self.X_NumOfChips} * {self.Y_NumOfChips}')
        logger.info(
            f'BPU InnerBandWid(Gbps): {self.config["InterBandWid(Gbps)"]}')
        logger.info(
            f'BPU InnerBandNumMax: {self.InterBandMax}')
        logger.info(f'BPU Chip Vertex Max Num: {self.ChipNodeNumMax}')
        logger.info(f'BPU Chip Edge Max Num: {self.ChipEdgeNumMax}')
        logger.info(f'SSSP Message Processing Num: {all_msg_num}')
        logger.info(f'SSSP Visit Edge Num = {edge_count}')
        logger.info(f'SSSP Running Time(s): {finish_time}')
        logger.info(f'SSSP GTEPS = {GTEPS}\n')
        logger.info(f'SSSP simulation finished! TIME COST = {t1-t0:.2f}s')

        return sssp_trees


def make_graph(scale, edgefactor, mode='random'):
    if mode == 'random':
        num_v = 2 ** scale
        num_e = num_v * edgefactor
        graph = RandomGraph(num_v, num_e)
        graph.add_random_edges(num_e)
        vertex_num = graph.vertex_num
        edge_map = graph.edge_map
    elif mode == 'kronecker':
        probArr = [1, 0.3, 0.3, 0.2]
        kg = KroneckerGraph(scale, edgefactor, probArr)
        kg.make_graph()
        edge_map = kg.edge_table()
        vertex_num = kg.vertex_num
    return vertex_num, edge_map


def evaluator(npu_config, scale, edgefactor):
    # make graph
    logger.info(
        f'Graph built start: <V, E> = <{2 ** scale}, {2 ** scale * edgefactor}>')
    t1 = time()
    vertex_num, edge_map = make_graph(scale, edgefactor, mode='random')
    t2 = time()
    logger.info(f'Graph generated successful! TIME COST = {t2-t1:.2f}s')

    # init npu
    # npu.graph_mapping(graph)
    npu = NPU(npu_config)
    npu.graph_mapping(vertex_num, edge_map)

    # bfs
    # root = random.randint(0, vertex_num-1)
    root = list(range(1))
    bfs_tree = npu.bfs(root)

    # sssp
    sssp_tree = npu.sssp(root)

    t3 = time()
    print(f'\n')

    # Validation
    GraphValidator.bfs(vertex_num, edge_map, root[0], bfs_tree[0])
    GraphValidator.sssp(vertex_num, edge_map, root[0], sssp_tree[0])

    logger.info(f'Simulation Running TIME: {t3-t1:.2f}s')
    logger.info(
        f'Graph: <V, E> = <{2 ** scale}, {2 ** scale * edgefactor}>')


@click.command()
@click.argument('npu-dir', type=Path)
@click.argument('scale', type=int)
@click.option('-e', '--edgefactor', default=16, type=int)
def process(npu_dir, scale, edgefactor):
    with open(npu_dir, 'r') as stream:
        npu_config = yaml.safe_load(stream)

    evaluator(npu_config, scale, edgefactor)


if __name__ == "__main__":
    process()
