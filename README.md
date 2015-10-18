wtrie - Weighted Trie library
===================
This is an implementation of an ordered set where each item has a 256 bit key (which determines the ordering), and an integer weight. Items can be added, removed, and queried by "cumulative weight". All operations are O(log N). <i>wtrie</i> is designed to be used to help select a random byte within a Bitcoin node's memory pool. 

Here is an example. Say the weighted trie contains three items, in the following order:
- Item A, with weight 12
- Item B, with weight 100
- Item C, with weight 8
Querying the trie with cumulative weights of between 0 and 11 (inclusive) would select item A. Likewise, cumulative weights of between 12 and 111 would select item B, and cumulative weights of between 112 and 119 would select item C.

It's as if all the items occupied a space equal to their weight in bytes, and all those items were placed contiguously, in order, in a large byte array. Cumulative weight refers to the position within that fictitious large byte array.

----------


Using
-------------
Include wtrie.cpp and wtrie.h in your source/header file list. There is a dependency on uint256 from Bitcoin Core/XT.

If you're not using uint256 from the Bitcoin Core/XT source, you may also need to include tinyformat.h, uint256.h, utilstrencodings.h, uint256.cpp, and utilstrencodings.cpp.

1. Use <code>new WeightedTrie()</code> to create an empty weighted trie object.
2. Use the <code>Add(key, weight)</code> method to add items to the weighted trie object. key is a uint256 (usually the Bitcoin transaction ID i.e. hash) and weight is the weight of the item (usually the size of the Bitcoin transaction).
3. Use the <code>Remove(key)</code> method to remove items from the weighted trie object.
4. Use the <code>GetByCumulativeWeight(cumulativeWeight)</code> method to query by cumulative weight. This method will return a pointer to an appropriate uint256 key, or NULL on error.

Testing
-------------
Using the usual GNU build tools, run "make" in the directory with a Makefile in it. This will generate an executable which tests the library. You can run this executable with the <code>--benchmark n</code> option to run a benchmark, where n is the number of items to add/remove/query.
