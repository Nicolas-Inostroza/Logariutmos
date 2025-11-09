#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "raylib.h"
#include "trie.h"

using namespace std;

const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;
const int MAX_WORDS_TO_LOAD = (1 << 18);  // 2^18

/**
 * Lee un archivo de palabras desde datasets/
 * @param filename: nombre del archivo en datasets/
 * @param words: vector donde se almacenan las palabras
 * @param limit: cantidad máxima de palabras a leer
 */
void read_words(const string& filename, vector<string>& words, long long limit = -1) {
    string filepath = "datasets/" + filename;
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Error: no se pudo abrir " << filepath << endl;
        return;
    }
    
    string word;
    long long count = 0;
    while (getline(file, word) && (limit == -1 || count < limit)) {
        if (!word.empty()) {
            words.push_back(word);
            count++;
        }
    }
    file.close();
    cout << "Leídas " << words.size() << " palabras de " << filepath << endl;
}

/**
 * Estructura para mantener el estado de la interfaz
 */
struct TrieInterface {
    Trie trie;
    string current_prefix;
    string current_text;
    int variant;  // 0 = frecuencia, 1 = reciente
    bool loading;
    
    TrieInterface() : variant(0), loading(true), current_prefix(""), current_text("") {}
    
    /**
     * Obtiene la sugerencia actual basada en el prefijo
     * @return: palabra sugerida o string vacío
     */
    string get_suggestion() {
        if (current_prefix.empty()) return "";
        
        TrieNode* current = trie.root;
        for (char c : current_prefix) {
            current = trie.descend(current, tolower(c));
            if (current == nullptr) return "";
        }
        
        TrieNode* suggestion = trie.autocomplete(current);
        if (suggestion && suggestion->str) {
            return *suggestion->str;
        }
        return "";
    }
    
    /**
     * Acepta la sugerencia actual
     */
    void accept_suggestion() {
        string suggestion = get_suggestion();
        if (!suggestion.empty()) {
            // Obtener el nodo terminal para actualizar prioridad
            TrieNode* current = trie.root;
            for (char c : suggestion) {
                current = trie.descend(current, tolower(c));
            }
            current = trie.descend(current, '$');
            
            if (current && variant == 0) {
                trie.update_priority_frequency(current);
            } else if (current && variant == 1) {
                trie.update_priority_recent(current);
            }
            
            current_text += suggestion + " ";
            current_prefix = "";
        }
    }
    
    /**
     * Ignora la sugerencia y escribe el prefijo actual
     */
    void write_prefix() {
        if (!current_prefix.empty()) {
            current_text += current_prefix + " ";
            current_prefix = "";
        }
    }
    
    /**
     * Borra el último carácter del prefijo
     */
    void backspace() {
        if (!current_prefix.empty()) {
            current_prefix.pop_back();
        } else if (!current_text.empty()) {
            // Si prefix está vacío, borrar del texto completo
            while (!current_text.empty() && current_text.back() == ' ') {
                current_text.pop_back();
            }
            while (!current_text.empty() && current_text.back() != ' ') {
                current_text.pop_back();
            }
        }
    }
    
    /**
     * Agrega un carácter al prefijo
     */
    void add_char(char c) {
        if (isalpha(c)) {
            current_prefix += tolower(c);
        }
    }
};

/**
 * Dibuja la interfaz gráfica
 */
