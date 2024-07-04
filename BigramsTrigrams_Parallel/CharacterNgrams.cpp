//
// Created by Giacomo Magistrato on 19/02/24.
//

#include "CharacterNgrams.h"
#include <iostream>
#include <functional>
#include <string>
#include <sstream>
#include <locale>
#include <omp.h>
void printCharacterHistogram(unordered_map<string, int> histogram) {
    string ngram;


    //ordino in modo decrescente
    priority_queue<pair<int, string>> q;

    for (auto& [ngram, count]: histogram) {
        count++;
        q.push({count, ngram});
    }

    // stampo n-grammi più numerosi

    for (int i = 0; i < 4; i++) {
        auto [count, ngram] = q.top();
        q.pop();
        cout << ngram << ": " << count << endl;
    }

}
/* VALIDO --
void CharacterNgrams::runCharacterNgrams(const string& filename) {

    cout << "Calcolo n-grammi di caratteri ..." << endl;
    unordered_map<string, int> histogram;

    ifstream file(filename);
    if (!file) {
        cout << "Errore nell'apertura file di input" << endl;
        return;
    }

    string word;
    vector<string> ngrams;
    while (file >> word) {      //scorro parole ed estraggo n-grammi caratteri
        ngrams = extractNgramsFromWord(word);
        for (auto ngram : ngrams) {
            histogram[ngram]++;
        }
    }
    printCharacterHistogram(histogram);
}

*/
void CharacterNgrams::runCharacterNgrams(const string& filename) {

    cout << "Calcolo n-grammi di caratteri ..." << endl;
    unordered_map<string, int> histogram;
    int n = getNgramLength();
    ifstream inputFile(filename);
    if (!inputFile) {
        cout << "Errore nell'apertura file di input" << endl;
        return;
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string text = buffer.str();
    for (size_t i = 0; i <= text.length() - n ; i++) {
        bool valid = true;
        std::string currentNgram = text.substr(i, n);
        for (char c: currentNgram) {
            if (!std::isalnum(c)) {
                valid = false;
                break;
            }
        }
        if(valid) {
            histogram[currentNgram]++;
        }
    }
    printCharacterHistogram(histogram);
}
/*
vector<string> CharacterNgrams::extractNgramsFromWord(const string& word) {

    int n = getNgramLength();
    vector<string> ngrams;

    if (word.size() >= n) {     //check se word è più lunga della lunghezza dell'n-gramma

        for (int i = 0; i < word.size() - (n - 1); i++) {

                ngrams.push_back(word.substr(i, n));

        }
    }

    return ngrams;

}
 */


void CharacterNgrams::runCharacterNgrams_parallel(const string& filename) {
    //std::locale loc;
    cout << "Calcolo n-grammi di caratteri ..." << endl;
    unordered_map<string, int> histogram;
    int n = getNgramLength();
    ifstream inputFile(filename);
    if (!inputFile) {
        cout << "Errore nell'apertura file di input" << endl;
        return;
    }
    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string text = buffer.str();

    int numThreads = 2;              // Definisci il numero di thread desiderato


    omp_set_num_threads(numThreads);
    int start_idx = 0;
    int end_idx = text.size();


#pragma omp parallel default(none) shared(start_idx, end_idx, n, text, histogram, cout)
    {
        int numThreads = omp_get_num_threads() ;
        //std::cout<<numThreads<<std::endl;
        int threadNum = omp_get_thread_num();
        int itemsPerThread = (end_idx - start_idx + 1) / numThreads;
        int threadStartIdx = start_idx + threadNum * itemsPerThread;
        int threadEndIdx = threadStartIdx + itemsPerThread - 1;

        std::unordered_map<std::string, int> localHistogram;
        if (threadNum == numThreads - 1) {
            // The last thread may have extra items.
            threadEndIdx = end_idx - 1;
        }

        for (size_t i = threadStartIdx; i <= threadEndIdx  ; i++) {
            bool valid = true;

            std::string currentNgram = text.substr(i, n);
            for (char c: currentNgram) {
                if (!std::isalnum(c)) {
                    valid = false;
                    break;
                }
            }
            if (valid) {
                localHistogram[currentNgram]++;
            }
        }

#pragma omp critical(updateHistogram)
        {
            for (const auto &pair: localHistogram) {
                histogram[pair.first] += pair.second;
            }
        }
    }
    printCharacterHistogram(histogram);
}

int CharacterNgrams::getNgramLength() const {
    return NgramLength;
}