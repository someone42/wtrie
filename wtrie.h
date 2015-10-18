#ifndef WTRIE_H
#define WTRIE_H

#include <stdint.h>
#include "uint256.h"

/** log2(log2(children)).
 *  0 => 2 children per node, 1 => 4 children, 2 => 16 children, 3 => 256 children.
 *  2 is a good middle ground for this. Any lower and leaf nodes are too deep.
 *  Any higher and non-leaf nodes take up too much memory.
 */
#define WTRIE_LOGLOG_CHILDREN   2

#define WTRIE_CHILDREN          (1 << (1 << WTRIE_LOGLOG_CHILDREN))

class WeightedTrieNode
{
private:
    const bool is_leaf;
public:
    uint64_t weight;

    WeightedTrieNode(const bool is_leaf, const uint64_t weight) : is_leaf(is_leaf), weight(weight) {}
    bool IsLeafNode() { return is_leaf; }
};

/** In a balanced tree, most nodes are leaf nodes, so we want to
 *  avoid having to store child node pointers for those nodes. Thus to save
 *  memory, node types are explicitly split into mutually exclusive leaf nodes
 *  and non-leaf nodes. 
 */
class WeightedTrieLeafNode : public WeightedTrieNode
{
public:
    const uint256 key;

    WeightedTrieLeafNode(const uint256& key, const uint64_t weight) : WeightedTrieNode(true, weight), key(key) {}
};

/** The weight of a non-leaf node is the weight of all leaf nodes below it. */
class WeightedTrieNonLeafNode : public WeightedTrieNode
{
public:
    WeightedTrieNode *child[WTRIE_CHILDREN];

    WeightedTrieNonLeafNode(const uint64_t weight) : WeightedTrieNode(false, weight)
    {
        memset(this->child, 0, sizeof(this->child));
    }
    /** Find out how to navigate to next level in trie, based on the specified
     *  key.
     */
    WeightedTrieNode*& SelectChildFromKey(const uint256& key, const unsigned int level)
    {
        const unsigned int index = level >> (3 - WTRIE_LOGLOG_CHILDREN);
        const unsigned int shift = (level & (7 >> WTRIE_LOGLOG_CHILDREN)) << WTRIE_LOGLOG_CHILDREN;
        const unsigned int mask = WTRIE_CHILDREN - 1;
        // index is modulo key.size() so that it will never go out of bounds.
        // It's a bit of a hack, but index should never >= key.size() anyway.
        // The modulo code just prevents a bad situation from getting worse.
        const unsigned int b = ((key.begin()[index % key.size()]) >> shift) & mask;
        return this->child[b];
    }
};

/** Data structure that looks like an ordered set of uint256 keys, where each
 *  item also has an attached integer weight. Members of the set can be queried
 *  by cumulative weight - imagine that each item occupied a space of the size
 *  equal to its weight, and that all items were placed contiguously, in order,
 *  in a large byte array. Cumulative weight refers to the position within that
 *  fictitious large byte array.
 */
class WeightedTrie
{
protected:
    WeightedTrieNode* root;

public:
    WeightedTrie() : root(NULL) {};

    /** Add item into set. key must be unique and weight must be non-zero.
     *  @return true if and only if the new item was added.
     */
    bool Add(const uint256& key, const uint64_t weight);
    /** Check set membership of key.
     *  @return true if and only if the set contains the specified key.
     */
    bool Contains(const uint256& key);
    /** Find out which item occupies a specific cumulative weight. For example,
     *  say there are three items in the following order:
     *  - Item A, with weight 12
     *  - Item B, with weight 100
     *  - Item C, with weight 8
     *  Calling this with cumulative weights of between 0 and 11 (inclusive)
     *  would select item A. Likewise, cumulative weights of between 12 and
     *  111 would select item B, and cumulative weights of between 112 and
     *  119 would select item C.
     *  @return NULL if specified cumulative weight was out of bounds,
     *          otherwise this returns the key of the item at the specified
     *          cumulative weight.
     */
    const uint256* GetByCumulativeWeight(const uint64_t cumulativeWeight);
    /** Remove item from set. An item with the specified key must exist
     *  within the set.
     *  @return true if and only if the specified item was removed.
     */
    bool Remove(const uint256& key);
};

#endif // WTRIE_H
