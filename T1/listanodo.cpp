#include "listanodo.h"
#include <stdexcept>
#include <iostream>
using namespace std;

int ListaNodo::size() const { return (int)nodes.size(); }

Nodo ListaNodo::read(int idx) {
    if (idx < 0 || idx >= (int)nodes.size()) {
        cerr << "ERROR: intento de leer nodo inválido idx=" << idx
             << " size=" << nodes.size() << "\n";
        throw runtime_error("Índice inválido en ListaNodo::read");
    }
    reads++;
    return nodes.at(idx);
}

void ListaNodo::write(int idx, const Nodo &n) {
    if (idx >= size()) {
        nodes.resize(idx + 1);
    }
    nodes[idx] = n;
    writes++;
}

int ListaNodo::append(const Nodo &n) {
    nodes.push_back(n);
    writes++;
    return (int)nodes.size() - 1;
}
