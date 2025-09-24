#include "manejodisco.h"
#include <fstream>
#include <stdexcept>
using namespace std;

DiskManager::DiskManager(string fname): filename(move(fname)) {}

void DiskManager::write_all(const ListaNodo &arr) {
    ofstream ofs(filename, ios::binary | ios::out | ios::trunc);
    if (!ofs) throw runtime_error("No se pudo abrir archivo para escribir: " + filename);
    for (int i = 0; i < arr.size(); ++i) {
        const Nodo &n = arr.nodes[i];
        ofs.write(reinterpret_cast<const char*>(&n), sizeof(Nodo));
        writes++;
    }
    ofs.close();
}

Nodo DiskManager::read_node_at(int idx) {
    ifstream ifs(filename, ios::binary);
    if (!ifs) throw runtime_error("No se pudo abrir archivo para lectura: " + filename);
    ifs.seekg((std::streamoff)idx * sizeof(Nodo), ios::beg);
    Nodo n;
    ifs.read(reinterpret_cast<char*>(&n), sizeof(Nodo));
    ifs.close();
    reads++;
    return n;
}

vector<Nodo> DiskManager::read_all() {
    ifstream ifs(filename, ios::binary);
    vector<Nodo> res;
    if (!ifs) throw runtime_error("No se pudo abrir archivo para lectura: " + filename);
    while (true) {
        Nodo n;
        ifs.read(reinterpret_cast<char*>(&n), sizeof(Nodo));
        if (!ifs) break;
        res.push_back(n);
        reads++;
    }
    ifs.close();
    return res;
}
