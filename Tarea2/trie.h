#ifndef TRIE_H
#define TRIE_H

#include <string> // Required for std::string

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

// Forward declaration for helper function
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
     * Destructor del Trie
     * Libera toda la memoria utilizada por la estructura
     */
    ~Trie();
    
    /**
     * Convierte un carácter a su índice en next[]
     * @param c: carácter a convertir
     * @return: índice en el rango [0, 26], -1 si no es válido
     */
    int char_to_index(char c);
    
    /**
     * Verifica si un nodo es un nodo terminal
     * @param node: nodo a verificar
     * @return: true si es nodo terminal, false en caso contrario
     */
    bool is_terminal_node(TrieNode* node);
    
    /**
     * Inserta una palabra en el trie
     * @param w: palabra a insertar
     */
    void insert(const string& w);
    
    /**
     * Desciende por un carácter desde un nodo
     * @param v: nodo actual
     * @param c: carácter por el que descender
     * @return: puntero al nodo hijo o nullptr si no existe
     */
    TrieNode* descend(TrieNode* v, char c);
    
    /**
     * Retorna el mejor autocompletado del subárbol
     * @param v: nodo raíz del subárbol
     * @return: puntero al nodo terminal con mejor prioridad o nullptr
     */
    TrieNode* autocomplete(TrieNode* v);
    
    /**
     * Actualiza prioridad según variante de frecuencia
     * @param terminal: nodo terminal a actualizar
     */
    void update_priority_frequency(TrieNode* terminal);
    
    /**
     * Actualiza prioridad según variante de reciente
     * @param terminal: nodo terminal a actualizar
     */
    void update_priority_recent(TrieNode* terminal);
    
    /**
     * Propaga la actualización de best_priority y best_terminal hacia la raíz
     * @param v: nodo terminal que fue actualizado
     */
    void propagate_best(TrieNode* v);
    
    /**
     * Obtiene el número total de nodos en el trie
     * @return: cantidad de nodos
     */
    long long get_node_count() const;
};

/**
 * Función auxiliar para obtener un nodo terminal
 * @param trie: referencia al trie
 * @param word: palabra a buscar
 * @return: puntero al nodo terminal o nullptr si no existe
 */
TrieNode* get_terminal(Trie& trie, const string& word);

#endif // TRIE_H