#include "btree.h"
#include <stdexcept>
#include <iostream>


/* 
insert_pair_node :: Nodo, Int, Float -> Void
Esta funcion se encarga de insertar el valor y llave de un nodo.
Para esto se busca la posicionicion donde debe ir el nodo segun el valor de la llave y la cantidad de pares llave-valor dentro del resto de nodos.
Luego movemos pares llave-valor de la lista pares del nodo para hacer espacio para los valores llave-valor a insertar en la posicion calculada.
*/
void insert_pair_in_node(Nodo &nodo, int llave, float valor) {
    int posicion = 0;
    //Loop para buscar la posicion donde deberia ir el par llave-valor a insertar se verifica con k para no pasar del tamaño del arreglo
    while (posicion < nodo.k && nodo.pares[posicion].llave < llave) posicion++;

    //recorremos la lista de pares desde el final al inicio moviendo cada par uno a la derecha hasta la posicion calculada
    for (int i = nodo.k; i > posicion; --i) nodo.pares[i] = nodo.pares[i-1];
    nodo.pares[posicion].llave = llave;
    nodo.pares[posicion].valor = valor;
    nodo.k++;
}


/*
find_child_index :: Nodo, Int -> Int
Recorre la lista de pares hasta encontrar un par donde la llave sea menor a la buscada y devuelve el indice de este
*/
int find_child_index(const Nodo &nodo, int llave) {
    int i = 0;
    while (i < nodo.k && llave > nodo.pares[i].llave) ++i;
    return i;
}

// split_node :: const Nodo&, bool -> SplitResult
// Divide un nodo que este con la cantidad maxima de nodos y los separa en dos retornando una estructura SplitResult con los nodos izquierdo, derecho y el par llave-valor del medio.
// Si es B+ y el nodo es hoja, el par llave-valor del medio se queda en el nodo izquierdo.
SplitResult split_node(const Nodo &nodo_full, bool es_Bplus) {
    SplitResult resultado;
    resultado.left = Nodo();
    resultado.right = Nodo();
    resultado.left.es_interno = nodo_full.es_interno;
    resultado.right.es_interno = nodo_full.es_interno;

    int indice_medio = B/2 - 1; //restamos uno ya que la lista empieza de 0
    int indice_izq = 0, indice_der = 0;


    //Separamos por casos si es B+ y si el nodo es hoja o interno
    if (!nodo_full.es_interno && es_Bplus) {
        for (int i = 0; i < nodo_full.k; ++i) {
            if (i <= indice_medio){
                 resultado.left.pares[indice_izq++] = nodo_full.pares[i];
            }
            else { 
                resultado.right.pares[indice_der++] = nodo_full.pares[i];
            }
        
        }
        //Seteamos los valores del par llave-valor al resultado.
        resultado.med_llave = nodo_full.pares[indice_medio].llave;
        resultado.med_valor = nodo_full.pares[indice_medio].valor;
    } else {
        for (int i = 0; i < nodo_full.k; ++i) {
            if (i < indice_medio){
                resultado.left.pares[indice_izq++] = nodo_full.pares[i];
            } 
            else if (i == indice_medio) {
                resultado.med_llave = nodo_full.pares[i].llave;
                resultado.med_valor = nodo_full.pares[i].valor;
            } else{
                resultado.right.pares[indice_der++] = nodo_full.pares[i];
            } 
        }
    }

    resultado.left.k = indice_izq;
    resultado.right.k = indice_der;
    // Si el nodo es interno, tambien debemos separar los hijos
    if (nodo_full.es_interno) {
        for (int i = 0; i <= indice_medio; ++i) resultado.left.hijos[i] = nodo_full.hijos[i];
        for (int i = indice_medio+1, j = 0; i <= nodo_full.k; ++i, ++j)
            resultado.right.hijos[j] = nodo_full.hijos[i];
    }

    return resultado;
}

void insert_recursive(ListaNodo &lista_nodos, int indice_nodo, int llave, float valor, bool es_Bplus);



