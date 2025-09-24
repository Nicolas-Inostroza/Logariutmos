#ifndef DRIVER_H
#define DRIVER_H

#include "listanodo.h"

std::vector<std::pair<int,float>> leer_datos(const std::string &fname, size_t N);
int construir_arbol(ListaNodo &arr, const std::vector<std::pair<int,float>> &datos, bool is_Bplus);

#endif
