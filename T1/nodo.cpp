#include "nodo.h"

Nodo::Nodo() {
    es_interno = 0;
    k = 0;
    siguiente = -1;
    for (int i = 0; i < B+1; ++i) hijos[i] = -1;
}
