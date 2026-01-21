#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <iomanip>
#include <cmath>

using namespace std;

struct Node {
    int item = -1;
    int freq = 0;
    Node* parent = nullptr;
    vector<pair<int, Node*>> children; 
    Node* hlink = nullptr;
};

struct Header {
    int item;
    int freq = 0;
    Node* next = nullptr;
    Node* tail = nullptr;
};


void remove_infrequent_items(vector<Header*>& itemList, int min_sup) {
    for (auto it = itemList.begin(); it != itemList.end();) {
        if ((*it) == nullptr || (*it)->freq < min_sup) {
            it = itemList.erase(it); 
        } else {
            ++it; 
        }
    }
}

int main(int argc , char* argv[]) {
    if (argc < 4) return 1;

    float min_sup_rate = stof(argv[1]);
    string input_file = argv[2];
    string output_file = argv[3];
    
    ifstream infile(input_file);
    if(!infile.is_open()){
        cerr << "Error opening input file." << endl;
        return 1;
    }

    vector<Header*> headers(1000, nullptr);
    vector<vector<Header*>> transactions;
    string line;
    while(getline(infile, line)){
        stringstream ss(line);
        string item;
        vector<Header*> transaction;
        while (getline(ss, item, ',')) {
            int itemVal = stoi(item);
            if(headers[itemVal] == nullptr){
                headers[itemVal] = new Header();
                headers[itemVal]->item = itemVal;
                headers[itemVal]->freq = 1;
            }
            else headers[itemVal]->freq++;
            transaction.push_back(headers[itemVal]);
        }
        transactions.push_back(transaction);
    }
    infile.close();

    int min_sup = (int)ceil(min_sup_rate * transactions.size());
    
    sort(headers.begin(), headers.end(), [](Header* a, Header* b){
        if(a != nullptr && b != nullptr && a->freq == b->freq) return a->item > b->item;
        return a != nullptr && b != nullptr ? a->freq < b->freq : a != nullptr;
    });
    
    remove_infrequent_items(headers, min_sup);
    
    for(auto& transaction : transactions){
        sort(transaction.begin(), transaction.end(), [](Header* a, Header* b){
            if(a->freq == b->freq) return a->item < b->item;
            return a->freq > b->freq;
        });
        remove_infrequent_items(transaction, min_sup);
    }
    return 0;
}