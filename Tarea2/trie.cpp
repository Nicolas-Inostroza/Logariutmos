#include <iostream>
#include <cstring>
#include <cctype>
#include "trie.h"

using namespace std;

const int ALFABETO = 27; // 26 letras + '$'

/**
 * Estructura de un nodo en el Trie
 * Representa un prefijo o una palabra completa en el árbol de búsqueda.
 */
struct TrieNode {
    TrieNode* parent;           // Referencia al nodo padre (nullptr si es raíz)
    TrieNode* next[ALFABETO];   // Mapeo de caracteres a hijos
    long long priority;         // Prioridad según criterio (frecuencia o reciente)
    string* str;                // Puntero al string si es nodo terminal
    TrieNode* best_terminal;    // Puntero al nodo terminal con mayor prioridad del subárbol
    long long best_priority;    // Prioridad del mejor nodo terminal del subárbol
    
    TrieNode() : parent(nullptr), priority(0), str(nullptr), 
                 best_terminal(nullptr), best_priority(-1) {
        for (int i = 0; i < ALFABETO; i++) {
            next[i] = nullptr;
        }
    }
};

// Forward declaration
void destroy_tree(TrieNode* node);

/**
 * Estructura del Trie
 * Implementa un árbol de prefijos para autocompletado con dos variantes:
 * - Frecuencia: retorna la palabra más accedida
 * - Reciente: retorna la palabra accedida más recientemente
 */
struct Trie {
    TrieNode* root;
    long long node_count;       // Contador de nodos en la estructura
    long long access_counter;   // Contador de accesos para variante reciente
    
    /**
     * Constructor del Trie
     * Inicializa la raíz y los contadores
     */
    Trie() : node_count(0), access_counter(0) {
        root = new TrieNode();
        node_count++;
    }
    
    /**
     * Convierte un carácter a su índice en next[]
     * @param c: carácter a convertir
     * @return: índice en el rango [0, 26], -1 si no es válido
     */
    int char_to_index(char c) {
        if (c == '$') return 26;
        if (c >= 'a' && c <= 'z') return c - 'a';
        if (c >= 'A' && c <= 'Z') return c - 'A';
        return -1;
    }
    
    /**
     * Verifica si un nodo es un nodo terminal
     * Un nodo es terminal si es el hijo '$' de su padre
     * @param node: nodo a verificar
     * @return: true si es nodo terminal, false en caso contrario
     */
    bool is_terminal_node(TrieNode* node) {
        if (node == nullptr || node->parent == nullptr) return false;
        int idx = char_to_index('$');
        return node->parent->next[idx] == node;
    }
    
    /**
     * Inserta una palabra en el trie
     * Crea nodos según sea necesario y marca el final con '$'
     * @param w: palabra a insertar
     */
    void insert(const string& w) {
        TrieNode* current = root;
        
        // Inserta cada carácter
        for (char c : w) {
            int idx = char_to_index(tolower(c));
            if (idx < 0) continue;
            
            if (current->next[idx] == nullptr) {
                current->next[idx] = new TrieNode();
                current->next[idx]->parent = current;
                node_count++;
            }
            current = current->next[idx];
        }
        
        // Marca el fin de palabra con '$'
        int idx = char_to_index('$');
        if (current->next[idx] == nullptr) {
            current->next[idx] = new TrieNode();
            current->next[idx]->parent = current;
            node_count++;
        }
        
        // El nodo terminal es el que contiene '$'
        TrieNode* terminal = current->next[idx];
        
        // Si ya existe la palabra, no crear un string duplicado
        if (terminal->str == nullptr) {
            terminal->str = new string(w);
            terminal->priority = 0;
            terminal->best_terminal = terminal;
            terminal->best_priority = 0;  // El nodo terminal es su propio mejor resultado
            
            // Propagar después de insertar por primera vez
            propagate_best(terminal);
        }
    }
    
    /**
     * Desciende por un carácter desde un nodo
     * @param v: nodo actual
     * @param c: carácter por el que descender
     * @return: puntero al nodo hijo o nullptr si no existe
     */
    TrieNode* descend(TrieNode* v, char c) {
        if (v == nullptr) return nullptr;
        int idx = char_to_index(tolower(c));
        if (idx < 0) return nullptr;
        return v->next[idx];
    }
    
    /**
     * Retorna el mejor autocompletado del subárbol
     * Retorna el nodo terminal con mayor prioridad en el subárbol
     * @param v: nodo raíz del subárbol
     * @return: puntero al nodo terminal con mejor prioridad o nullptr
     */
    TrieNode* autocomplete(TrieNode* v) {
        if (v == nullptr) return nullptr;
        
        // Verificar que best_terminal es válido, tiene str y best_priority es válido
        if (v->best_priority >= 0 && v->best_terminal != nullptr && 
            v->best_terminal->str != nullptr && is_terminal_node(v->best_terminal)) {
            return v->best_terminal;
        }
        
        return nullptr;
    }
    
    /**
     * Actualiza prioridad según variante de frecuencia
     * Incrementa la prioridad en 1 y propaga cambios hacia la raíz
     * @param terminal: nodo terminal a actualizar
     */
    void update_priority_frequency(TrieNode* terminal) {
        if (terminal == nullptr || terminal->str == nullptr) return;
        
        terminal->priority++;
        propagate_best(terminal);
    }
    
    /**
     * Actualiza prioridad según variante de reciente
     * Asigna un timestamp basado en access_counter y propaga cambios
     * @param terminal: nodo terminal a actualizar
     */
    void update_priority_recent(TrieNode* terminal) {
        if (terminal == nullptr || terminal->str == nullptr) return;
        
        terminal->priority = ++access_counter;
        propagate_best(terminal);
    }
    
