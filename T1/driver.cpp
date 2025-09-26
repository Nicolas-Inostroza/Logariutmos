#include "driver.h"
#include "btree.h"
#include <fstream>
using namespace std;

/*
leer_datos :: String, Int -> vector<pair<Int,Float>>
Lee N pares llave-valor desde el archivo fname y los devuelve en un vector.
Cada par llave-valor en el archivo está almacenado en binario, con la llave como un int (4 bytes) y el valor como un float (4 bytes).
*/
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

/*
construir_arbol :: ListaNodo, vector<pair<Int,Float>>, Bool -> Int
Construye un arbol B o B+ (segun is_Bplus) insertando los pares llave-valor.
Devuelve el índice de la raíz del árbol.
*/
int construir_arbol(ListaNodo &arr, const vector<pair<int,float>> &datos, bool is_Bplus) {
    Nodo root;
    int root_idx = arr.append(root);
    for (auto &p : datos) insert(arr, root_idx, p.first, p.second, is_Bplus);
    return root_idx;
}
