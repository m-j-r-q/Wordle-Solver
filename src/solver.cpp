#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <cmath>
#include <stdint.h>
#include <emscripten/bind.h>

using namespace std;

struct SolverData {
    vector<string> words;
    vector<uint8_t> table;
    int n;

    uint8_t getPattern(int guessIdx, int targetIdx) const {
        return table[guessIdx * n + targetIdx];
    }

    string getWord(int Idx) const {
        return words[Idx];
    }
};

vector<string> loadWords(const string& filename) {
    
    ifstream word_file(filename);

    if (!word_file.is_open()) {
        cerr << "Error: Could not open the file!" << endl;
        return {};
    }
    
    vector<string> words;
    string word;

    while (getline(word_file, word)) {
            if (!word.empty() && word.back() == '\r') word.pop_back();

            if (word.length() != 5) continue;

            bool valid = true;
            for (char c : word) {
                if (!islower(c)) { valid = false; break; }
            }
                if (valid) words.push_back(word);
    
    }
    
    word_file.close();
    return words;
}

array<int,5> getPatternArray(const string& guess, const string& target) {
    array<int,5> Pattern = {0,0,0,0,0};
    array<int,5> Markedg = {0,0,0,0,0};
    array<int,5> Markedt = {0,0,0,0,0};

    // Mark Greens
    for (int i = 0; i<5; i++) {
        if (guess[i] == target[i]){
            Pattern[i] = 2;
            Markedg[i] = 1;
            Markedt[i] = 1;
        }
    }

    // Mark Yellows
    for (int i = 0; i<5; i++) {
        bool trig = false;
        if (!Markedg[i]){
            for (int j = 0; j<5; j++) {
                if (!Markedt[j] && !trig){
                    if (guess[i] == target[j]) {
                        Pattern[i] = 1;
                        Markedg[i] = 1;
                        Markedt[j] = 1;
                        trig = true;
                    }
                } 
            }
        }
    }

    return Pattern;
}

uint8_t encodePattern(const array<int,5>& g) {
    return g[0] + g[1]*3 + g[2]*9 + g[3]*27 + g[4]*81; 
}

SolverData buildTable(const vector<string>& word_vec, int n) {
    SolverData data;
    data.n = n;
    data.words = word_vec;
    data.table.resize(n * n);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            data.table[i*n + j] = encodePattern(getPatternArray(word_vec[i], word_vec[j]));

    return data;
}

vector<int> filterIndices(const vector<int>& candidates, int guessIdx, uint8_t Pattern, const SolverData& data) {
    vector<int> New;
    for (int Idx : candidates) {
        if (data.getPattern(guessIdx, Idx) == Pattern) {New.push_back(Idx);}
    }
    return New;
    
}

double getEntropy(int guessIdx, const vector<int>& candidateIndices, const SolverData& data) {
    unordered_map<uint8_t,int> Pattern_Freq;
    for (int targetIdx: candidateIndices) {
        Pattern_Freq[data.getPattern(guessIdx, targetIdx)]++;
    }

    double Entropy = 0;
    double total = candidateIndices.size();
    for (const auto& [key,value] : Pattern_Freq) {
        double p = static_cast<double>(value) / total;
        Entropy += p *log2(1.0/p);
    } 
    return Entropy;
}

int bestGuess(const vector<int>& candidateIndices, const SolverData& data) {
    
    if (candidateIndices.empty()) return -1;

    int best = -1;
    double bestEntropy = -1.0;
    for (int Idx : candidateIndices) {
        double h = getEntropy(Idx, candidateIndices, data);
        if (h > bestEntropy) {
            bestEntropy = h;
            best = Idx;
        }
    }
    return best;
}

array<int,5> getArray(const string& p) {
    
    array<int,5> Array = {0,0,0,0,0};

    for (int i = 0; i < 5; i++)
    {
        if (p[i] == 'G'){Array[i] = 2;}
        else if (p[i] == 'Y'){Array[i] = 1;}
        else if (p[i] == 'B'){Array[i] = 0;}
    }
    
    return Array;
}

static SolverData solverData;
static vector<int> candidateIndices;
static unordered_map<string,int> wordIndex;

void initialize(const string& filename) {
    vector<string> words = loadWords(filename);
    int n = static_cast<int>(words.size());

    solverData = buildTable(words, n);

    candidateIndices.clear();
    wordIndex.clear();

    for (int i = 0; i < n; i++) {
        wordIndex[words[i]] = i;
        candidateIndices.push_back(i);
    }
}

string getSuggestion() {
    if (candidateIndices.empty()) return "";
    int idx = bestGuess(candidateIndices, solverData);
    return solverData.getWord(idx);
}

string applyGuess(const string& guess, const string& patternStr) {
    auto it = wordIndex.find(guess);
    if (it == wordIndex.end()) return "invalid";

    int guess_Idx = it->second;
    array<int,5> pattern_array = getArray(patternStr);
    array<int,5> solved = {2,2,2,2,2};

    if (pattern_array == solved) return "solved";

    candidateIndices = filterIndices(candidateIndices, guess_Idx, encodePattern(pattern_array), solverData);

    if (candidateIndices.empty())   return "no_candidates";
    if (candidateIndices.size()==1) return "answer:" + solverData.getWord(candidateIndices[0]);
    return "ongoing";
}

int getRemainingCount() {
    return static_cast<int>(candidateIndices.size());
}

void reset() {
    candidateIndices.clear();
    for (int i = 0; i < solverData.n; i++) {
        candidateIndices.push_back(i);
    }
}

EMSCRIPTEN_BINDINGS(solver) {
    emscripten::function("initialize", &initialize);
    emscripten::function("getSuggestion", &getSuggestion);
    emscripten::function("applyGuess", &applyGuess);
    emscripten::function("getRemainingCount", &getRemainingCount);
    emscripten::function("reset", &reset);
}