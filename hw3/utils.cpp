#include "utils.h"

bool isAllocated(TreeNode *node) {
    /**
     * Return whether this node is allocated or not
     */
    return node->allocID != UNDEFINED;
}

TreeNode *pmemAlloc(TreeNode *node, int pageNum) {
    /**
     * Return the smallest available node such that has size being equal to or greater than pageNum
     * No memory split
     */
    TreeNode *res = NULL;
    bool isReafNode = (node->left == NULL) && (node->right == NULL);
    if(isReafNode) {
        if(!isAllocated(node) && (node->size/2 < pageNum) && (pageNum <= node->size)) {
            return node;
        } else if(!isAllocated(node) && pageNum <= node->size/2) {
            return node;
        }
    } else {
        TreeNode *resLeft = pmemAlloc(node->left, pageNum);
        TreeNode *resRight = pmemAlloc(node->right, pageNum);
        if(resLeft != NULL && resRight == NULL) {
            res = resLeft;
        } else if(resLeft == NULL && resRight != NULL) {
            res = resRight;
        // smaller node
        } else if(resLeft != NULL && resRight != NULL) {
            if(resLeft->size <= resRight->size) {
                res = resLeft;
            } else {
                res = resRight;
            }
        }
    }
    return res;
}

TreeNode *pmemSplit(TreeNode *node, int pageNum) {
    /**
     * Split node until fulfilled
     */
    TreeNode *res = NULL;
    bool isReafNode = (node->left == NULL) && (node->right == NULL);
    if(isReafNode) {
        if(!isAllocated(node) && (node->size/2 < pageNum) && (pageNum <= node->size)) {
            return node;
        } else if(!isAllocated(node) && pageNum <= node->size/2) {
            int newSize = node->size/2;
            node->left = new TreeNode();
            node->left->parent = node;
            node->left->start = node->start;
            node->left->end = node->start + newSize;
            node->left->size = newSize;

            node->right = new TreeNode();
            node->right->parent = node;
            node->right->start = node->left->end;
            node->right->end = node->end;
            node->right->size = newSize;

            // search available node in low address first
            res = pmemSplit(node->left, pageNum);
            // search available node in high address after
            if(res == NULL) {
                res = pmemSplit(node->right, pageNum);
            }
        }
    } else {
        // go to leaf node
        res = pmemSplit(node->left, pageNum);
        if(res == NULL) {
            res = pmemSplit(node->right, pageNum);
        }
    }
    return res;
}

TreeNode *pmemSearch(TreeNode *node, int allocID) {
    /**
     * Search physical memory by allocation id
     */
    TreeNode *res = NULL;
    bool isReafNode = (node->left == NULL) && (node->right == NULL);
    if(isReafNode) {
        if(node->allocID == allocID) {
            return node;
        }
    } else {
        // go to leaf node
        res = pmemSearch(node->left, allocID);
        if(res == NULL) {
            res = pmemSearch(node->right, allocID);
        }
    }
    return res;
}

void pmemRelease(TreeNode *node, vector<int>& pmem) {
    /**
     * Release physical memory
     */
    node->allocID = UNDEFINED;
    int start = node->start;
    int end = node->end;
    for(int i=start;i<end;i++) {
        pmem[i] = UNDEFINED;
    }
    pmemMerge(node);
}

void pmemMerge(TreeNode *node) {
    /**
     * Release physical memory, merge two nodes if needed
     */
    if(node->parent == NULL) {
        return;
    }

    TreeNode *other = NULL;
    if(node == node->parent->left) {
        other = node->parent->right;
    } else {
        other = node->parent->left;
    }
    // merge only if sibling node is leaf node
    bool isReafNode = (other->left == NULL) && (other->right == NULL);
    // merge two nodes
    if(!isAllocated(node) && !isAllocated(other) && isReafNode) {
        node->parent->left = node->parent->right = NULL;
        pmemMerge(node->parent);
    }
}