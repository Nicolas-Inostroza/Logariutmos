#ifndef NODO_H
#define NODO_H

#include <bits/stdc++.h>
using namespace std;

constexpr int B = 340;
constexpr int Bytes_nodo = 4096;

struct LlaveValor {
    int llave;
    float valor;
};

struct Nodo {
    int es_interno;
    int k;
    LlaveValor pares[B];
    int hijos[B+1];
    int siguiente;

    Nodo();
};

static_assert(sizeof(Nodo) == Bytes_nodo, "sizeof(Nodo) must be 4096 bytes");

#endif
