#include "driver.h"
#include "btree.h"
#include <fstream>
using namespace std;

vector<pair<int,float>> leer_datos(const string &fname, size_t N) {
    ifstream ifs(fname, ios::binary);
    vector<pair<int,float>> datos;
    datos.reserve(N);
    for (size_t i = 0; i < N; i++) {
        int t; float v;
        ifs.read(reinterpret_cast<char*>(&t), sizeof(int));
        ifs.read(reinterpret_cast<char*>(&v), sizeof(float));
        if (!ifs) break;
        datos.emplace_back(t, v);
    }
    return datos;
}

int construir_arbol(ListaNodo &arr, const vector<pair<int,float>> &datos, bool is_Bplus) {
    Nodo root;
    int root_idx = arr.append(root);
    for (auto &p : datos) insert(arr, root_idx, p.first, p.second, is_Bplus);
    return root_idx;
}
