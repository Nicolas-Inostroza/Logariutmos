#include "busqueda.h"
#include "btree.h"
#include <stdexcept>
#include <iostream>

/*
range_search_B_disk :: DiskManager, Int, Int, Int, vector<pair<Int,Float>>& -> Void
Realiza una busqueda de rango en un arbol B almacenado en disco.
*/
void range_search_B_disk(DiskManager &disck_manager, int node_idx, int l, int u, vector<pair<int,float>> &out, int &io_busquedas) {
    if (node_idx == -1) return;
    io_busquedas++;
    Nodo node = disck_manager.read_node_at(node_idx);
    if (!node.es_interno) {
        for (int i = 0; i < node.k; ++i)
            if (node.pares[i].llave >= l && node.pares[i].llave <= u)
                out.emplace_back(node.pares[i].llave, node.pares[i].valor);
    } else {
        for (int i = 0; i <= node.k; ++i)
            if (node.hijos[i] != -1)
                range_search_B_disk(disck_manager, node.hijos[i], l, u, out, io_busquedas);
    }
}

/*
range_search_Bplus_disk :: DiskManager, Int, Int, Int -> vector<pair<Int,Float>>
Realiza una busqueda de rango en un arbol B+ almacenado en disco.
*/
vector<pair<int,float>> range_search_Bplus_disk(DiskManager &disck_manager, int indice_raiz, int l, int u, int &io_busquedas) {
    if (indice_raiz == -1) return {};
    int indice_actual = indice_raiz;
    while (true) {
        io_busquedas++;
        Nodo node = disck_manager.read_node_at(indice_actual);
        if (!node.es_interno) {
            vector<pair<int,float>> out;
            int indice_iterador = indice_actual;
            while (indice_iterador != -1) {
                io_busquedas++;
                Nodo hoja = disck_manager.read_node_at(indice_iterador);
                for (int i = 0; i < hoja.k; ++i) {
                    if (hoja.pares[i].llave >= l && hoja.pares[i].llave <= u)
                        out.emplace_back(hoja.pares[i].llave, hoja.pares[i].valor);
                    else if (hoja.pares[i].llave > u)
                        return out;
                }
                indice_iterador = hoja.siguiente;
            }
            return out;
        } else {
            int child = find_child_index(node, l);
            indice_actual = node.hijos[child];
        }
    }
}