#include "ItemSuggestionTrie.h"
#include "soh/Enhancements/randomizer/static_data.h"

#include <iostream>
#include <algorithm>
#include <ranges>
#include <string>

void ItemSuggestionTrie::AddItem(const RandomizerGet rg) {
    std::string ItemName = Rando::StaticData::GetItemTable()[rg].GetName().GetEnglish();
    std::transform(ItemName.begin(), ItemName.end(), ItemName.begin(), [](unsigned char c) { return std::tolower(c); });
    
    for (auto word : std::views::split(ItemName, ' ')) {
        std::string parsed = std::string(word.data());
        parsed.erase(std::remove_if(parsed.begin(), parsed.end(), [](unsigned char c) { return !isalpha(c); }),
                     parsed.end());
        AddWord(parsed, rg);
    }
}

const std::unordered_set<RandomizerGet> ItemSuggestionTrie::GetSuggestions(const std::string_view& searchString) const {
    std::unordered_set<RandomizerGet> suggestions;
    for (auto splitWord : std::views::split(searchString, ' ')) {
        std::string parsed = std::string(splitWord.begin(), splitWord.end() - splitWord.begin());
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
            std::unordered_set<RandomizerGet> intersection;
            for (const RandomizerGet rg : new_suggestions.value()) {
                if (suggestions.contains(rg)) {
                    intersection.insert(rg);
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

void ItemSuggestionTrie::AddWord(const std::string& word, const RandomizerGet rg) {
    TrieNode* nextNode = rootNode.get();
    for (const unsigned char letter : word) {
        nextNode = FindOrCreateNode(letter, nextNode);
    }
    nextNode->leaf.emplace(rg);
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
        std::unordered_set<RandomizerGet> suggestions;
        GetAllSuggestions(nextNode, suggestions);
        return suggestions;
    } catch (const NodeNotFoundException& e) { return std::nullopt; }
    return std::nullopt;
}

void ItemSuggestionTrie::GetAllSuggestions(TrieNode* node, std::unordered_set<RandomizerGet>& outSuggestions) const {
    for (const RandomizerGet rg : node->leaf) {
        outSuggestions.insert(rg);
    }
    for (auto&& child : node->children) {
        GetAllSuggestions(child.get(), outSuggestions);
    }
}