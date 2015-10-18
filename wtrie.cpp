#include <stdint.h>
#include "uint256.h"
#include "wtrie.h"

bool WeightedTrie::Add(const uint256& key, const uint64_t weight)
{
    if (weight == 0) {
        // Don't add nodes with 0 weight as they will never be selected and
        // thus are a waste of memory.
        return false;
    }
    if (this->Contains(key)) {
        // Item already exists, don't add it again.
        return false;
    }

    WeightedTrieNode** current = &(this->root);
    unsigned int level = 0;

    while (*current) {
        while ((*current)->IsLeafNode()) {
            // Can't add node below leaf node; push it down one level and
            // then try again.
            WeightedTrieLeafNode* leafNode = (WeightedTrieLeafNode*)(*current);
            WeightedTrieNonLeafNode* replacementNode = new WeightedTrieNonLeafNode(leafNode->weight);
            replacementNode->SelectChildFromKey(leafNode->key, level) = *current;
            *current = replacementNode;
        }
        // (*current) is now guaranteed to be a non-leaf node
        WeightedTrieNonLeafNode* nonLeafNode = (WeightedTrieNonLeafNode*)(*current);
        WeightedTrieNode*& nextNode = nonLeafNode->SelectChildFromKey(key, level);
        (*current)->weight += weight;
        current = &nextNode;
        level++;
    }
    WeightedTrieNode* newNode = new WeightedTrieLeafNode(key, weight);
    *current = newNode;
    return true;
}

bool WeightedTrie::Contains(const uint256& key)
{
    WeightedTrieNode** current = &(this->root);
    unsigned int level = 0;

    while (*current) {
        if ((*current)->IsLeafNode()) {
            WeightedTrieLeafNode* leafNode = (WeightedTrieLeafNode*)(*current);
            // Given a key, the path through the trie is deterministic.
            // Thus if we end up at a leaf node, this is the only place where
            // the search key could possibly be; there is no need to check
            // any other branches.
            return key == leafNode->key;
        } else {
            WeightedTrieNonLeafNode* nonLeafNode = (WeightedTrieNonLeafNode*)(*current);
            current = &(nonLeafNode->SelectChildFromKey(key, level));
            level++;
        }
    }
    return false;
}

const uint256* WeightedTrie::GetByCumulativeWeight(const uint64_t cumulativeWeight)
{
    if (this->root == NULL) {
        // Tree is empty
        return NULL;
    }
    if (this->root->weight <= cumulativeWeight) {
        // Cumulative weight is out of range
        return NULL;
    }

    WeightedTrieNode *current = this->root;
    uint64_t currentWeight = 0;

    while (!current->IsLeafNode()) {
        WeightedTrieNonLeafNode* nonLeafNode = (WeightedTrieNonLeafNode*)current;
        for (unsigned int i = 0; i < WTRIE_CHILDREN; i++) {
            if (nonLeafNode->child[i]) {
                uint64_t childWeight = nonLeafNode->child[i]->weight;
                if (cumulativeWeight < currentWeight + childWeight) {
                    // Correct weight range; follow this branch
                    current = nonLeafNode->child[i];
                    break;
                } else {
                    // Skip this branch
                    currentWeight += childWeight;
                }
            }
        }
    }
    WeightedTrieLeafNode* leafNode = (WeightedTrieLeafNode*)current;
    return &(leafNode->key);
}

static uint64_t RemoveRecursive(const uint256& key, const unsigned int level, WeightedTrieNode*& node)
{
    bool deleteNode = false;
    uint64_t deletedWeight = 0;

    if (node == NULL) {
        return 0; // not found
    }
    if (node->IsLeafNode()) {
        // Given a key, the path through the trie is deterministic.
        // Thus if we end up at a leaf node, this is the only place where
        // the key to remove could possibly be; there is no need to check
        // any other branches.
        WeightedTrieLeafNode* leafNode = (WeightedTrieLeafNode*)node;
        if (key == leafNode->key) {
            deleteNode = true;
            deletedWeight = node->weight;
        }
    } else {
        WeightedTrieNonLeafNode* nonLeafNode = (WeightedTrieNonLeafNode*)node;
        deletedWeight = RemoveRecursive(key, level + 1, nonLeafNode->SelectChildFromKey(key, level));
        node->weight -= deletedWeight;
        if (node->weight == 0) {
            // Non-leaf node is now empty; delete it
            deleteNode = true;
        }
    }
    if (deleteNode) {
        delete node;
        // Set node to NULL to signify to other functions that the child no
        // longer exists.
        node = NULL;
    }
    return deletedWeight;
}

bool WeightedTrie::Remove(const uint256& key)
{
    return RemoveRecursive(key, 0, this->root) != 0;
}

