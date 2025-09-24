#include <bits/stdc++.h>
#include "driver.h"
#include "manejodisco.h"
#include "busqueda.h"
using namespace std;

const int MIN_KEY = 1546300800;
const int MAX_KEY = 1754006400;
const int RANGE_SIZE = 604800;
const int Q = 50;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string datos_file = "datos.bin";
    ofstream out("resultados.csv");
    out << "tipo,N,IOs_insert,nodos,tam_bytes,tiempo_busqueda_ms,IOs_busqueda,tiempo_insert_ms\n";

    for (int exp = 15; exp <= 26; exp++) {
        size_t N = 1ULL << exp;
        cout << "Ejecutando experimento con N=" << N << "\n";

        // =============== B-Tree ===============
        vector<pair<int,float>> datosB = leer_datos(datos_file, N);
        ListaNodo arrB;
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

        // =============== B+ Tree ===============
        vector<pair<int,float>> datosBp = leer_datos(datos_file, N);
        ListaNodo arrBp;
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
