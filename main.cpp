// main.cpp (con 50 consultas de una semana)
#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

constexpr int B = 340;                // número de pares por nodo
constexpr int NODE_BYTES = 4096;      // tamaño por nodo (se diseñó b para esto)

// Constantes del enunciado
const int MIN_KEY = 1546300800;
const int MAX_KEY = 1754006400;
const int RANGE_SIZE = 604800; // segundos en 7 días
const int Q = 50;              // número de consultas

struct KeyValue {
    int key;
    float val;
};

struct Nodo {
    int es_interno;
    int k;
    KeyValue pares[B];
    int hijos[B+1];
    int siguiente;

    Nodo() {
        es_interno = 0;
        k = 0;
        siguiente = -1;
        for (int i = 0; i < B+1; ++i) hijos[i] = -1;
    }
};

static_assert(sizeof(Nodo) == NODE_BYTES, "sizeof(Nodo) must be 4096 bytes");

struct NodeArray {
    vector<Nodo> nodes;
    uint64_t reads = 0;
    uint64_t writes = 0;

    int size() const { return (int)nodes.size(); }

    Nodo read(int idx) {
        if (idx < 0 || idx >= (int)nodes.size()) {
            cerr << "ERROR: intento de leer nodo inválido idx=" << idx
                 << " size=" << nodes.size() << "\n";
            throw runtime_error("Índice inválido en NodeArray::read");
        }
        reads++;
        return nodes.at(idx);
    }

    void write(int idx, const Nodo &n) {
        if (idx >= size()) {
            nodes.resize(idx + 1);
        }
        nodes[idx] = n;
        writes++;
    }

    int append(const Nodo &n) {
        nodes.push_back(n);
        writes++;
        return (int)nodes.size() - 1;
    }
};

struct DiskManager {
    string filename;
    mutable uint64_t reads = 0;   // <-- add counters
    mutable uint64_t writes = 0;

    DiskManager(string fname): filename(move(fname)) {}

    void write_all(const NodeArray &arr) {
        ofstream ofs(filename, ios::binary | ios::out | ios::trunc);
        if (!ofs) throw runtime_error("No se pudo abrir archivo para escribir: " + filename);
        for (int i = 0; i < arr.size(); ++i) {
            const Nodo &n = arr.nodes[i];
            ofs.write(reinterpret_cast<const char*>(&n), sizeof(Nodo));
            writes++;   // <-- increment writes
        }
        ofs.close();
    }

    Nodo read_node_at(int idx) {
        ifstream ifs(filename, ios::binary);
        if (!ifs) throw runtime_error("No se pudo abrir archivo para lectura: " + filename);
        ifs.seekg((std::streamoff)idx * sizeof(Nodo), ios::beg);
        Nodo n;
        ifs.read(reinterpret_cast<char*>(&n), sizeof(Nodo));
        ifs.close();
        reads++;   // <-- increment reads
        return n;
    }

    vector<Nodo> read_all() {
        ifstream ifs(filename, ios::binary);
        vector<Nodo> res;
        if (!ifs) throw runtime_error("No se pudo abrir archivo para lectura: " + filename);
        while (true) {
            Nodo n;
            ifs.read(reinterpret_cast<char*>(&n), sizeof(Nodo));
            if (!ifs) break;
            res.push_back(n);
            reads++;   // <-- count read per node
        }
        ifs.close();
        return res;
    }
};


// ---------- Funciones auxiliares ----------

void insert_pair_in_node(Nodo &node, int key, float val) {
    int pos = 0;
    while (pos < node.k && node.pares[pos].key < key) pos++;
    for (int i = node.k; i > pos; --i) node.pares[i] = node.pares[i-1];
    node.pares[pos].key = key;
    node.pares[pos].val = val;
    node.k++;
}

int find_child_index(const Nodo &node, int key) {
    int i = 0;
    while (i < node.k && key > node.pares[i].key) ++i;
    return i;
}

