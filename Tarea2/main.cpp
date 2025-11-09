#include <iostream>
#include <cstring>
#include <cctype>
#include <fstream>      // Para leer archivos
#include <vector>       // Para almacenar datasets
#include <string>       // Para std::string
#include <chrono>       // Para medir el tiempo
#include <iomanip>      // Para formatear la salida (setprecision)
#include <set>          // Para los puntos de medición
#include <cmath>        // Para pow()


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
 * Inserta una palabra en el trie - VERSIÓN CORREGIDA
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
        
        // El nodo terminal es el que contiene '$'
        TrieNode* terminal = current->next[idx];
        terminal->str = new string(w);
        terminal->priority = 0;
        
        // IMPORTANTE: El nodo terminal es su propio best_terminal
        terminal->best_terminal = terminal;
        terminal->best_priority = 0;
        
        // CRÍTICO: Propagar inmediatamente después de crear el nodo terminal
        propagate_best(terminal);
    }
    // Si ya existe, no hacer nada (la palabra ya estaba insertada)
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
 * Actualiza prioridad según variante de frecuencia - VERSIÓN CORREGIDA
 * Incrementa la prioridad en 1 y propaga cambios hacia la raíz
 * @param terminal: nodo terminal a actualizar
 */
void update_priority_frequency(TrieNode* terminal) {
    if (terminal == nullptr || terminal->str == nullptr) return;
    if (!is_terminal_node(terminal)) return;
    
    terminal->priority++;
    terminal->best_priority = terminal->priority;  // IMPORTANTE: actualizar best_priority también
    propagate_best(terminal);
}

/**
 * Actualiza prioridad según variante de reciente - VERSIÓN CORREGIDA
 * Asigna un timestamp basado en access_counter y propaga cambios
 * @param terminal: nodo terminal a actualizar
 */
void update_priority_recent(TrieNode* terminal) {
    if (terminal == nullptr || terminal->str == nullptr) return;
    if (!is_terminal_node(terminal)) return;
    
    terminal->priority = ++access_counter;
    terminal->best_priority = terminal->priority;  // IMPORTANTE: actualizar best_priority también
    propagate_best(terminal);
}
    
    /**
 * Propaga la actualización de best_priority y best_terminal hacia la raíz - VERSIÓN CORREGIDA
 * @param v: nodo desde donde propagar (usualmente un nodo terminal recién actualizado)
 */
