#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include "nodo.h"
#include "listanodo.h"


/*
DiskManager :: struct
Estructura que maneja la lectura y escritura de nodos en disco.
Contiene el nombre del archivo y contadores de lecturas y escrituras.
*/
struct DiskManager {
    std::string filename;
    mutable uint64_t reads = 0;
    mutable uint64_t writes = 0;

    DiskManager(std::string fname);
    void write_all(const ListaNodo &arr);
    Nodo read_node_at(int idx);
};

#endif
