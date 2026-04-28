#include "ItemSuggestionTrie.h"
#include "soh/Enhancements/randomizer/static_data.h"

#include <iostream>
#include <algorithm>
#include <ranges>
#include <string>

// A custom implementation of a Trie (https://en.wikipedia.org/wiki/Trie)
// Used to quickly filter through a list of relevant's Archipelago Item Id for hinting
// We only implement Adding and Searching through the trie, we currently don't really need more
//  - Todo clearing the trie for new data (should be as simple as re-assigning the root node)

void ItemSuggestionTrie::AddItem(std::string ApItemName, const int64_t ApItemId) {
    std::transform(ApItemName.begin(), ApItemName.end(), ApItemName.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    ApItemName.erase(std::remove_if(ApItemName.begin(), ApItemName.end(), [](unsigned char c) { return !isalpha(c); }),
                     ApItemName.end());

    std::string_view view = ApItemName;
    while (!view.empty()) {
        AddWord(std::string(view), ApItemId);
        view = view.substr(1, view.size() - 1);
    }
    std::transform(ApItemName.begin(), ApItemName.end(), ApItemName.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

const std::unordered_set<int64_t> ItemSuggestionTrie::GetSuggestions(const std::string_view& searchString) const {
    std::unordered_set<int64_t> suggestions;

    if (searchString.empty()) {
        GetAllSuggestions(rootNode.get(), suggestions);
        return suggestions;
    }

    for (auto splitWord : std::views::split(searchString, ' ')) {
        // use substring constructor to hopefully make windows compiler happy
        std::string parsed = std::string(searchString, splitWord.begin() - searchString.begin(), splitWord.end() - splitWord.begin());
        std::transform(parsed.begin(), parsed.end(), parsed.begin(), [](unsigned char c) { return std::tolower(c); });
        parsed.erase(std::remove_if(parsed.begin(), parsed.end(), [](unsigned char c) { return !isalpha(c); }),
                     parsed.end());

        OptionalSuggestions new_suggestions = GetWordSuggestion(parsed);
        if (!new_suggestions.has_value()) {
            return {};
        }
        if (suggestions.empty()) {
            suggestions = new_suggestions.value();
        } else {
            std::unordered_set<int64_t> intersection;
            for (const int64_t apId : new_suggestions.value()) {
                if (suggestions.contains(apId)) {
                    intersection.insert(apId);
                }
            }
            if (!intersection.empty()) {
                suggestions.swap(intersection);
            } else {
                return {};
            }
        }
    }
    return suggestions;
}

void ItemSuggestionTrie::Clear() {
    rootNode = std::make_unique<TrieNode>('_');
}

void ItemSuggestionTrie::AddWord(const std::string& word, const int64_t ApItemId) {
    TrieNode* nextNode = rootNode.get();
    for (const unsigned char letter : word) {
        nextNode = FindOrCreateNode(letter, nextNode);
    }
    nextNode->leaf.insert(ApItemId);
}

ItemSuggestionTrie::TrieNode* ItemSuggestionTrie::FindOrCreateNode(const char letter, TrieNode* node) {
    for (auto&& findNode : node->children) {
        if (findNode->c == letter) {
            return findNode.get();
        }
    }
    // no existing node found
    node->children.emplace_back(std::make_unique<TrieNode>(TrieNode(letter)));
    return node->children.back().get();
}

ItemSuggestionTrie::TrieNode* ItemSuggestionTrie::FindNode(const char letter, TrieNode* node) const {
    for (auto&& findNode : node->children) {
        if (findNode->c == letter) {
            return findNode.get();
        }
    }
    throw NodeNotFoundException();
}

ItemSuggestionTrie::OptionalSuggestions ItemSuggestionTrie::GetWordSuggestion(const std::string& searchString) const {
    try {
        TrieNode* nextNode = rootNode.get();
        for (const unsigned char letter : searchString) {
            nextNode = FindNode(letter, nextNode);
        }
        std::unordered_set<int64_t> suggestions;
        GetAllSuggestions(nextNode, suggestions);
        return suggestions;
    } catch (const NodeNotFoundException& e) { return std::nullopt; }
    return std::nullopt;
}

void ItemSuggestionTrie::GetAllSuggestions(TrieNode* node, std::unordered_set<int64_t>& outSuggestions) const {
    for (const int64_t ApId : node->leaf) {
        outSuggestions.insert(ApId);
    }
    for (auto&& child : node->children) {
        GetAllSuggestions(child.get(), outSuggestions);
    }
}