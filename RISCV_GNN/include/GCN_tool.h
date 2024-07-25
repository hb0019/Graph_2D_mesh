#ifndef  __GCN_TOOL_H__
#define __GCN_TOOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "sMPI.h"
#include "numerical_tool.h"
#include "lbii.h"


struct vector{
    float *mat;
    uint32_t n;
};

struct matrix{
    vector *vec;
    uint32_t m, n;
};

struct node_feature{
    uint32_t *layer_num;
    uint32_t *node_num;
    uint32_t **degree;
    // matrix *feature;
    uint32_t *feature;
};

struct message_package{
    uint32_t layer_index;
    uint32_t node_index;
    vector node;
}

struct GCN_parameter{
    uint32_t *layer_num;
    uint32_t *layer_dimension;
    float *weight;
    float *bias;
}


void node_data_create(uint32_t *layer_dimension, struct node_feature *node_feature_data, int node_num);
void aggregator(struct message_package *rec_MessagePackage, struct node_feature *node_feature_data);
void message(struct message_package *send_MessagePackage, struct node_feature *node_feature_data, int *node_list, int node_num, int layer_index);


struct vector create_vector(int n){
    static vector T;
    T.mat = (float *)malloc(sizeof(float) * n);
    memset(T.mat, 0, sizeof(float) * n);
    T.n = n;

    return T;
}

struct matrix create_matrix(int m, int n){
    static matrix T;
    T.vec = (vector *)malloc(sizeof(vector) * m);
    for(int i = 0; i < m; i++){
        T.vec[i] = create_vector(n);
    }
    T.m = m;
    T.n = n;
    return T;
}

int vector_add(struct vector *a, struct vector *b, struct vector *c){
    if (a->n != b->n){
        printf("向量A和向量B的列数必须相同, a.n:%d b.n:%d\n", a->n, b->n);
        return -1;
    }
    for (int i=0; i<a->n; i++){
        float temp = float_add(a->mat[i], b->mat[i]);
        c->mat[i] =  float_add(temp, c->mat[i]);
    }
    return 0;
}

int matrix_mux(struct vector *a, struct matrix *b, struct vector *c){
    if (a->n != b->m){
        printf("向量A的列数和向量B的行数必须相同, a.n:%d b.m:%d\n", a->n, b->m);
        return -1;
    }
    for (int i=0; i<b->n; i++){
        for (int j=0; j<b->m; j++){
            float temp = float_mul(a->mat[j], b->vec[j].mat[i]);
            c->mat[i] = float_add(temp, c->mat[i]);
        }
    }
    return 0;
}

// 测试创建节点特征
void node_data_create(uint32_t *layer_dimension, struct node_feature *node_feature_data, int node_num){
    const uint8_t layer_num = sizeof(layer_dimension)/sizeof(layer_dimension[0]);
    
    node_feature_data->layer_num = layer_num;
    node_feature_data->degree = (uint16_t **)malloc(sizeof(uint16_t *) * layer_num);
    for (int i=0; i<layer_num; i++){
        node_feature_data->degree[i] = (uint16_t *)malloc(sizeof(uint16_t) * node_num);
        for (int j=0; j<node_num; j++){
            node_feature_data->degree[i][j] = 10;
        }
    }

    node_feature_data->feature = (matrix *)malloc(sizeof(matrix) * layer_num);
    for (int i=0; i<layer_num; i++){
        node_feature_data->feature[i] = create_matrix(node_num, layer_dimension[i]);
        for (int j=0; j<node_num; i++){
            for (int k=0; k<layer_dimension[i]; k++){
                if (i ==0){
                    node_feature_data->feature[i].vec[j].mat[k] = i+1;
                } else {
                    node_feature_data->feature[i].vec[j].mat[k] = 0;
                }
            }
        }
    }
    
    return;
}

// 测试节点特征聚合
void aggregator(struct message_package *rec_MessagePackage, struct node_feature *node_feature_data){
    uint8_t layer_index = rec_MessagePackage->layer_index;
    uint16_t node_index = rec_MessagePackage->node_index;
    int flag = vector_add(&node_feature_data->feature[layer_index].vec[node_index], &rec_MessagePackage->node, &node_feature_data->feature[layer_index].vec[node_index]);
    node_feature_data->degree[layer_index][node_index] = node_feature_data->degree[layer_index][node_index] - 1;
    if (node_feature_data->degree[layer_index][node_index] == 0){
        node_feature_data->feature[layer_index].vec[node_index] = node_feature_data->feature[layer_index].vec[node_index] * 10;
    }
    return;
}

// 测试发送节点数据
void message(struct message_package *send_MessagePackage, struct node_feature *node_feature_data, \
    int *node_list, int node_num, int layer_index){
    for (int i=0; i<node_num; i++){
        send_MessagePackage->layer_index = layer_index+1;
        send_MessagePackage->node_index = node_list[i];
        send_MessagePackage->node = node_feature_data->feature[layer_index].vec[node_list[i]];
        send_raw_pkt(&send_MessagePackage, 0, 0, 0, 0);
    }
    return;
}

void linearization(struct node_feature *node_feature_data){
    
}



void GCN_mem_init(struct node_feature *node_feature_data, struct GCN_parameter *GCN_parameter_data){
    GCN_parameter_data.layer_num = (volatile uint32_t *)RISCV_SRAM_BASE_ADDR(0);
    GCN_parameter_data.layer_dimension = (volatile uint32_t *)RISCV_SRAM_BASE_ADDR(1);
    node_feature_data.node_num = (volatile uint32_t *)RISCV_SRAM_BASE_ADDR(*(GCN_parameter_data.layer_num)+1);
    uint32_t *weight_addr = (volatile uint32_t *)RISCV_SRAM_BASE_ADDR(*(GCN_parameter_data.layer_num)+2);
    uint32_t *node_values = (volatile uint32_t *)RISCV_SRAM_BASE_ADDR(*weight_addr);
    uint32_t *dst_weight = (volatile uint32_t *)RISCV_SRAM_BASE_ADDR(*node_values);
    uint32_t *gcn_weight = (volatile uint32_t *)RISCV_SRAM_BASE_ADDR(*dst_weight);

    node_feature_data.feature = (volatile uint32_t *)node_values;
    GCN_parameter_data.weight = (volatile uint32_t *)gcn_weight;
    GCN_parameter_data.bias = (volatile uint32_t *)(gcn_weight+(*GCN_parameter_data.layer_dimension)*(*(GCN_parameter_data.layer_dimension+1)));

    

    return;
}

#endif

