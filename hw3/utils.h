#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <algorithm>
#include <bitset>

#define UNDEFINED -1
#define TIME_QUANTUM 10
#define TIME_INTERVAL 8
#define BUF_SIZE 100
#define RR 5

using namespace std;

/* Scheduling */
typedef struct _IO {
    int time;
    int pID;
} IO;

/* Page Table Entry */
typedef struct _PTE {
    int pageID = UNDEFINED;
    int pageNum = 0;
    int pageAddr = UNDEFINED;
    int allocID = UNDEFINED;
    int validBit = 0;
    int refBit = 0;
    bitset<TIME_INTERVAL> refByte;
} PTE;

/* Buddy system */
typedef struct _TreeNode {
    _TreeNode *left = NULL;
    _TreeNode *right = NULL;
    _TreeNode *parent = NULL;
    int start = UNDEFINED;
    int end = UNDEFINED;
    int size = UNDEFINED;
    int allocID = UNDEFINED;
} TreeNode;

bool isAllocated(TreeNode *node);
TreeNode *pmemAlloc(TreeNode *node, int pageNum);
TreeNode *pmemSplit(TreeNode *node, int pageNum);
TreeNode *pmemSearch(TreeNode *node, int allocID);
void pmemRelease(TreeNode *node, vector<int>& pmem);
void pmemMerge(TreeNode *node);

// comparator, based on AID
struct compPTE {
    bool operator()(PTE *pte1, PTE *pte2) {
        return pte1->allocID < pte2->allocID;
    }
};

#endif