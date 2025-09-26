#ifndef NODEARRAY_H
#define NODEARRAY_H

#include "nodo.h"

/*
ListaNodo :: struct
Estructura que representa una lista de nodos en memoria.
Contiene un vector de nodos y contadores de lecturas y escrituras.
*/
struct ListaNodo {
    std::vector<Nodo> nodes;
    uint64_t reads = 0;
    uint64_t writes = 0;

    int size() const;
    Nodo read(int idx);
    void write(int idx, const Nodo &n);
    int append(const Nodo &n);
};

#endif
