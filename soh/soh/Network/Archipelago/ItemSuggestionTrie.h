#pragma once

#include "soh/Enhancements/randomizer/randomizerTypes.h"

#include <unordered_set>
#include <vector>
#include <string>
#include <string_view>
#include <exception>
#include <memory>
#include <optional>
#include <functional>

class ItemSuggestionTrie {
  public:
    class NodeNotFoundException : public std::exception {
      private:
        std::string message;

      public:
        NodeNotFoundException() : message("Could not find child node.") {
        }

        const char* what() const noexcept {
            return message.c_str();
        }
    };

    using OptionalSuggestions = std::optional<std::unordered_set<RandomizerGet>>;

    void AddItem(const RandomizerGet rg);
    const std::unordered_set<RandomizerGet> GetSuggestions(const std::string_view& searchString) const;

  private:
    struct TrieNode {
        const unsigned char c;
        std::unordered_set<RandomizerGet> leaf;
        std::vector<std::unique_ptr<TrieNode>> children;

        TrieNode() = delete;
        TrieNode(unsigned char letter) : c(letter) {};
    };

    std::unique_ptr<TrieNode> rootNode = std::make_unique<TrieNode>('_');

    void AddWord(const std::string& word, const RandomizerGet rg);
    TrieNode* FindOrCreateNode(const char letter, TrieNode* node);
    TrieNode* FindNode(const char letter, TrieNode* node) const;
    OptionalSuggestions GetWordSuggestion(const std::string& searchString) const;
    void GetAllSuggestions(TrieNode* node, std::unordered_set<RandomizerGet>& outSuggestions) const;
};