struct SplitResult {
    Nodo left, right;
    int med_key;
    float med_val;
};

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
        res.med_key = full.pares[med_idx].key;
        res.med_val = full.pares[med_idx].val;
    } else {
        for (int i = 0; i < full.k; ++i) {
            if (i < med_idx) res.left.pares[idxL++] = full.pares[i];
            else if (i == med_idx) {
                res.med_key = full.pares[i].key;
                res.med_val = full.pares[i].val;
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

void insert_recursive(NodeArray &arr, int node_idx, int key, float val, bool is_Bplus);

void insert(NodeArray &arr, int &root_idx, int key, float val, bool is_Bplus) {
    Nodo root = arr.read(root_idx);
    if (root.k < B) {
        insert_recursive(arr, root_idx, key, val, is_Bplus);
    } else {
        SplitResult sp = split_node(root, is_Bplus);
        int right_idx = arr.append(sp.right);
        arr.write(root_idx, sp.left);

        Nodo newroot;
        newroot.es_interno = 1;
        newroot.k = 1;
        newroot.pares[0].key = sp.med_key;
        newroot.pares[0].val = sp.med_val;
        newroot.hijos[0] = root_idx;
        newroot.hijos[1] = right_idx;

        root_idx = arr.append(newroot);

        if (key <= sp.med_key) insert_recursive(arr, newroot.hijos[0], key, val, is_Bplus);
        else insert_recursive(arr, newroot.hijos[1], key, val, is_Bplus);
    }
}

void insert_recursive(NodeArray &arr, int node_idx, int key, float val, bool is_Bplus) {
    Nodo cur = arr.read(node_idx);
    if (!cur.es_interno) {
        if (cur.k < B) {
            insert_pair_in_node(cur, key, val);
            arr.write(node_idx, cur);
        } else {
            SplitResult sp = split_node(cur, is_Bplus);
            int right_idx = arr.append(sp.right);
            sp.left.siguiente = right_idx;
            arr.write(node_idx, sp.left);
        }
    } else {
        int child_rel = find_child_index(cur, key);
        int child_idx = cur.hijos[child_rel];
        if (child_idx == -1) {
            Nodo nuevo;
            int nuevo_idx = arr.append(nuevo);
            cur.hijos[child_rel] = nuevo_idx;
            arr.write(node_idx, cur);
            insert_recursive(arr, nuevo_idx, key, val, is_Bplus);
        } else {
            Nodo child = arr.read(child_idx);
            if (child.k == B) {
                SplitResult sp = split_node(child, is_Bplus);
                int right_idx = arr.append(sp.right);
                arr.write(child_idx, sp.left);
                insert_pair_in_node(cur, sp.med_key, sp.med_val);
                for (int i = cur.k; i > child_rel+1; --i)
                    cur.hijos[i] = cur.hijos[i-1];
                cur.hijos[child_rel+1] = right_idx;
                arr.write(node_idx, cur);
                if (key <= sp.med_key) insert_recursive(arr, child_idx, key, val, is_Bplus);
                else insert_recursive(arr, right_idx, key, val, is_Bplus);
            } else insert_recursive(arr, child_idx, key, val, is_Bplus);
        }
    }
}

// ---------- Búsquedas ----------

void range_search_B_disk(DiskManager &dm, int node_idx, int l, int u, vector<pair<int,float>> &out) {
    if (node_idx == -1) return;
    Nodo node = dm.read_node_at(node_idx);
    if (!node.es_interno) {
        for (int i = 0; i < node.k; ++i)
            if (node.pares[i].key >= l && node.pares[i].key <= u)
                out.emplace_back(node.pares[i].key, node.pares[i].val);
    } else {
        for (int i = 0; i <= node.k; ++i)
            if (node.hijos[i] != -1)
                range_search_B_disk(dm, node.hijos[i], l, u, out);
    }
}

vector<pair<int,float>> range_search_Bplus_disk(DiskManager &dm, int root_idx, int l, int u) {
    if (root_idx == -1) return {};
    int cur_idx = root_idx;
    while (true) {
        Nodo node = dm.read_node_at(cur_idx);
        if (!node.es_interno) {
            vector<pair<int,float>> out;
            int iter_idx = cur_idx;
            while (iter_idx != -1) {
                Nodo leaf = dm.read_node_at(iter_idx);
                for (int i = 0; i < leaf.k; ++i) {
                    if (leaf.pares[i].key >= l && leaf.pares[i].key <= u)
                        out.emplace_back(leaf.pares[i].key, leaf.pares[i].val);
                    else if (leaf.pares[i].key > u)
                        return out;
                }
                iter_idx = leaf.siguiente;
            }
            return out;
        } else {
            int child = find_child_index(node, l);
            cur_idx = node.hijos[child];
        }
    }
}

// ---------- Driver ----------

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

int construir_arbol(NodeArray &arr, const vector<pair<int,float>> &datos, bool is_Bplus) {
    Nodo root;
    int root_idx = arr.append(root);
    for (auto &p : datos) insert(arr, root_idx, p.first, p.second, is_Bplus);
    return root_idx;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string datos_file = "datos.bin";
    ofstream out("resultados.csv");
    out << "tipo,N,IOs_insert,nodos,tam_bytes,tiempo_busqueda_ms,IOs_busqueda,tiempo_insert_ms\n";

    for (int exp = 15; exp <= 26; exp++) {
        size_t N = 1ULL << exp;
        cout << "Ejecutando experimento con N=" << N << "\n";

        // ==================== B-Tree ====================
        vector<pair<int,float>> datosB = leer_datos(datos_file, N);
        NodeArray arrB;
        auto t1 = chrono::high_resolution_clock::now();
        int root_idx_B = construir_arbol(arrB, datosB, false);
        auto t2 = chrono::high_resolution_clock::now();
        double tiempo_insert_ms = chrono::duration<double, milli>(t2 - t1).count();

        size_t ios_insert = arrB.reads + arrB.writes;
        size_t nodos = arrB.size();
        size_t tam_bytes = nodos * sizeof(Nodo);

        DiskManager dmB("treeB_" + to_string(exp) + ".bin");
        dmB.write_all(arrB);

        double sum_time = 0.0;
        size_t sum_ios = 0;
        mt19937 rng(42);
        uniform_int_distribution<int> distL(MIN_KEY, MAX_KEY - RANGE_SIZE);

        for (int q = 0; q < Q; q++) {
            int l = distL(rng);
            int u = l + RANGE_SIZE;
            vector<pair<int,float>> res;
            auto tq1 = chrono::high_resolution_clock::now();
            range_search_B_disk(dmB, root_idx_B, l, u, res);
            auto tq2 = chrono::high_resolution_clock::now();
            sum_time += chrono::duration<double, milli>(tq2 - tq1).count();
            double pct = 100.0 * res.size() / N;
            cout << "[B] Consulta " << q << " -> " << res.size()
                 << " resultados (" << pct << "%)\n";
        }

        double avg_time = sum_time / Q;
        out << "B," << N << "," << ios_insert << "," << nodos << "," << tam_bytes
            << "," << avg_time << "," << sum_ios << "," << tiempo_insert_ms << "\n";

        // ==================== B+ Tree ====================
        vector<pair<int,float>> datosBp = leer_datos(datos_file, N);
        NodeArray arrBp;
        t1 = chrono::high_resolution_clock::now();
        int root_idx_Bp = construir_arbol(arrBp, datosBp, true);
        t2 = chrono::high_resolution_clock::now();
        tiempo_insert_ms = chrono::duration<double, milli>(t2 - t1).count();

        ios_insert = arrBp.reads + arrBp.writes;
        nodos = arrBp.size();
        tam_bytes = nodos * sizeof(Nodo);

        DiskManager dmBp("treeBplus_" + to_string(exp) + ".bin");
        dmBp.write_all(arrBp);

        sum_time = 0.0;
        sum_ios = 0;

        for (int q = 0; q < Q; q++) {
            int l = distL(rng);
            int u = l + RANGE_SIZE;
            auto tq1 = chrono::high_resolution_clock::now();
            auto res = range_search_Bplus_disk(dmBp, root_idx_Bp, l, u);
            auto tq2 = chrono::high_resolution_clock::now();
            sum_time += chrono::duration<double, milli>(tq2 - tq1).count();
            double pct = 100.0 * res.size() / N;
            cout << "[B+] Consulta " << q << " -> " << res.size()
                 << " resultados (" << pct << "%)\n";
        }

        avg_time = sum_time / Q;
        out << "B+," << N << "," << ios_insert << "," << nodos << "," << tam_bytes
            << "," << avg_time << "," << sum_ios << "," << tiempo_insert_ms << "\n";
    }
    return 0;
}
