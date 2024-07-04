//
// Created by Giacomo Magistrato on 16/01/24.
//

#include "WordNgrams.h"
#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <cctype>
#include <unordered_map>
#include <omp.h>
#include <sstream>
#include <map>
using namespace std;
static std::vector<std::string> words;


void WordNgrams::initWords(const string &filename){
    int n = this->NgramLength;
    unordered_map<string, int> histogram;
    std::ifstream inputFile(filename);
    if (!inputFile.is_open() || !inputFile.good()) {

        std::cout << "Impossibile aprire il file o l'input non e' valido." << std::endl;
        exit(EXIT_FAILURE);

    }
    //vector<string> previous_words(n - 1); //verrà usato per tenere traccia delle parole precedenti nell'elaborazione degli ngrammi

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string text = buffer.str();
    std::istringstream iss(text);

    words = std::vector<std::string>(std::istream_iterator<std::string>(iss),
                                     std::istream_iterator<std::string>());
}
int WordNgrams::getNgramLength() const {
    return NgramLength;
}

void printHistogram(unordered_map<string, int> histogram) {
    string ngram;

    //ordino l'istogramma in ordine decrescente
    priority_queue<pair<int, string>> q;
    for (auto& [ngram, count]: histogram) {
        q.push({count, ngram});
    }
    // stampo le parole più numerose
    for (int i = 0; i < 10; i++) {
        auto [count, ngram] = q.top();
        q.pop();
        cout << ngram << ": " << count << endl;
    }
}


void WordNgrams::runWordNgrams(const string& filename) {
    cout << "Calcolo n-grammi di parole Sequenziale..." << endl;

    int n = this->NgramLength;
    unordered_map<string, int> histogram;

    for (size_t i = 0; i <= words.size() - n-1; i++) {
        std::string currentNgram;
        int countW = 0;
        for (int j = 0; j < n; j++) {
            if(words[j] != "") {
                currentNgram += words[i + j] + " ";
                countW++;
            }
        }
        if(countW == n) {
            histogram[currentNgram]++;
        }
    }

    printHistogram(histogram);
}


void WordNgrams::runWordNgrams_parallel2(const string& filename) {
        auto startP = std::chrono::steady_clock::now();
        cout << "Calcolo n-grammi di parole..." << endl;
        int n = this->NgramLength;
        unordered_map<string, int> histogram;

        std::ifstream inputFile(filename);
        if (!inputFile.is_open() || !inputFile.good()) {

            std::cout << "Impossibile aprire il file o l'input non e' valido." << std::endl;
            exit(EXIT_FAILURE);

        }
        //vector<string> previous_words(n - 1); //verrà usato per tenere traccia delle parole precedenti nell'elaborazione degli ngrammi

        std::stringstream buffer;
        buffer << inputFile.rdbuf();
        std::string text = buffer.str();
        std::istringstream iss(text);

        std::vector<std::string> words(std::istream_iterator<std::string>{iss},
                                       std::istream_iterator<std::string>());
#pragma omp parallel for shared(histogram)
        for (size_t i = 0; i <= words.size() - n-1; i++) {
            std::string currentNgram;
            int countW = 0;
            for (int j = 0; j < n; j++) {
                if(words[j] != "") {
                    currentNgram += words[i + j] + " ";
                    countW++;
                }
            }
            if(countW == n) {
#pragma omp atomic
                histogram[currentNgram]++;
            }
        }
        auto endP = std::chrono::steady_clock::now();
        auto durationP = std::chrono::duration_cast<std::chrono::milliseconds>(endP - startP);
        std::cout<<"durata fino a print: "<< durationP.count()<<std::endl;
#pragma omp barrier
        printHistogram(histogram);
    }


void WordNgrams::runWordNgrams_parallel(const string& filename) {

    std::cout << "Calcolo n-grammi di parole in parallelo..." << endl;
    int n = this->NgramLength;
    unordered_map<string, int> histogram;
    int numThreads = 4;//omp_get_num_threads(); // Definisci il numero di thread desiderato
    omp_set_num_threads(numThreads);
    int start_idx = 0;
    int end_idx = words.size();
#pragma omp parallel defaut(none) shared(histogram, start_idx, end_idx)
    {
        int numThreads = omp_get_num_threads() ;
        int threadNum = omp_get_thread_num();

        int itemsPerThread = (end_idx - start_idx + 1) / numThreads;
        std::unordered_map<std::string, int> localHistogram;

        int threadStartIdx = start_idx + threadNum * itemsPerThread;
        int threadEndIdx = threadStartIdx + itemsPerThread - 1;

        if (threadNum == numThreads - 1) {
            // The last thread may have extra items.
            threadEndIdx = end_idx -1;
        }

        for (int i = threadStartIdx; i <= threadEndIdx; i++) {
            int countW = 0;
            std::string currentNgram;

            for (int j = i; j < i + n; j++) {
                if(words[j] != "") {
                    currentNgram += words[j] + " ";
                    countW++;
                }
            }
            if(countW == n) { //aggiungo solo se n-gramma di lunghezza corretta
                localHistogram[currentNgram]++;
            }
        }

#pragma omp critical(updateHistogram)
            for (auto [ngram, count]: localHistogram) {

                histogram[ngram] += count;
            }
#pragma omp barrier
    }

    printHistogram(histogram);
}







