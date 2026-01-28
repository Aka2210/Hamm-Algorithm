#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>
#include <iomanip>
#include <cmath>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <unistd.h>
#include <sys/resource.h>
#endif

using namespace std;

int max_id_found = 0;

map<string, int> str_to_id;
vector<string> id_to_str;

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

long get_memory_usage() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.PeakWorkingSetSize / 1024;
    }
    return 0;
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss;
    }
    return 0;
#endif
}

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

void write_output(vector<int> pattern, int support, ofstream& outfile, int total_transactions) {
    sort(pattern.begin(), pattern.end());
    for (size_t i = 0; i < pattern.size(); i++) {
        outfile << pattern[i] << (i != pattern.size() - 1 ? " " : "");
    }

    outfile << " #SUP: " << support << "\n";
}

Node* construct_tree(const vector<pair<vector<int>, int>>& condPaths, vector<Header*>& newHeaders) {
    Node* newRoot = new Node();
    vector<Header*> headerMap(1000, nullptr); 
    for (auto* h : newHeaders) {
        headerMap[h->item] = h;
        h->next = nullptr; 
        h->tail = nullptr;
    }

    for (auto& p : condPaths) {
        const vector<int>& rawPath = p.first;
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
    return newRoot;
}

bool is_single_node(Header* header) {
    return (header->next != nullptr && header->next->hlink == nullptr);
}

void hamm_search_optimized(int index, int current_sum, vector<int>& current_pattern, 
                           const vector<int>& IL, const vector<int>& UL, 
                           int min_sup, ofstream& outfile, int total_transactions) {
    if (index == IL.size()) {
        if (!current_pattern.empty()) {
            write_output(current_pattern, current_sum, outfile, total_transactions);
        }
        return;
    }

    int sum_without_current = current_sum;
    
    if (sum_without_current >= min_sup) {
        hamm_search_optimized(index + 1, sum_without_current, current_pattern, IL, UL, min_sup, outfile, total_transactions);
    }

    current_pattern.push_back(IL[index]);
    hamm_search_optimized(index + 1, current_sum, current_pattern, IL, UL, min_sup, outfile, total_transactions);
    current_pattern.pop_back();
}

void FP_Growth(Node* root, vector<Header*>& headers, vector<int> prefix, int min_sup, ofstream& outfile, int total_transactions) {
    for(auto& header : headers){
        vector<int> newPattern = prefix;
        newPattern.push_back(header->item);
        
        write_output(newPattern, header->freq, outfile, total_transactions);

        if (is_single_node(header)) {
            vector<int> IL, UL;
            IL.reserve(50); 
            UL.reserve(50);
            
            Node* curr = header->next->parent;
            while (curr->item != -1) {
                IL.push_back(curr->item);
                UL.push_back(header->freq);
                curr = curr->parent;
            }
            hamm_search_optimized(0, header->freq, newPattern, IL, UL, min_sup, outfile, total_transactions);
            continue; 
        }
        
        Node* node = header->next;
        vector<int> condCounts(1000, 0);
        vector<pair<vector<int>, int>> condPaths;
        
        while(node){ 
            Node* parent = node->parent;
            vector<int> path;
            while(parent->item != -1){
                condCounts[parent->item] += node->freq;
                path.push_back(parent->item);
                parent = parent->parent;
            }
            if(!path.empty()) condPaths.push_back({path, node->freq});
            node = node->hlink;
        }

        vector<Header*> newHeaders;
        for(int item = 0; item < 1000; item++){
            int count = condCounts[item];
            if(count >= min_sup){
                Header* h = new Header();
                h->item = item;
                h->freq = count;
                newHeaders.push_back(h);
            }
        }
        if(newHeaders.empty()) continue;

        sort(newHeaders.begin(), newHeaders.end(), [](Header* a, Header* b){
            if(a->freq == b->freq) return a->item < b->item; 
            return a->freq < b->freq; 
        });

        Node* newRoot = construct_tree(condPaths, newHeaders);
        FP_Growth(newRoot, newHeaders, newPattern, min_sup, outfile, total_transactions);
    }
}

int main(int argc , char* argv[]) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(NULL);
    if (argc < 4) return 1;
    float min_sup_rate = stof(argv[1]);
    string input_file = argv[2];
    string output_file = argv[3];
    
    ifstream infile(input_file);
    if(!infile.is_open()) return 1;

    vector<vector<int>> transactions;
    map<int, int> temp_counts;
    string line;

    while(getline(infile, line)){
        if(line.empty()) continue;
        stringstream ss(line);
        int item_id;
        vector<int> transaction;
        while(ss >> item_id) {
            transaction.push_back(item_id);
            temp_counts[item_id]++;
            if(item_id > max_id_found) max_id_found = item_id;
        }
        transactions.push_back(transaction);
    }
    infile.close();

    int min_sup = (int)ceil(min_sup_rate * transactions.size());

    vector<Header*> headers;
    vector<Header*> header_map(max_id_found + 1, nullptr);
    for(auto const& [id, freq] : temp_counts) {
        if(freq >= min_sup) {
            Header* h = new Header();
            h->item = id;
            h->freq = freq;
            headers.push_back(h);
            header_map[id] = h;
        }
    }

    sort(headers.begin(), headers.end(), [](Header* a, Header* b){
        if(a->freq == b->freq) return a->item < b->item;
        return a->freq < b->freq;
    });

    remove_infrequent_items(headers, min_sup);

    vector<pair<vector<int>, int>> initialPaths;
    for(auto& trans : transactions) {
        vector<int> filtered_path;
        for(int id : trans) {
            if(header_map[id]) filtered_path.push_back(id);
        }
        if(!filtered_path.empty()) initialPaths.push_back({filtered_path, 1});
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    Node* root = construct_tree(initialPaths, headers);
    
    ofstream outfile(output_file);
    if (!outfile.is_open()) return 1;

    FP_Growth(root, headers, {}, min_sup, outfile, transactions.size());

    outfile.close();

    auto end_time = std::chrono::high_resolution_clock::now();
    long final_memory = get_memory_usage();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    cout << "===== Performance Report =====" << "\n";
    cout << "Time Elapsed: " << duration.count() << " ms" << "\n";
    cout << "Memory Usage (Peak): " << final_memory << " KB" << "\n";
    cout << "==============================" << "\n";
    
    return 0;
}