void propagate_best(TrieNode* v) {
    if (v == nullptr) return;
    
    // Empezamos desde el padre del nodo que cambió
    TrieNode* current = v->parent;
    
    while (current != nullptr) {
        long long old_best_priority = current->best_priority;
        TrieNode* old_best_terminal = current->best_terminal;
        
        // Recalcular el mejor hijo
        current->best_priority = -1;
        current->best_terminal = nullptr;
        
        for (int i = 0; i < ALFABETO; i++) {
            if (current->next[i] != nullptr) {
                TrieNode* child = current->next[i];
                
                // IMPORTANTE: Solo considerar hijos que tienen un best_terminal válido
                if (child->best_terminal != nullptr && child->best_priority >= 0) {
                    // Usar > estricto (no >=) para consistencia
                    if (current->best_terminal == nullptr || 
                        child->best_priority > current->best_priority) {
                        current->best_priority = child->best_priority;
                        current->best_terminal = child->best_terminal;
                    }
                }
            }
        }
        
        // Optimización: si no hubo cambios, detener propagación
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


/**
 * load_words :: String -> Vector
 * Función para cargar un data crear un Vector desde un archivo de texto
 * @param filename: nombre del archivo
 * @return: un vector de strings con las palabras del archivo
 */
vector<string> load_words(const string& filename) {
    ifstream file(filename);
    vector<string> words;
    string word;
    
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return words;
    }
    
    while (file >> word) {
        words.push_back(word);
    }
    file.close();
    return words;
}

/**
 * create_measurementpoints :: long long -> Set
 * Genera los puntos de medición (potencias de 2) Que se usaran para los experimentos posteriores
 * @param max_val: valor máximo (N o L)
 * @return: un set de long long con los puntos
 */
set<long long> create_measurement_points(long long max_val) {
    set<long long> points;
    long long p = 1; // Empieza en 2^0
    while (p < max_val) {
        points.insert(p);
        p *= 2;
    }
    points.insert(max_val); // Asegura el último punto
    return points;
}

/**
 * run_memory_experiment :: Vector -> void
 * Función encargada de obtener el uso de memoria del Trie en diferentes puntos de su creación.
 * En particular se mide la cantidad de nodos que tiene el Trie para cada potencia de 2 palabras insertadas.
 * @param words: dataset 'words.txt'
 */
void run_memory_experiment(const vector<string>& words) {
    cout << "\n--- 1. Experimento de Memoria (Seccion 4.1) ---" << endl;
    cout << "Formato, Insercion_N, Nodos_Totales, Caracteres_Totales, Nodos_Por_Caracter" << endl;
    
    Trie trie;
    long long N = words.size();
    if (N == 0) return;
    
    // Puntos de medición {2^0, 2^1, ..., 2^17, N} [cite: 54]
    set<long long> measurement_points = create_measurement_points(N);
    long long total_chars_inserted = 0;
    
    for (long long i = 0; i < N; ++i) {
        trie.insert(words[i]);
        total_chars_inserted += words[i].length();
        
        // i+1 es el número de inserción (1-based)
        if (measurement_points.count(i + 1)) {
            double nodes_per_char = (total_chars_inserted == 0) ? 0 : 
                                    (double)trie.get_node_count() / total_chars_inserted;
            cout << "Memoria," << (i + 1) << "," 
                 << trie.get_node_count() << "," 
                 << total_chars_inserted << "," 
                 << nodes_per_char << endl;
        }
    }
}

/**
 * run_time_experiment :: Vector -> void
 * Función encargada de medir el tiempo de inserción de cada conjunto de N/M palabras a lo largo de la creación del Trie.
 * @param words: dataset 'words.txt'
 */
void run_time_experiment(const vector<string>& words) {
    cout << "\n--- 2. Experimento de Tiempo (Seccion 4.2) ---" << endl;
    cout << "Formato, Grupo_M, Tiempo_us, Caracteres_Grupo, us_Por_Caracter" << endl;
    
    Trie trie; // Un trie nuevo para medir la inserción
    long long N = words.size();
    int M = 16; // 16 grupos 
    if (N == 0 || N % M != 0) {
        cerr << "Error: Tamaño del dataset (" << N << ") no es divisible por M=16" << endl;
        return;
    }
    
    long long group_size = N / M;
    
    for (int g = 0; g < M; ++g) {
        long long start_index = g * group_size;
        long long end_index = (g + 1) * group_size;
        long long chars_in_group = 0;
        
        auto start_time = chrono::high_resolution_clock::now();
        
        for (long long i = start_index; i < end_index; ++i) {
            trie.insert(words[i]);
            chars_in_group += words[i].length();
        }
        
        auto end_time = chrono::high_resolution_clock::now();
        long long duration_us = chrono::duration_cast<chrono::microseconds>(end_time - start_time).count();
        
        double time_per_char = (chars_in_group == 0) ? 0 : 
                               (double)duration_us / chars_in_group;
        
        cout << "Tiempo," << (g + 1) << "," 
             << duration_us << "," 
             << chars_in_group << "," 
             << time_per_char << endl;
    }
}

/**
 * run_autocomplete_simulation - VERSIÓN CON DEBUG
 * Incluye verificaciones para entender qué está pasando
 */
void run_autocomplete_simulation(Trie& trie, const vector<string>& text_data, 
                                 const string& variant_name, const string& dataset_name) {
    
    cout << "\n--- 3. Experimento de Autocompletado (" << variant_name << ", " << dataset_name << ") ---" << endl;
    cout << "Formato, Dataset, Variante, Palabra_N, Caracteres_Escritos, Caracteres_Totales, Porcentaje_Escrito" << endl;
    
    long long L = text_data.size();
    if (L == 0) return;
    
    set<long long> measurement_points = create_measurement_points(L);
    
    long long total_chars_typed = 0;
    long long total_chars_in_text = 0;
    long long successful_autocompletes = 0;  // Para debug
    long long words_not_in_trie = 0;  // Para debug
    
    auto sim_start_time = chrono::high_resolution_clock::now();
    
    for (long long i = 0; i < L; ++i) {
        const string& w = text_data[i];
        if (w.empty()) continue;
        
        TrieNode* current = trie.root;
        int chars_typed_count = 0;
        bool word_found_in_trie = false;
        bool autocomplete_success = false;
        
        total_chars_in_text += w.length();
        
        // Simular escritura letra por letra
        for (size_t char_idx = 0; char_idx < w.length(); ++char_idx) {
            char c = w[char_idx];
            TrieNode* next_node = trie.descend(current, c);
            
            if (next_node == nullptr) {
                // La palabra no pertenece al trie
                total_chars_typed += w.length();
                word_found_in_trie = false;
                words_not_in_trie++;
                break;
            }
            
            current = next_node;
            chars_typed_count++;
            word_found_in_trie = true;
            
            // Verificar autocompletado después de cada letra
            TrieNode* suggestion = trie.autocomplete(current);
            
            if (suggestion != nullptr && suggestion->str != nullptr) {
                if (*(suggestion->str) == w) {
                    // ¡Autocompletado exitoso!
                    autocomplete_success = true;
                    total_chars_typed += chars_typed_count;
                    successful_autocompletes++;
                    break;
                }
            }
        }
        
        // Si llegamos al final sin autocompletar
        if (word_found_in_trie && !autocomplete_success) {
            total_chars_typed += w.length();
        }
        
        // Actualizar prioridad
        if (word_found_in_trie) {
            TrieNode* terminal = get_terminal(trie, w);
            if (terminal != nullptr && trie.is_terminal_node(terminal)) {
                if (variant_name == "frecuencia") {
                    trie.update_priority_frequency(terminal);
                } else {
                    trie.update_priority_recent(terminal);
                }
            }
        }
        
        // Reportar en los puntos de medición 
        if (measurement_points.count(i + 1)) {
            double percentage_typed = (total_chars_in_text == 0) ? 0 :
                                      (double) total_chars_typed / total_chars_in_text;
            cout << "Autocompletar," << dataset_name << "," << variant_name << "," 
                 << (i + 1) << "," 
                 << total_chars_typed << "," 
                 << total_chars_in_text << "," 
                 << 100- (percentage_typed * 100.0) << endl;
        }
    }
    
    auto sim_end_time = chrono::high_resolution_clock::now();
    long long duration_ms = chrono::duration_cast<chrono::milliseconds>(sim_end_time - sim_start_time).count();
    
    // Información de debug
    cout << "\n--- Debug Info (" << variant_name << ", " << dataset_name << ") ---" << endl;
    cout << "Autocompletados exitosos: " << successful_autocompletes << " de " << L << endl;
    cout << "Palabras no en trie: " << words_not_in_trie << endl;
    double ahorro_porcentual = (total_chars_in_text == 0) ? 0 :
                                100.0 * (1.0 - (double)total_chars_typed / total_chars_in_text);
    cout << "Ahorro total: " << ahorro_porcentual << "%" << endl;
    
    cout << "\n--- Resumen de Tiempo de Simulacion (" << variant_name << ", " << dataset_name << ") ---" << endl;
    cout << "Formato, Dataset, Variante, TiempoTotal_ms, PalabrasTotales, CaracteresTotales, ms_Por_Palabra, ms_Por_Caracter" << endl;
    
    double ms_per_word = (L == 0) ? 0 : (double)duration_ms / L;
    double ms_per_char = (total_chars_in_text == 0) ? 0 : (double)duration_ms / total_chars_in_text;
    
    cout << "SimTiempo," << dataset_name << "," << variant_name << "," 
         << duration_ms << "," 
         << L << "," 
         << total_chars_in_text << "," 
         << ms_per_word << "," 
         << ms_per_char << endl;
}

/**
 * Función Main
 * Orquesta la carga de archivos y la ejecución de los 3 experimentos
 * para ambas variantes (frecuencia y reciente).
 */
int main() {
    // Configurar precisión de salida para decimales
    cout << fixed << setprecision(6);
    
    // --- Carga de Datasets ---
    cout << "Cargando dataset principal 'words.txt'..." << endl;
    const long long N = 262144; // 2^18
    vector<string> words = load_words("words.txt");
    if (words.empty() || words.size() != N) {
        cerr << "Error: 'words.txt' no se pudo cargar o no tiene N=" << N << " palabras." << endl;
        cerr << "Cargadas: " << words.size() << " palabras." << endl;
        // Se puede continuar si se cargaron algunas, pero los experimentos
        // de N/M fallarán si el número no es exacto.
        if (words.empty()) return 1;
    }
    
    cout << "Cargando dataset de simulacion 'wikipedia.txt'..." << endl;
    const long long L = 4194304; // 2^22
    vector<string> wiki_words = load_words("wikipedia.txt");
    if (wiki_words.empty() || wiki_words.size() != L) {
        cerr << "Advertencia: 'wikipedia.txt' no se pudo cargar o no tiene L=" << L << " palabras." << endl;
        cerr << "Cargadas: " << wiki_words.size() << " palabras." << endl;
    }
    
    cout << "Cargando dataset de simulacion 'random.txt'..." << endl;
    vector<string> rand_words = load_words("random.txt");
    if (rand_words.empty() || rand_words.size() != L) {
        cerr << "Advertencia: 'random.txt' no se pudo cargar o no tiene L=" << L << " palabras." << endl;
        cerr << "Cargadas: " << rand_words.size() << " palabras." << endl;
    }
    
    cout << "Cargando dataset de simulacion 'random_with_distribution.txt'..." << endl;
    vector<string> rand_dist_words = load_words("random_with_distribution.txt");
    if (rand_dist_words.empty() || rand_dist_words.size() != L) {
        cerr << "Advertencia: 'random_with_distribution.txt' no se pudo cargar o no tiene L=" << L << " palabras." << endl;
        cerr << "Cargadas: " << rand_dist_words.size() << " palabras." << endl;
    }
    
    // --- Ejecución de Experimentos de tiempo y memoria en la creación del Trie ---
    
    run_memory_experiment(words);
    run_time_experiment(words);
    
    // --- Ejecución de Experimento 3 (Variante Frecuencia) ---
    { // Se usa un bloque para que el trie se destruya y libere memoria
        cout << "\n=============================================" << endl;
        cout << "INICIANDO SIMULACION: VARIANTE FRECUENCIA" << endl;
        cout << "=============================================" << endl;
        
        Trie trie_freq;
        cout << "Construyendo trie para Frecuencia..." << endl;
        for (const string& w : words) {
            trie_freq.insert(w);
        }
        cout << "Trie construido. Nodos: " << trie_freq.get_node_count() << endl;
        
        if (!wiki_words.empty())
            run_autocomplete_simulation(trie_freq, wiki_words, "frecuencia", "wikipedia.txt");
        if (!rand_words.empty())
            run_autocomplete_simulation(trie_freq, rand_words, "frecuencia", "random.txt");
        if (!rand_dist_words.empty())
            run_autocomplete_simulation(trie_freq, rand_dist_words, "frecuencia", "random_with_distribution.txt");
    }
    
    // --- Ejecución de Experimento 3 (Variante Reciente) ---
    { // Bloque para liberar memoria del trie anterior
        cout << "\n=============================================" << endl;
        cout << "INICIANDO SIMULACION: VARIANTE RECIENTE" << endl;
        cout << "=============================================" << endl;
        
        Trie trie_recent;
        cout << "Construyendo trie para Reciente..." << endl;
        for (const string& w : words) {
            trie_recent.insert(w);
        }
        cout << "Trie construido. Nodos: " << trie_recent.get_node_count() << endl;
        
        if (!wiki_words.empty())
            run_autocomplete_simulation(trie_recent, wiki_words, "reciente", "wikipedia.txt");
        if (!rand_words.empty())
            run_autocomplete_simulation(trie_recent, rand_words, "reciente", "random.txt");
        if (!rand_dist_words.empty())
            run_autocomplete_simulation(trie_recent, rand_dist_words, "reciente", "random_with_distribution.txt");
    }
    
    cout << "\n--- Todos los experimentos completados ---" << endl;
    
    return 0;
}