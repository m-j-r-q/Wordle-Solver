#include <solver.cpp>

int main() {
    vector<string> candidate_string = loadWords("words_1.txt");
    int size = static_cast<int>(candidate_string.size());
    SolverData table = buildTable(candidate_string, size);
    vector<int> candidate_Idx;
    for (int i = 0; i < size; i++) {
        candidate_Idx.push_back(i);
    }

    bool trig = true;
    int turn = 0;
    
    // Turn Loop.
    while (trig && turn<=6) {
        turn++;
        int suggestion_Idx = bestGuess(candidate_Idx,table);
        string guess;
        string pattern;

        cout << "Try the word: " << table.getWord(suggestion_Idx) << endl;

        int guess_Idx = -1;
        // Validity Checks for Inputs.
        while (guess_Idx == -1) {
            cout << "What is your guess: ";
            cin >> guess;
            cout << "What is your pattern (e.g GYBYG): ";
            cin >> pattern;

            for (char &c : pattern) {
                c = std::toupper(static_cast<unsigned char>(c));
            }

            if (guess.length() != 5 || pattern.length() != 5) {
                cout << "Invalid input, both must be 5 characters." << endl;
                continue;
            }

            for (int i = 0; i < (int)candidate_string.size(); i++) {
                if (candidate_string[i] == guess) { guess_Idx = i; break; }
            }

            if (guess_Idx == -1) cout << "Word not in list, try again." << endl;
        }

        array<int,5> pattern_array = getArray(pattern);
        array<int,5> hi = {2,2,2,2,2};

        // Check whether solved or not, else update Vector Indices based on guess.
        if (pattern_array == hi) {
            cout << "Solved in " << turn << " turns!" << endl;
            trig = false;
        }
        else {
            candidate_Idx = filterIndices(candidate_Idx, guess_Idx, encodePattern(pattern_array), table);
            if (candidate_Idx.empty()) {
                cout << "No candidates remain, check your pattern inputs." << endl;
                trig = false;
            }
            else if (candidate_Idx.size() == 1) {
                cout << "Answer is: " << table.getWord(candidate_Idx[0]) << endl;
                trig = false;
            }
        }

        // Turns Exceeded.
        if (turn > 6 && trig) {
            cout << "Failed to solve. Remaining candidates: ";
            for (const int& w : candidate_Idx) cout << table.getWord(w) << " ";
            cout << endl;
        }
        
    }
}