#include "trie.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int trieCharIndex(char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a';
    }
    if (ch == ' ') {
        return 26;
    }
    if (ch >= '0' && ch <= '9') {
        return 27 + (ch - '0');
    }
    return -1;
}

Trie *trieCreate(void) {
    Trie *trie;
    TrieNode *root;
    int i;

    trie = (Trie *)malloc(sizeof(Trie));
    if (trie == NULL) {
        return NULL;
    }

    root = (TrieNode *)calloc(1, sizeof(TrieNode));
    if (root == NULL) {
        free(trie);
        return NULL;
    }

    for (i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        root->children[i] = NULL;
    }
    root->isEndOfWord = 0;

    trie->root = root;
    return trie;
}

static void trieFreeNode(TrieNode *node) {
    int i;

    if (node == NULL) {
        return;
    }

    for (i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            trieFreeNode(node->children[i]);
        }
    }
    free(node);
}

void trieFree(Trie *trie) {
    if (trie == NULL) {
        return;
    }
    trieFreeNode(trie->root);
    free(trie);
}

int trieInsert(Trie *trie, const char *word) {
    TrieNode *node;
    int i;
    int index;

    if (trie == NULL || word == NULL || word[0] == '\0') {
        return 0;
    }

    node = trie->root;
    for (i = 0; word[i] != '\0'; i++) {
        index = trieCharIndex(word[i]);
        if (index < 0) {
            return 0;
        }

        if (node->children[index] == NULL) {
            node->children[index] = (TrieNode *)calloc(1, sizeof(TrieNode));
            if (node->children[index] == NULL) {
                return 0;
            }
        }
        node = node->children[index];
    }

    node->isEndOfWord = 1;
    return 1;
}

int trieSearch(const Trie *trie, const char *word) {
    const TrieNode *node;
    int i;
    int index;

    if (trie == NULL || word == NULL) {
        return 0;
    }

    node = trie->root;
    for (i = 0; word[i] != '\0'; i++) {
        index = trieCharIndex(word[i]);
        if (index < 0 || node->children[index] == NULL) {
            return 0;
        }
        node = node->children[index];
    }

    return node->isEndOfWord;
}

static void trieCollectWords(const TrieNode *node, char *current, int depth,
                             char results[][64], int maxResults, int *count) {
    int i;

    if (node == NULL || *count >= maxResults) {
        return;
    }

    if (node->isEndOfWord) {
        current[depth] = '\0';
        strncpy(results[*count], current, 63);
        results[*count][63] = '\0';
        (*count)++;
    }

    for (i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i] != NULL) {
            current[depth] = (i >= 0 && i <= 25) ? (char)('a' + i)
                            : (i == 26) ? ' '
                            : (i >= 27 && i <= 36) ? (char)('0' + (i - 27))
                            : '?';
            trieCollectWords(node->children[i], current, depth + 1, results,
                             maxResults, count);
        }
    }
}

int triePrefixSearch(const Trie *trie, const char *prefix, char results[][64],
                     int maxResults, int *resultCount) {
    const TrieNode *node;
    char buffer[64];
    int i;
    int index;

    if (trie == NULL || prefix == NULL || results == NULL ||
        resultCount == NULL || maxResults <= 0) {
        return 0;
    }

    *resultCount = 0;
    node = trie->root;

    for (i = 0; prefix[i] != '\0'; i++) {
        index = trieCharIndex(prefix[i]);
        if (index < 0 || node->children[index] == NULL) {
            return 1;
        }
        buffer[i] = prefix[i];
        node = node->children[index];
    }

    buffer[i] = '\0';
    trieCollectWords(node, buffer, i, results, maxResults, resultCount);
    return 1;
}
