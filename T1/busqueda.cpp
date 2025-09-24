#include "busqueda.h"
#include "btree.h"
#include <stdexcept>
#include <iostream>


void range_search_B_disk(DiskManager &dm, int node_idx, int l, int u, vector<pair<int,float>> &out) {
    if (node_idx == -1) return;
    Nodo node = dm.read_node_at(node_idx);
    if (!node.es_interno) {
        for (int i = 0; i < node.k; ++i)
            if (node.pares[i].llave >= l && node.pares[i].llave <= u)
                out.emplace_back(node.pares[i].llave, node.pares[i].valor);
    } else {
        for (int i = 0; i <= node.k; ++i)
            if (node.hijos[i] != -1)
                range_search_B_disk(dm, node.hijos[i], l, u, out);
    }
}

vector<pair<int,float>> range_search_Bplus_disk(DiskManager &dm, int root_idx, int l, int u) {
    if (root_idx == -1) return {};
    int cur_idx = root_idx;
    while (true) {
        Nodo node = dm.read_node_at(cur_idx);
        if (!node.es_interno) {
            vector<pair<int,float>> out;
            int iter_idx = cur_idx;
            while (iter_idx != -1) {
                Nodo leaf = dm.read_node_at(iter_idx);
                for (int i = 0; i < leaf.k; ++i) {
                    if (leaf.pares[i].llave >= l && leaf.pares[i].llave <= u)
                        out.emplace_back(leaf.pares[i].llave, leaf.pares[i].valor);
                    else if (leaf.pares[i].llave > u)
                        return out;
                }
                iter_idx = leaf.siguiente;
            }
            return out;
        } else {
            int child = find_child_index(node, l);
            cur_idx = node.hijos[child];
        }
    }
}