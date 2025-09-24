#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include "nodo.h"
#include "listanodo.h"

struct DiskManager {
    std::string filename;
    mutable uint64_t reads = 0;
    mutable uint64_t writes = 0;

    DiskManager(std::string fname);
    void write_all(const ListaNodo &arr);
    Nodo read_node_at(int idx);
    std::vector<Nodo> read_all();
};

#endif
