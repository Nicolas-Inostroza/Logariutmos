#include "listanodo.h"
#include <stdexcept>
#include <iostream>
using namespace std;

int ListaNodo::size() const { return (int)nodes.size(); }

/*
read :: Int -> Nodo
Lee el nodo en la posición idx de la lista de nodos.
aumenta el contador de lecturas.
*/
Nodo ListaNodo::read(int idx) {
    if (idx < 0 || idx >= (int)nodes.size()) {
        cerr << "ERROR: intento de leer nodo inválido idx=" << idx
             << " size=" << nodes.size() << "\n";
        throw runtime_error("Índice inválido en ListaNodo::read");
    }
    reads++;
    return nodes.at(idx);
}

/*
write :: Int, Nodo -> Void
Escribe el nodo n en la posición idx de la lista de nodos.
aumenta el contador de escrituras.
*/
void ListaNodo::write(int idx, const Nodo &n) {
    if (idx >= size()) {
        nodes.resize(idx + 1);
    }
    nodes[idx] = n;
    writes++;
}

/*
append :: Nodo -> Int
Agrega el nodo n al final de la lista de nodos.
Devuelve el índice donde se agregó el nodo.
Aumenta el contador de escrituras.
*/
int ListaNodo::append(const Nodo &n) {
    nodes.push_back(n);
    writes++;
    return (int)nodes.size() - 1;
}
