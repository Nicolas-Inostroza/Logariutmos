#include "btree.h"
#include <stdexcept>
#include <iostream>

void insert_pair_in_node(Nodo &node, int llave, float valor) {
    int pos = 0;
    while (pos < node.k && node.pares[pos].llave < llave) pos++;
    for (int i = node.k; i > pos; --i) node.pares[i] = node.pares[i-1];
    node.pares[pos].llave = llave;
    node.pares[pos].valor = valor;
    node.k++;
}

int find_child_index(const Nodo &node, int llave) {
    int i = 0;
    while (i < node.k && llave > node.pares[i].llave) ++i;
    return i;
}


SplitResult split_node(const Nodo &full, bool is_Bplus) {
    SplitResult res;
    res.left = Nodo();
    res.right = Nodo();
    res.left.es_interno = full.es_interno;
    res.right.es_interno = full.es_interno;

    int med_idx = B/2 - 1;
    int idxL = 0, idxR = 0;

    if (!full.es_interno && is_Bplus) {
        for (int i = 0; i < full.k; ++i) {
            if (i <= med_idx) res.left.pares[idxL++] = full.pares[i];
            else res.right.pares[idxR++] = full.pares[i];
        }
        res.med_llave = full.pares[med_idx].llave;
        res.med_valor = full.pares[med_idx].valor;
    } else {
        for (int i = 0; i < full.k; ++i) {
            if (i < med_idx) res.left.pares[idxL++] = full.pares[i];
            else if (i == med_idx) {
                res.med_llave = full.pares[i].llave;
                res.med_valor = full.pares[i].valor;
            } else res.right.pares[idxR++] = full.pares[i];
        }
    }

    res.left.k = idxL;
    res.right.k = idxR;

    if (full.es_interno) {
        for (int i = 0; i <= med_idx; ++i) res.left.hijos[i] = full.hijos[i];
        for (int i = med_idx+1, j = 0; i <= full.k; ++i, ++j)
            res.right.hijos[j] = full.hijos[i];
    }

    return res;
}

void insert_recursive(ListaNodo &arr, int node_idx, int llave, float valor, bool is_Bplus);

void insert(ListaNodo &arr, int &root_idx, int llave, float valor, bool is_Bplus) {
    Nodo root = arr.read(root_idx);
    if (root.k < B) {
        insert_recursive(arr, root_idx, llave, valor, is_Bplus);
    } else {
        SplitResult sp = split_node(root, is_Bplus);
        int right_idx = arr.append(sp.right);
        arr.write(root_idx, sp.left);

        Nodo newroot;
        newroot.es_interno = 1;
        newroot.k = 1;
        newroot.pares[0].llave = sp.med_llave;
        newroot.pares[0].valor = sp.med_valor;
        newroot.hijos[0] = root_idx;
        newroot.hijos[1] = right_idx;

        root_idx = arr.append(newroot);

        if (llave <= sp.med_llave) insert_recursive(arr, newroot.hijos[0], llave, valor, is_Bplus);
        else insert_recursive(arr, newroot.hijos[1], llave, valor, is_Bplus);
    }
}

void insert_recursive(ListaNodo &arr, int node_idx, int llave, float valor, bool is_Bplus) {
    Nodo cur = arr.read(node_idx);
    if (!cur.es_interno) {
        if (cur.k < B) {
            insert_pair_in_node(cur, llave, valor);
            arr.write(node_idx, cur);
        } else {
            SplitResult sp = split_node(cur, is_Bplus);
            int right_idx = arr.append(sp.right);
            sp.left.siguiente = right_idx;
            arr.write(node_idx, sp.left);
        }
    } else {
        int child_rel = find_child_index(cur, llave);
        int child_idx = cur.hijos[child_rel];
        if (child_idx == -1) {
            Nodo nuevo;
            int nuevo_idx = arr.append(nuevo);
            cur.hijos[child_rel] = nuevo_idx;
            arr.write(node_idx, cur);
            insert_recursive(arr, nuevo_idx, llave, valor, is_Bplus);
        } else {
            Nodo child = arr.read(child_idx);
            if (child.k == B) {
                SplitResult sp = split_node(child, is_Bplus);
                int right_idx = arr.append(sp.right);
                arr.write(child_idx, sp.left);
                insert_pair_in_node(cur, sp.med_llave, sp.med_valor);
                for (int i = cur.k; i > child_rel+1; --i)
                    cur.hijos[i] = cur.hijos[i-1];
                cur.hijos[child_rel+1] = right_idx;
                arr.write(node_idx, cur);
                if (llave <= sp.med_llave) insert_recursive(arr, child_idx, llave, valor, is_Bplus);
                else insert_recursive(arr, right_idx, llave, valor, is_Bplus);
            } else insert_recursive(arr, child_idx, llave, valor, is_Bplus);
        }
    }
}