void draw_interface(TrieInterface& interface, bool loading) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Fondo principal
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Color{240, 240, 245, 255});
    
    // Encabezado
    DrawRectangle(0, 0, WINDOW_WIDTH, 80, Color{52, 73, 94, 255});
    DrawText("TRIE AUTOCOMPLETADO", 20, 15, 32, RAYWHITE);
    
    string variant_text = (interface.variant == 0) ? "Modo: FRECUENCIA" : "Modo: RECIENTE";
    DrawText(variant_text.c_str(), 20, 50, 16, Color{200, 200, 200, 255});
    
    if (loading) {
        DrawText("Cargando dataset...", WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 20, 24, DARKBLUE);
        EndDrawing();
        return;
    }
    
    // Área de input
    int input_y = 110;
    DrawText("Prefijo actual:", 20, input_y, 18, DARKGRAY);
    
    // Caja del prefijo
    DrawRectangle(20, input_y + 30, 400, 50, RAYWHITE);
    DrawRectangleLines(20, input_y + 30, 400, 50, DARKGRAY);
    DrawText(interface.current_prefix.c_str(), 30, input_y + 40, 20, BLUE);
    
    // Sugerencia
    DrawText("Sugerencia:", 20, input_y + 100, 18, DARKGRAY);
    string suggestion = interface.get_suggestion();
    
    // Caja de sugerencia
    DrawRectangle(20, input_y + 130, 400, 50, Color{200, 220, 255, 255});
    DrawRectangleLines(20, input_y + 130, 400, 50, BLUE);
    
    if (!suggestion.empty()) {
        DrawText(suggestion.c_str(), 30, input_y + 140, 20, DARKBLUE);
    } else {
        DrawText("(ninguna)", 30, input_y + 140, 18, DARKGRAY);
    }
    
    // Instrucciones
    DrawText("CONTROLES:", 500, input_y, 18, DARKGRAY);
    DrawText("TAB - Aceptar sugerencia", 500, input_y + 35, 14, DARKBLUE);
    DrawText("ENTER - Escribir prefijo", 500, input_y + 55, 14, DARKBLUE);
    DrawText("BACKSPACE - Borrar", 500, input_y + 75, 14, DARKBLUE);
    DrawText("S - Cambiar modo", 500, input_y + 95, 14, DARKBLUE);
    DrawText("ESC - Salir", 500, input_y + 115, 14, RED);
    
    // Estadísticas
    DrawText("ESTADÍSTICAS:", 20, 350, 18, DARKGRAY);
    DrawText(("Nodos totales: " + to_string(interface.trie.get_node_count())).c_str(), 
             20, 380, 14, DARKBLUE);
    DrawText(("Caracteres en prefijo: " + to_string(interface.current_prefix.length())).c_str(), 
             20, 405, 14, DARKBLUE);
    
    // Texto escrito
    DrawText("TEXTO ESCRITO:", 20, 450, 18, DARKGRAY);
    DrawRectangle(20, 480, WINDOW_WIDTH - 40, 280, RAYWHITE);
    DrawRectangleLines(20, 480, WINDOW_WIDTH - 40, 280, DARKGRAY);
    
    // Envolver texto
    int text_x = 30;
    int text_y = 490;
    int line_height = 20;
    int max_width = WINDOW_WIDTH - 60;
    string current_line = "";
    
    for (char c : interface.current_text) {
        current_line += c;
        int text_width = MeasureText(current_line.c_str(), 14);
        
        if (text_width > max_width) {
            // Retroceder última palabra
            size_t last_space = current_line.rfind(' ');
            if (last_space != string::npos && last_space > 0) {
                string line_to_draw = current_line.substr(0, last_space);
                DrawText(line_to_draw.c_str(), text_x, text_y, 14, BLACK);
                current_line = current_line.substr(last_space + 1);
                text_y += line_height;
            }
        }
    }
    if (!current_line.empty()) {
        DrawText(current_line.c_str(), text_x, text_y, 14, BLACK);
    }
    
    EndDrawing();
}

int main() {
    // Inicializar raylib
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Trie Autocompletado con Raylib");
    SetTargetFPS(60);
    
    TrieInterface interface;
    
    // Cargar palabras en un thread separado (simulado)
    cout << "Inicializando interfaz..." << endl;
    cout << "Cargando dataset de palabras..." << endl;
    
    vector<string> words;
    read_words("words.txt", words, MAX_WORDS_TO_LOAD);
    
    if (words.empty()) {
        cerr << "Error: no se pudo cargar el dataset de palabras" << endl;
        CloseWindow();
        return 1;
    }
    
    cout << "Construyendo Trie..." << endl;
    for (const auto& word : words) {
        interface.trie.insert(word);
    }
    cout << "Trie listo con " << interface.trie.get_node_count() << " nodos" << endl;
    
    interface.loading = false;
    
    // Loop principal
    while (!WindowShouldClose()) {
        // Procesar input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {  // Caracteres imprimibles
                interface.add_char((char)key);
            }
            key = GetCharPressed();
        }
        
        // Teclas especiales
        if (IsKeyPressed(KEY_TAB)) {
            interface.accept_suggestion();
        }
        if (IsKeyPressed(KEY_ENTER)) {
            interface.write_prefix();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            interface.backspace();
        }
        if (IsKeyPressed(KEY_S)) {
            interface.variant = (interface.variant == 0) ? 1 : 0;
        }
        
        // Dibujar
        draw_interface(interface, interface.loading);
    }
    
    CloseWindow();
    return 0;
}