#include "nodo.h"

/*
Nodo :: Constructor
Inicializa un nodo con valores por defecto.
*/
Nodo::Nodo() {
    es_interno = 0;
    k = 0;
    siguiente = -1;
    for (int i = 0; i < B+1; ++i) hijos[i] = -1;
}