    /**
     * Propaga la actualización de best_priority y best_terminal hacia la raíz
     * Actualiza el mejor nodo terminal de cada nodo en el camino a la raíz
     * @param v: nodo terminal que fue actualizado
     */
    void propagate_best(TrieNode* v) {
        TrieNode* current = v->parent;
        
        while (current != nullptr) {
            long long old_best_priority = current->best_priority;
            TrieNode* old_best_terminal = current->best_terminal;
            
            // Reiniciar best_priority del nodo actual
            current->best_priority = -1;
            current->best_terminal = nullptr;
            
            // Buscar el hijo con mejor prioridad
            for (int i = 0; i < ALFABETO; i++) {
                if (current->next[i] != nullptr) {
                    TrieNode* child = current->next[i];
                    if (child->best_priority > current->best_priority) {
                        current->best_priority = child->best_priority;
                        current->best_terminal = child->best_terminal;
                    }
                }
            }
            
            // Si no hubo cambios, se puede detener la propagación
            if (current->best_priority == old_best_priority && 
                current->best_terminal == old_best_terminal) {
                break;
            }
            
            current = current->parent;
        }
    }
    
    /**
     * Obtiene el número total de nodos en el trie
     * @return: cantidad de nodos
     */
    long long get_node_count() const {
        return node_count;
    }
    
    /**
     * Destructor del Trie
     * Libera toda la memoria utilizada por la estructura
     */
    ~Trie() {
        destroy_tree(root);
    }
};

/**
 * Función auxiliar para destruir recursivamente el árbol
 * @param node: nodo raíz del subárbol a destruir
 */
void destroy_tree(TrieNode* node) {
    if (node == nullptr) return;
    for (int i = 0; i < ALFABETO; i++) {
        destroy_tree(node->next[i]);
    }
    if (node->str != nullptr) {
        delete node->str;
    }
    delete node;
}

/**
 * Función auxiliar para obtener un nodo terminal
 * @param trie: referencia al trie
 * @param word: palabra a buscar
 * @return: puntero al nodo terminal o nullptr si no existe
 */
TrieNode* get_terminal(Trie& trie, const string& word) {
    TrieNode* current = trie.root;
    for (char c : word) {
        current = trie.descend(current, c);
        if (current == nullptr) return nullptr;
    }
    return trie.descend(current, '$');
}

/*
int main() {
    Trie trie;
    
    // Insertar palabras
    trie.insert("apple");
    trie.insert("application");
    trie.insert("apply");
    trie.insert("appreciate");
    
    cout << "Nodos creados: " << trie.get_node_count() << endl;
    
    // Obtener referencias a los nodos terminales
    TrieNode* term_apple = get_terminal(trie, "apple");
    TrieNode* term_application = get_terminal(trie, "application");
    TrieNode* term_apply = get_terminal(trie, "apply");
    TrieNode* term_appreciate = get_terminal(trie, "appreciate");
    
    cout << "\n--- ANTES de actualizar prioridades ---" << endl;
    TrieNode* node_a = trie.descend(trie.root, 'a');
    if (node_a) {
        TrieNode* best = trie.autocomplete(node_a);
        if (best && best->str) {
            cout << "Mejor autocompletado desde 'a': " << *best->str 
                 << " (prioridad: " << best->priority << ")" << endl;
        }
    }
    
    // Simular accesos con diferentes frecuencias
    cout << "\n--- Actualizando prioridades (FRECUENCIA) ---" << endl;
    if (term_apple && trie.is_terminal_node(term_apple)) {
        trie.update_priority_frequency(term_apple);
        trie.update_priority_frequency(term_apple);
        trie.update_priority_frequency(term_apple);
        cout << "apple: +3 accesos (prioridad: " << term_apple->priority << ")" << endl;
    }
    
    if (term_application && trie.is_terminal_node(term_application)) {
        trie.update_priority_frequency(term_application);
        trie.update_priority_frequency(term_application);
        cout << "application: +2 accesos (prioridad: " << term_application->priority << ")" << endl;
    }
    
    if (term_apply && trie.is_terminal_node(term_apply)) {
        trie.update_priority_frequency(term_apply);
        cout << "apply: +1 acceso (prioridad: " << term_apply->priority << ")" << endl;
    }
    
    if (term_appreciate && trie.is_terminal_node(term_appreciate)) {
        cout << "appreciate: sin accesos (prioridad: " << term_appreciate->priority << ")" << endl;
    }
    
    cout << "\n--- DESPUES de actualizar prioridades ---" << endl;
    node_a = trie.descend(trie.root, 'a');
    if (node_a) {
        TrieNode* best = trie.autocomplete(node_a);
        if (best && best->str) {
            cout << "Mejor autocompletado desde 'a': " << *best->str 
                 << " (prioridad: " << best->priority << ")" << endl;
        }
        
        TrieNode* node_ap = trie.descend(node_a, 'p');
        if (node_ap) {
            best = trie.autocomplete(node_ap);
            if (best && best->str) {
                cout << "Mejor autocompletado desde 'ap': " << *best->str 
                     << " (prioridad: " << best->priority << ")" << endl;
            }
        }
    }
    
    // Prueba de inserción duplicada
    cout << "\n--- Prueba de palabra duplicada ---" << endl;
    trie.insert("apple");  // Insertar "apple" de nuevo
    cout << "Nodos después de insertar 'apple' duplicado: " << trie.get_node_count() << endl;
    
    return 0;
}
*/