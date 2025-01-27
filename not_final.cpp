#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <cctype> // For character processing
#include <algorithm> // For sorting
#include <iomanip>   // For formatting output

using namespace std;

// Function to normalize a word by converting to uppercase and removing punctuation
string normalizeWord(const string &word) {
    string normalized;
    for (char ch : word) {
        if (isalnum(ch)) { // Keep only alphanumeric characters
            normalized += toupper(ch); // Convert to uppercase
        }
    }
    return normalized;
}

// Function to calculate the top 100 frequent words and their normalized frequencies
map<string, double> calculateTopFrequentWords(const string &fileName, const vector<string> &stopWords) {
    ifstream file(fileName);
    if (!file.is_open()) {
        throw runtime_error("Error: Could not open file " + fileName);
    }

    map<string, int> wordCounts;
    int totalWords = 0;
    string word;

    // Count word frequencies
    while (file >> word) {
        word = normalizeWord(word);
        if (!word.empty() && find(stopWords.begin(), stopWords.end(), word) == stopWords.end()) {
            wordCounts[word]++;
            totalWords++;
        }
    }
    file.close();

    // Normalize frequencies
    map<string, double> normalizedFreq;
    for (const auto &entry : wordCounts) {
        normalizedFreq[entry.first] = static_cast<double>(entry.second) / totalWords;
    }

    // Sort by frequency and keep the top 100 words
    vector<pair<string, double>> sortedWords(normalizedFreq.begin(), normalizedFreq.end());
    sort(sortedWords.begin(), sortedWords.end(), [](const auto &a, const auto &b) {
        return b.second < a.second;
    });

    map<string, double> topWords;
    for (size_t i = 0; i < min<size_t>(100, sortedWords.size()); i++) {
        topWords[sortedWords[i].first] = sortedWords[i].second;
    }
    return topWords;
}

// Function to calculate similarity index between two books
double calculateSimilarity(const map<string, double> &freq1, const map<string, double> &freq2) {
    double similarity = 0.0;
    for (const auto &entry : freq1) {
        if (freq2.find(entry.first) != freq2.end()) {
            similarity += entry.second + freq2.at(entry.first);
        }
    }
    return similarity;
}

int main() {
    // List of stop words
    vector<string> stopWords = {"A", "AND", "AN", "OF", "IN", "THE"};

    // List of book files (you can replace this with the list of your books)
    vector<string> bookFiles = {
        "The Anatomy of Melancholy Part 1.txt",
        "The Anatomy of Melancholy Part 2.txt",
        "The Anatomy of Melancholy Part 3.txt",
        "The History of Ireland - Geoffrey Keating.txt"
    };

    int numBooks = bookFiles.size();
    vector<map<string, double>> bookFrequencies(numBooks);

    // Calculate normalized frequencies for each book
    for (int i = 0; i < numBooks; i++) {
        try {
            cout << "Processing: " << bookFiles[i] << endl;
            bookFrequencies[i] = calculateTopFrequentWords(bookFiles[i], stopWords);
        } catch (const exception &e) {
            cerr << e.what() << endl;
            return 1;
        }
    }

    // Build the similarity matrix
    vector<vector<double>> similarityMatrix(numBooks, vector<double>(numBooks, 0.0));
    for (int i = 0; i < numBooks; i++) {
        for (int j = i + 1; j < numBooks; j++) {
            double similarity = calculateSimilarity(bookFrequencies[i], bookFrequencies[j]);
            similarityMatrix[i][j] = similarity;
            similarityMatrix[j][i] = similarity;
        }
    }

    // Find the top 10 similar book pairs
    vector<pair<pair<int, int>, double>> similarityPairs;
    for (int i = 0; i < numBooks; i++) {
        for (int j = i + 1; j < numBooks; j++) {
            similarityPairs.push_back({{i, j}, similarityMatrix[i][j]});
        }
    }
    sort(similarityPairs.begin(), similarityPairs.end(), [](const auto &a, const auto &b) {
        return b.second > a.second;
    });

    // Display the top 10 similar book pairs
    cout << "\nTop 10 Similar Book Pairs (Cleaned Format):\n";
    for (int i = 0; i < 10 && i < similarityPairs.size(); i++) {
        int book1 = similarityPairs[i].first.first;
        int book2 = similarityPairs[i].first.second;
        cout << i + 1 << ". \"" << bookFiles[book1] << "\" and \"" << bookFiles[book2]
             << "\" - Similarity Index: " << fixed << setprecision(6) << similarityPairs[i].second << endl;
    }

    // Save results to a file
    ofstream outputFile("similarity_results.txt");
    if (outputFile.is_open()) {
        outputFile << "Top 10 Similar Book Pairs:\n";
        for (int i = 0; i < 10 && i < similarityPairs.size(); i++) {
            int book1 = similarityPairs[i].first.first;
            int book2 = similarityPairs[i].first.second;
            outputFile << i + 1 << ". \"" << bookFiles[book1] << "\" and \"" << bookFiles[book2]
                       << "\" - Similarity Index: " << fixed << setprecision(6) << similarityPairs[i].second << "\n";
        }
        outputFile.close();
        cout << "\nResults saved to similarity_results.txt\n";
    } else {
        cerr << "Error: Could not save results to file.\n";
    }

    return 0;
}