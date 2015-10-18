#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#include "mtrand.h"
#include "wtrie.h"

class WeightedTrieWithDump : public WeightedTrie
{
public:
    WeightedTrieWithDump() : WeightedTrie() {};

    /** Do depth-first traversal of trie, dumping contents to stdout. */
    void Dump();
};

static void DumpRecursive(WeightedTrieNode* node, unsigned int level)
{
    unsigned int i;

    for (i = 0; i < level; i++) {
        std::cout << "  ";
    }
    if (node->IsLeafNode()) {
        std::cout << ((WeightedTrieLeafNode*)node)->key.GetHex();
        std::cout << ", w = " <<node->weight << std::endl;
    } else {
        std::cout << "xxxx, w = " << node->weight << std::endl;
        for (i = 0; i < WTRIE_CHILDREN; i++) {
            WeightedTrieNode* child = ((WeightedTrieNonLeafNode*)node)->child[i];
            if (child) {
                DumpRecursive(child, level + 1);
            }
        }
    }
}

void WeightedTrieWithDump::Dump()
{
    if (this->root) {
        DumpRecursive(this->root, 0);
    } else {
        std::cout << "tree is empty" << std::endl;
    }
}

static uint256& GenerateRandomTxID(MTRand_int32& mt)
{
    std::vector<unsigned char>& v = *(new std::vector<unsigned char>(32, 0));
    for (unsigned int i = 0; i < v.size(); i++) {
        // Don't use std::rand as its period is too short.
        v[i] = (unsigned char)mt();
    }
    return *(new uint256(v));
}

// http://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c
uint64_t GetTimeMs64()
{
#ifdef _WIN32
    /* Windows */
    FILETIME ft;
    LARGE_INTEGER li;

    /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
    * to a LARGE_INTEGER structure. */
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    uint64_t ret = li.QuadPart;
    ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
    ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

    return ret;
#else
    /* Linux */
    struct timeval tv;

    gettimeofday(&tv, NULL);

    uint64_t ret = tv.tv_usec;
    /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
    ret /= 1000;

    /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
    ret += (tv.tv_sec * 1000);

    return ret;
#endif
}

#define DEFAULT_NUM_TX  50000

static void usage(char* processName) {
    std::cout << "Usage:" << std::endl;
    std::cout << processName << std::endl;
    std::cout << "  ...this will run tests" << std::endl;
    std::cout << processName << " --benchmark n" << std::endl;
    std::cout << "  ...this will run benchmarks" << std::endl;
    std::cout << "  n is the number of items to use in benchmark (default " << DEFAULT_NUM_TX << ")" << std::endl;
}

static void runTests();
static void benchmark(unsigned int numberOfItems);

int main(int argc, char **argv)
{
    if (argc == 1) {
        // No benchmark, just test
        runTests();
    } else if (argc == 3) {
        if (std::string(argv[1]) == "--benchmark") {
            // Run benchmarks
            unsigned int numberOfItems = atoi(argv[2]);
            if (numberOfItems == 0) {
                numberOfItems = DEFAULT_NUM_TX;
            }
            benchmark(numberOfItems);
        } else {
            usage(argv[0]);
        }
    } else {
        usage(argv[0]);
    }

    exit(0);
}