/*
insert :: ListaNodo, Int&, Int, Float, Bool -> Void
Funcion principal para insertar un par llave-valor en el arbol B o B+.
Si la raiz esta llena, se divide y se crea una nueva raiz.
Si no esta llena, se llama a la funcion recursiva insert_recursive para insertar el par en el nodo correspondiente.
*/
void insert(ListaNodo &lista_nodos, int &indice_raiz, int llave, float valor, bool es_Bplus) {
    Nodo raiz = lista_nodos.read(indice_raiz);
    if (raiz.k < B) {
        insert_recursive(lista_nodos, indice_raiz, llave, valor, es_Bplus);
    } else {
        //Si la raiz esta llena es decir k=B, se divide con split_node y se crea una nueva raiz
        SplitResult separados = split_node(raiz, es_Bplus);
        int indice_der = lista_nodos.append(separados.right);
        lista_nodos.write(indice_raiz, separados.left);
        
        //Iniciamos la nueva raiz con los valores correspondientes
        Nodo nueva_raiz;
        nueva_raiz.es_interno = 1;
        nueva_raiz.k = 1;
        nueva_raiz.pares[0].llave = separados.med_llave;
        nueva_raiz.pares[0].valor = separados.med_valor;
        nueva_raiz.hijos[0] = indice_raiz;
        nueva_raiz.hijos[1] = indice_der;


        indice_raiz = lista_nodos.append(nueva_raiz);

        //Ahora insertamos el par llave-valor en el nodo izquierdo o derecho segun corresponde
        if (llave <= separados.med_llave){
            insert_recursive(lista_nodos, nueva_raiz.hijos[0], llave, valor, es_Bplus);
        }
        else {
            insert_recursive(lista_nodos, nueva_raiz.hijos[1], llave, valor, es_Bplus);
        }
    }
}


/*
insert_recursive :: ListaNodo, Int, Int, Float, Bool -> Void
Funcion recursiva que se encarga de insertar un par llave-valor en el nodo correspondiente
Si el nodo es hoja y tiene espacio, se inserta el par directamente.
Si el nodo es hoja y no tiene espacio, se divide el nodo y se inserta el par en el nodo correspondiente.
Si el nodo es interno, se busca el hijo correspondiente y se llama recursivamente a insert_recursive.
*/
void insert_recursive(ListaNodo &lista_nodos, int indice_nodo, int llave, float valor, bool es_Bplus) {

    Nodo nodo_actual = lista_nodos.read(indice_nodo);
    // Vemos si estamos en una hoja
    if (!nodo_actual.es_interno) {
        // Si el nodo tiene espacio insertamos el par directamente, sino se separa el nodo en dos y se actualiza "el arbol" con los cambios.
        if (nodo_actual.k < B) {
            insert_pair_in_node(nodo_actual, llave, valor);
            lista_nodos.write(indice_nodo, nodo_actual);
        } else {
            SplitResult separados = split_node(nodo_actual, es_Bplus);
            int indice_der = lista_nodos.append(separados.right);
            separados.left.siguiente = indice_der;
            lista_nodos.write(indice_nodo, separados.left);
        }
    } else {
        int child_rel = find_child_index(nodo_actual, llave);
        int child_idx = nodo_actual.hijos[child_rel];
        // Si el hijo no existe, se crea uno nuevo y se inserta el par ahi.
        if (child_idx == -1) {
            Nodo nuevo;
            int nuevo_idx = lista_nodos.append(nuevo);
            nodo_actual.hijos[child_rel] = nuevo_idx;
            lista_nodos.write(indice_nodo, nodo_actual);
            insert_recursive(lista_nodos, nuevo_idx, llave, valor, es_Bplus);
        } else {
            Nodo child = lista_nodos.read(child_idx);
            if (child.k == B) {
                SplitResult separados = split_node(child, es_Bplus);
                int indice_der = lista_nodos.append(separados.right);
                lista_nodos.write(child_idx, separados.left);
                insert_pair_in_node(nodo_actual, separados.med_llave, separados.med_valor);
                for (int i = nodo_actual.k; i > child_rel+1; --i)
                    nodo_actual.hijos[i] = nodo_actual.hijos[i-1];
                nodo_actual.hijos[child_rel+1] = indice_der;
                lista_nodos.write(indice_nodo, nodo_actual);
                if (llave <= separados.med_llave) insert_recursive(lista_nodos, child_idx, llave, valor, es_Bplus);
                else insert_recursive(lista_nodos, indice_der, llave, valor, es_Bplus);
            } else insert_recursive(lista_nodos, child_idx, llave, valor, es_Bplus);
        }
    }
}