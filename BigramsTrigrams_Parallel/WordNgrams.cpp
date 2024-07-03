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
    int count;
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
    /*
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
   */

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
    /*
    while (iss >> word) {
        if (num_words >= n - 1) {   // controllo se ho raggiunto la lunghezza giusta per formare n-gramma
            string ngram = "";
            for (int i = 0; i < n - 1; i++) {
                ngram += previous_words[i] + " ";           //riempio stringa con l'n-1 - gramma
            }
            ngram += word;                                  //aggiungo word attuale e completo n-gramma
            histogram[ngram]++;
        }
        for (int i = 0; i < n - 2; i++) {           // ciclo per spostare (destra a sinistra) le parole precedenti via via
            previous_words[i] = previous_words[i + 1];
        }
        previous_words[n - 2] = word;   // salvo word attuale in posizione n-2 (in coda)
        num_words++;                    //incremento contatore n-gramma
    }
    std::cout<<"finito lettura"<< std::endl;
    printHistogram(histogram);
}
     */

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
    /*
    std::ifstream inputFile(filename);
    if (!inputFile.is_open() || !inputFile.good()) {

        std::cout << "Impossibile aprire il file o l'input non e' valido." << std::endl;
        exit(EXIT_FAILURE);

    }

    std::stringstream buffer;
    buffer << inputFile.rdbuf();
    std::string text = buffer.str();
    std::istringstream iss(text);


    std::vector<std::string> words(std::istream_iterator<std::string>{iss},
                                   std::istream_iterator<std::string>());
                                   */
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

        //std::cout<<"thread: "<< omp_get_thread_num()<< " start: "<<threadStartIdx<<" end: "<<threadEndIdx<< "tot: "<<threadEndIdx - threadStartIdx<< " tot thread: "<<omp_get_num_threads()<<std::endl;
        for (int i = threadStartIdx; i <= threadEndIdx; i++) {
            int countW = 0;
            std::string currentNgram;

            for (int j = i; j < i + n; j++) {
                if(words[j] != "") {
                    currentNgram += words[j] + " ";
                    countW++;
                }
            }
/*
            if(i == threadEndIdx)
            {
                //std::cout<<"ultima "<<omp_get_thread_num() <<": "<<words[threadEndIdx]<<" "<< currentNgram <<std::endl;
                //std::cout<<"entratoPR "<< omp_get_thread_num() <<" "<<currentNgram<<std::endl;
                std::istringstream iss(currentNgram);
                int count = 0;
                std::string word;
                std::cout<<" entrato: "<< countW<<" id: "<<omp_get_thread_num()<<" "<<currentNgram<<" lung: "<<currentNgram.length()<<std::endl;
                while (iss >> word) {
                    count++;
                }

                //std::cout<<currentNgram<<"  "<< currentNgram.length()<<std::endl;
                if(count != n)
                {

                    ok = false;
                    //std::cout<<"entrato "<<omp_get_thread_num()<<std::endl;
                    for(int j = i+1; j<= threadEndIdx + (n - count); j++)
                    {

                        currentNgram += words[j] + " ";
                    }
                }
            }*/
            if(countW == n) { //aggiungo solo se n-gramma di lunghezza corretta
                localHistogram[currentNgram]++;
            }
        }


//#pragma omp critical(updateHistogram)
        //{
        /*
        auto endP = std::chrono::steady_clock::now();


        auto durationP = std::chrono::duration_cast<std::chrono::milliseconds>(endP - startP);
#pragma omp critical
        std::cout << "durata fino a update: "<< durationP.count() << std::endl;
         */
#pragma omp critical(updateHistogram)
            for (auto [ngram, count]: localHistogram) {

                histogram[ngram] += count;
            }

       // }
#pragma omp barrier
    }

    printHistogram(histogram);
}