static void runTests()
{
    WeightedTrieWithDump* t = new WeightedTrieWithDump();
    unsigned int i;
    const uint256* result = NULL;
    unsigned long seed[] = {0x42, 0x69, 0x69, 0x42};
    MTRand_int32 mt(seed, 4);
    bool failed = false;

    // Check that Remove/Contains/GetByCumulativeWeight don't choke on an
    // empty trie.
    if (t->Remove(GenerateRandomTxID(mt)) != false) {
        std::cout << "Unexpected return from Remove() on empty trie" << std::endl;
        failed = true;
    }
    if (t->Contains(GenerateRandomTxID(mt)) != false) {
        std::cout << "Unexpected return from Contains() on empty trie" << std::endl;
        failed = true;
    }
    if (t->GetByCumulativeWeight(0) != NULL) {
        std::cout << "Unexpected return from GetByCumulativeWeight() on empty trie" << std::endl;
        failed = true;
    }

    // Add one item and check Remove/Contains/GetByCumulativeWeight deal with it properly.
    uint256& testHash1 = GenerateRandomTxID(mt);
    if (t->Add(testHash1, 10) != true) {
        std::cout << "Couldn't add single item" << std::endl;
        failed = true;
    }
    if (t->Contains(testHash1) != true) {
        std::cout << "Contains() doesn't recognise single item" << std::endl;
        failed = true;
    }
    if (*(t->GetByCumulativeWeight(0)) != testHash1) {
        std::cout << "GetByCumulativeWeight() doesn't recognise single item" << std::endl;
        failed = true;
    }
    if (t->Remove(testHash1) != true) {
        std::cout << "Remove() doesn't recognise single item" << std::endl;
        failed = true;
    }

    // Try to add same key twice.
    t->Add(testHash1, 10);
    if (t->Add(testHash1, 10) != false) {
        std::cout << "Was able to add same key twice" << std::endl;
        failed = true;
    }

    // Add two keys, remove first and check that Contains() acts consistently
    uint256& testHash2 = GenerateRandomTxID(mt);
    t->Add(testHash1, 10);
    t->Add(testHash2, 10);
    if (!t->Contains(testHash1) || !t->Contains(testHash2)) {
        std::cout << "Contains() doesn't recognise both keys" << std::endl;
        failed = true;
    }
    t->Remove(testHash1);
    if (t->Contains(testHash1) || !t->Contains(testHash2)) {
        std::cout << "Contains() doesn't recognise when first key is removed" << std::endl;
        failed = true;
    }

    // TODO: Add more tests

    if (!failed) {
        std::cout << "All tests passed!" << std::endl;
    }
}

static void benchmark(unsigned int numberOfItems)
{
    WeightedTrieWithDump* t = new WeightedTrieWithDump();
    unsigned long seed[] = {0x42, 0x69, 0x69, 0x42};
    MTRand_int32 mt(seed, 4);
    unsigned int i;

    std::cout << "Benchmark size: " << numberOfItems << std::endl;

    std::cout << "Generating test data...";
    uint256** hashes = new uint256*[numberOfItems];
    for (i = 0; i < numberOfItems; i++) {
        hashes[i] = &(GenerateRandomTxID(mt));
    }
    std::cout << "done" << std::endl;

    // prefill with n items so that trie is never empty, otherwise
    // benchmarks will be optimistic
    for (i = 0; i < numberOfItems; i++) {
        if (!t->Add(GenerateRandomTxID(mt), 10)) {
            std::cout << i << " (prefill) couldn't be added" << std::endl;
        }
    }
    // add
    uint64_t startTime = GetTimeMs64();
    for (i = 0; i < numberOfItems; i++) {
        if (!t->Add(*(hashes[i]), 10)) {
            std::cout << i << " couldn't be added" << std::endl;
        }
    }
    uint64_t endTime = GetTimeMs64();
    std::cout << "add required " << ((double)(endTime - startTime) * 1000.0 / numberOfItems) << " us per op" << std::endl;
    // remove
    startTime = GetTimeMs64();
    for (i = 0; i < numberOfItems; i++) {
        t->Remove(*(hashes[i]));
    }
    endTime = GetTimeMs64();
    std::cout << "remove required " << ((double)(endTime - startTime) * 1000.0 / numberOfItems) << " us per op" << std::endl;
    // get
    startTime = GetTimeMs64();
    // Accumulate return value of GetByCumulativeWeight() so that compiler does
    // not optimise it away.
    intptr_t keyPtr = 0;
    for (i = 0; i < numberOfItems; i++) {
        keyPtr += (intptr_t)t->GetByCumulativeWeight(mt() % (10 * (numberOfItems + numberOfItems)));
    }
    // Display keyPtr so that compiler does not consider it to be unused
    std::cout << "Ignore this number: " << keyPtr << std::endl;
    endTime = GetTimeMs64();
    std::cout << "query required " << ((double)(endTime - startTime) * 1000.0 / numberOfItems) << " us per op" << std::endl;
}
