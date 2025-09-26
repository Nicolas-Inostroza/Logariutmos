#ifndef NODO_H
#define NODO_H

#include <bits/stdc++.h>
using namespace std;

constexpr int B = 340;
constexpr int Bytes_nodo = 4096;

/*
LlaveValor :: struct
Estructura que representa un par llave-valor.
*/
struct LlaveValor {
    int llave;
    float valor;
};

/*
Nodo :: struct
Estructura que representa un nodo en un árbol B o B+.
Contiene un arreglo de pares llave-valor, un arreglo de hijos, un indicador de si es interno o hoja, y un índice al siguiente nodo hoja (solo para B+).
*/
struct Nodo {
    int es_interno;
    int k;
    LlaveValor pares[B];
    int hijos[B+1];
    int siguiente;

    Nodo();
};

/*
Verifica que el tamaño de la estructura Nodo sea igual a Bytes_nodo (4096 bytes).
*/
static_assert(sizeof(Nodo) == Bytes_nodo, "sizeof(Nodo) must be 4096 bytes");

#endif
