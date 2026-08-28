#ifndef TRIE_H
#define TRIE_H

#define TRIE_ALPHABET_SIZE 64

typedef struct TrieNode {
    struct TrieNode *children[TRIE_ALPHABET_SIZE];
    int isEndOfWord;
} TrieNode;

typedef struct {
    TrieNode *root;
} Trie;

Trie *trieCreate(void);
void trieFree(Trie *trie);
int trieInsert(Trie *trie, const char *word);
int trieSearch(const Trie *trie, const char *word);
int triePrefixSearch(const Trie *trie, const char *prefix, char results[][64],
                     int maxResults, int *resultCount);

#endif
