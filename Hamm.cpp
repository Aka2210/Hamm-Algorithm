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

Node* get_child(Node* parent, int item) {
    for (auto& pair : parent->children) {
        if (pair.first == item) return pair.second;
    }
    return nullptr;
}

void add_child(Node* parent, Node* child) {
    parent->children.push_back({child->item, child});
}

void remove_infrequent_items(vector<Header*>& itemList, int min_sup) {
    for (auto it = itemList.begin(); it != itemList.end();) {
        if ((*it) == nullptr || (*it)->freq < min_sup) {
            it = itemList.erase(it); 
        } else {
            ++it; 
        }
    }
}

void write_output(vector<int>& pattern, int support, ofstream& outfile, int total_transactions) {
    sort(pattern.begin(), pattern.end());
    for (size_t i = 0; i < pattern.size(); i++) {
        outfile << pattern[i] << (i != pattern.size() - 1 ? "," : "");
    }
    double ratio = (double)support / total_transactions;
    double final_support = std::round(ratio * 10000.0) / 10000.0;
    outfile << ":" << fixed << setprecision(4) << final_support << endl;
}

void FP_Growth(vector<Header*>& headers, vector<int> prefix, int min_sup, ofstream& outfile, int total_transactions) {
    for(auto& header : headers){
        vector<int> newPattern = prefix;
        newPattern.push_back(header->item);
        write_output(newPattern, header->freq, outfile, total_transactions);
        
        Node* node = header->next;
        vector<int> condCounts(1000, 0);
        vector<pair<vector<int>, int>> condPaths;
        
        while(node){ 
            Node* parent = node->parent;
            vector<int> path;
            path.push_back(node->item); 
            while(parent->item != -1){
                condCounts[parent->item] += node->freq;
                path.push_back(parent->item);
                parent = parent->parent;
            }

            if(!path.empty()) {
                reverse(path.begin(), path.end()); 
                condPaths.push_back({path, node->freq});
            }
            node = node->hlink;
        }

        vector<Header*> newHeaders;
        vector<Header*> headerMap(1000, nullptr); 
        
        for(int item = 0; item < 1000; item++){
            int count = condCounts[item];
            if(count >= min_sup){
                Header* h = new Header();
                h->item = item;
                h->freq = count;
                newHeaders.push_back(h);
                headerMap[item] = h;
            }
        }
        if(newHeaders.empty()) continue;

        sort(newHeaders.begin(), newHeaders.end(), [](Header* a, Header* b){
            if(a->freq == b->freq) return a->item < b->item;
            return a->freq < b->freq; 
        });

        Node* newRoot = new Node();
        for (auto& p : condPaths) {
            vector<int>& rawPath = p.first;
            int count = p.second;           

            vector<Header*> sortedPath;
            for (int item : rawPath) {
                if (headerMap[item] != nullptr) {
                    sortedPath.push_back(headerMap[item]);
                }
            }
            sort(sortedPath.begin(), sortedPath.end(), [](Header* a, Header* b){
                if (a->freq == b->freq) return a->item < b->item; 
                return a->freq > b->freq; 
            });

            Node* curr = newRoot;
            for (auto* header : sortedPath) { 
                Node* child = get_child(curr, header->item);
                if (child != nullptr) {
                    child->freq += count;   
                    curr = child;
                }
                else {
                    Node* node = new Node();
                    node->item = header->item;
                    node->freq = count; 
                    node->parent = curr;
                    
                    add_child(curr, node);

                    if (header->next == nullptr) {
                        header->next = node;
                        header->tail = node;
                    }
                    else {
                        header->tail->hlink = node;
                        header->tail = node;
                    }
                    curr = node;
                }
            }
        }
        FP_Growth(newHeaders, newPattern, min_sup, outfile, total_transactions);
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

    // build FP-tree
    Node* root = new Node();
    for(auto& transaction : transactions){
        Node* curr = root;
        for(auto* header : transaction){
            if(!header) continue;
            Node* child = get_child(curr, header->item);
            
            if(child != nullptr){
                child->freq++;
                curr = child;
            }
            else{
                Node* node = new Node();
                node->item = header->item;
                node->freq = 1;
                node->parent = curr;
                
                add_child(curr, node);
                
                if(header->next == nullptr){
                    header->next = node;
                    header->tail = node;
                }
                else{
                    header->tail->hlink = node;
                    header->tail = node;
                }
                curr = node;
            }
        }
    }
    
    ofstream outfile(output_file);
    if (!outfile.is_open()) return 1;
    FP_Growth(headers, {}, min_sup, outfile, transactions.size());
    outfile.close();
    return 0;
}