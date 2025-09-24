#ifndef BTREE_UTILS_H
#define BTREE_UTILS_H

#include "listanodo.h"
#include "nodo.h"

void insert_pair_in_node(Nodo &node, int key, float val);
int find_child_index(const Nodo &node, int key);

struct SplitResult {
    Nodo left, right;
    int med_llave;
    float med_valor;
};

SplitResult split_node(const Nodo &full, bool is_Bplus);
void insert(ListaNodo &arr, int &root_idx, int key, float val, bool is_Bplus);
void insert_recursive(ListaNodo &arr, int node_idx, int key, float val, bool is_Bplus);

#endif
