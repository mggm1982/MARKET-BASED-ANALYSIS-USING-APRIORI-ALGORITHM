#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <string>
#include <sstream>
#include <cmath>
#include <limits> 

using namespace std;

// Type Aliases for Readability
using Itemset = set<string>;
using Transactions = vector<Itemset>;
using SupportMap = map<Itemset, int>;

// Helper to print a set like {Milk, Bread}
void print_itemset(const Itemset& itemset) {
    cout << "{";
    for (auto it = itemset.begin(); it != itemset.end(); ++it) {
        cout << *it << (next(it) == itemset.end() ? "" : ", ");
    }
    cout << "}";
}

// Generates candidate itemsets of size k
set<Itemset> generate_candidates(const SupportMap& Lk_minus_1) {
    set<Itemset> candidates;
    int k = Lk_minus_1.begin()->first.size() + 1; // Determine target size
    for (auto it1 = Lk_minus_1.begin(); it1 != Lk_minus_1.end(); ++it1) {
        for (auto it2 = next(it1); it2 != Lk_minus_1.end(); ++it2) {
            Itemset combined = it1->first;
            combined.insert(it2->first.begin(), it2->first.end());

            if (combined.size() == k) {
                // Pruning Step
                bool all_subsets_frequent = true;
                for (const auto& item : combined) {
                    Itemset subset = combined;
                    subset.erase(item);
                    if (Lk_minus_1.find(subset) == Lk_minus_1.end()) {
                        all_subsets_frequent = false;
                        break;
                    }
                }
                if (all_subsets_frequent) candidates.insert(combined);
            }
        }
    }
    return candidates;
}

int main() {
    // 1. Read Input
    int num_transactions;
    double min_support_percent, min_confidence_percent;
    
    cout << "Please enter the number of transactions desired: ";
    cin >> num_transactions;

    cout << "Please enter the minimum support percentage (e.g., 40): ";
    cin >> min_support_percent;

    cout << "Please enter the minimum confidence percentage (e.g., 60): ";
    cin >> min_confidence_percent;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "\nPlease enter each transaction on a new line:" << endl;
    Transactions transactions(num_transactions);
    for (int i = 0; i < num_transactions; ++i) {
        string line;
        getline(cin, line);
        stringstream ss(line);
        string item;
        while (ss >> item) {
            transactions[i].insert(item);
        }
    }

    // 2. Find All Frequent Itemsets
    int min_support_count = static_cast<int>(ceil((min_support_percent / 100.0) * num_transactions));
    map<int, SupportMap> all_frequent_itemsets;

    // First Pass (L1)
    SupportMap item_counts;
    for (const auto& tx : transactions) {
        for (const auto& item : tx) item_counts[{item}]++;
    }
    for (const auto& pair : item_counts) {
        if (pair.second >= min_support_count) all_frequent_itemsets[1][pair.first] = pair.second;
    }
    
    // Subsequent Passes (L2, L3, ...)
    for (int k = 2; !all_frequent_itemsets[k-1].empty(); ++k) {
        set<Itemset> candidates = generate_candidates(all_frequent_itemsets[k-1]);
        SupportMap Ck_counts;
        for (const auto& tx : transactions) {
            for (const auto& cand : candidates) {
                if (includes(tx.begin(), tx.end(), cand.begin(), cand.end())) {
                    Ck_counts[cand]++;
                }
            }
        }
        for (const auto& pair : Ck_counts) {
            if (pair.second >= min_support_count) all_frequent_itemsets[k][pair.first] = pair.second;
        }
    }

    // 3. Generate and Print Rules
    cout << "\n--- Association Rules ---" << endl;
    double min_confidence = min_confidence_percent / 100.0;
    for (const auto& level : all_frequent_itemsets) {
        if (level.first < 2) continue;
        for (const auto& pair : level.second) {
            const Itemset& itemset = pair.first;
            int itemset_support = pair.second;
            int n = itemset.size();

            // Generate subsets using a bitmask
            for (int i = 1; i < (1 << n) - 1; ++i) {
                Itemset antecedent;
                Itemset consequent;
                vector<string> v(itemset.begin(), itemset.end());
                for (int j = 0; j < n; ++j) {
                    if ((i >> j) & 1) antecedent.insert(v[j]);
                    else consequent.insert(v[j]);
                }
                
                int antecedent_support = all_frequent_itemsets.at(antecedent.size()).at(antecedent);
                double confidence = (double)itemset_support / antecedent_support;
                
                if (confidence >= min_confidence) {
                    print_itemset(antecedent);
                    cout << " -> ";
                    print_itemset(consequent);
                    cout << " [Sup: " << itemset_support << ", Conf: " << round(confidence * 100) << "%]" << endl;
                }
            }
        }
    }
    
    return 0;
}