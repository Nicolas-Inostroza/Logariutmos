#ifndef BTREE_UTILS_H
#define BTREE_UTILS_H

#include "listanodo.h"
#include "nodo.h"

void insert_pair_in_node(Nodo &node, int key, float val);
int find_child_index(const Nodo &node, int key);


/*
SplitResult :: struct
Estructura que representa el resultado de dividir un nodo.
Contiene los nodos izquierdo y derecho, y el par llave-valor del medio.
*/
struct SplitResult {
    Nodo left, right;
    int med_llave;
    float med_valor;
};

SplitResult split_node(const Nodo &full, bool is_Bplus);
void insert(ListaNodo &arr, int &root_idx, int key, float val, bool is_Bplus);
void insert_recursive(ListaNodo &arr, int node_idx, int key, float val, bool is_Bplus);

#endif
