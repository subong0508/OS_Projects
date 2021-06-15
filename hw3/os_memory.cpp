#include "os.h"

void OS::refreshRefByte() {
    /**
     * Refresh reference bit and reference byte
     */
    if(page != "sampled") {
        return;
    }
    if(cycle % TIME_INTERVAL == 0) {
        list<Process *>::iterator iter;
        // for all processes(blocked or not)
        for(iter=runningProcessList.begin();iter!=runningProcessList.end();iter++) {
            (*iter)->refreshRefByte();
        }
    }
}

void OS::memAccess(int pageID) {
    /**
     * Handle memory access of page
     */
    PTE *pte = nowProcess->findPage(pageID);
    nowProcess->setNowPage(pte);
    // haven't been allocated yet
    if(pte->allocID == UNDEFINED) {
        pte->allocID = frameCnt++;
    }
    // need to allocate physical memory
    if(pte->validBit == 0) {
        pageFault++;
        TreeNode *node;
        bool allocated = false;
        while(!allocated) {
            node = pmemAlloc(root, pte->pageNum);
            if(node != NULL) {
                node = pmemSplit(node, pte->pageNum);
            }
            if(node != NULL) {
                node->allocID = pte->allocID;
                for(int i=node->start;i<node->end;i++) {
                    pmem[i] = pte->allocID;
                }
                allocated = true;
            } else {
                memReplace();
            }
        }
        // update valid bit
        pte->validBit = 1;
    }
    // update reference bit
    if(page != "lru") {
        pte->refBit = 1;
    }
    // store page
    storePage(pte);
}

void OS::memReplace() {
    /**
     * Evict physical memory, differs by page replace algorithm
     */
    // page to be removed
    PTE *pte = stack.front();
    list<PTE *>::iterator iter;
    if(page == "lru") {
        pte = stack.front();
    } else if(page == "sampled") {
        // find page with minimum reference byte and allocation id
        for(iter=stack.begin();iter!=stack.end();iter++) {
            if((*iter)->refByte.to_ulong() < pte->refByte.to_ulong()) {
                pte = *iter;
            } else if(((*iter)->refByte.to_ulong() == pte->refByte.to_ulong()) && ((*iter)->allocID < pte->allocID)) {
                pte = *iter;
            }
        }
        pte->refBit = 0;
    } else if(page == "clock") {
        // find start of the clock
        if(clockAID == UNDEFINED) {
            clockAID = stack.front()->allocID;
        } else {
            // find minimum allocation id which is bigger than or equal to current clockAID
            int newClockAID = UNDEFINED;
            for(iter=stack.begin();iter!=stack.end();iter++) {
                if((*iter)->allocID >= clockAID) {
                    newClockAID = (*iter)->allocID;
                    break;
                }
            }
            // if not found, clockAID is stack.front()->allocID
            if(newClockAID == UNDEFINED) {
                newClockAID = stack.front()->allocID;
            }
            clockAID = newClockAID;
        }
        // find clock pointer
        list<PTE *>::iterator clockPointer;
        for(iter=stack.begin();iter!=stack.end();iter++) {
            if((*iter)->allocID == clockAID) {
                clockPointer = iter;
                break;
            }
        }
        // iterate from clock pointer 2 times, choose page to be evicted
        bool found = false;
        for(iter=clockPointer;iter!=stack.end();iter++) {
            if((*iter)->refBit == 0) {
                pte = *iter;
                found = true;
                break;
            } else {
                (*iter)->refBit = 0;
            }
        }
        if(!found) {
            iter = stack.begin();
            while(!found) {
                if((*iter)->refBit == 0) {
                    pte = *iter;
                    found = true;
                } else {
                    (*iter)->refBit = 0;
                    iter++;
                }
            }
        }
        // change clockAID
        clockAID = pte->allocID + 1;  
    }
    // remove evicted page
    pte->validBit = 0;
    removePage(pte->allocID);
    // release physical memory
    TreeNode *node= pmemSearch(root, pte->allocID);
    pmemRelease(node, pmem);
}

void OS::storePage(PTE *pte) {
    /**
     * Store current page to stack
     */
    // remove if already in stack
    removePage(pte->allocID);
    list<PTE *>::iterator iter;
    if(page == "lru" || page == "sampled") {
        // move to the top
        stack.push_back(pte);
    } else if(page == "clock") {
        // sorted by allocation id
        bool found = false;
        for(iter=stack.begin();iter!=stack.end();iter++) {
            if((*iter)->allocID > pte->allocID) {
                stack.insert(iter, pte);
                found = true;
                break;
            }
        }
        if(!found) {
            stack.push_back(pte);
        }
    }
}

void OS::removePage(int allocID) {
    /** 
     * Remove from stack
     */
    list<PTE *>::iterator iter;
    for(iter=stack.begin();iter!=stack.end();iter++) {
        if((*iter)->allocID == allocID) {
            stack.erase(iter);
            break;
        }
    }
}

void OS::memRelease(int pageID) {
    /**
     * Release page from virtual/physical memory
     */
    PTE *pte = nowProcess->findPage(pageID);
    nowProcess->setNowPage(pte);
    // release virtual memory
    nowProcess->vmemRelease(pageID);
    if(pte->validBit == 1) {
        pte->validBit = 0;
        TreeNode *node= pmemSearch(root, pte->allocID);
        // release physical memory
        pmemRelease(node, pmem);
        // remove from stack
        removePage(pte->allocID);
    }
}

void OS::memReleaseAll() {
    /**
     * Release all pages of the process
     */
    // get page table entries sorted by allocation id
    vector<int> pageIDList = nowProcess->getPages();
    for(int i=0;i<pageIDList.size();i++) {
        memRelease(pageIDList[i]);
    }
}

void OS::memInfo() {
    /**
     * Print result for memory.txt
     */
    int opcode;
    if(nowProcess != NULL) {
        opcode = nowProcess->getOpcode();
    } else {
        // NO-OP
        opcode = UNDEFINED;
    }

    string func = "";
    switch(opcode) {
        case 0:
            func = "ALLOCATION";
            break;
        case 1:
            func = "ACCESS";
            break;
        case 2:
            func = "RELEASE";
            break;
        case 3:
            func = "NON-MEMORY";
            break;
        case 4:
            func = "SLEEP";
            break;
        case 5:
            func = "IOWAIT";
            break;
    }

    switch(opcode) {
        // no operation
        case UNDEFINED:
            fprintf(memOut, "[%d Cycle] Input: Function[NO-OP]\n", cycle);
            break;
        // mem alloc, access, release
        case 0:
        case 1:
        case 2:
            fprintf(memOut, "[%d Cycle] Input: Pid[%d] Function[%s] Page ID[%d] Page Num[%d]\n", cycle, nowProcess->getPID(), func.c_str(), 
                nowProcess->getNowPage()->pageID, nowProcess->getNowPage()->pageNum);
            break;
        // non-memory, sleep, io wait
        case 3:
        case 4:
        case 5:
            fprintf(memOut, "[%d Cycle] Input: Pid[%d] Function[%s]\n", cycle, nowProcess->getPID(), func.c_str());
            break;
    }

    // print physical memory
    fprintf(memOut, "%-30s", ">> Physical Memory: ");
    fprintf(memOut, "|");
    int cnt = 0;
    for(int i=0;i<pmemSize;i++) {
        if(pmem[i] == UNDEFINED) {
            fprintf(memOut, "-");
        } else {
            fprintf(memOut, "%d", pmem[i]);
        }
        cnt++;
        if(cnt == 4) {
            cnt = 0;
            fprintf(memOut, "|");
        }
    }
    fprintf(memOut, "\n");
  
    // sort processes by process id
    runningProcessList.sort(compProcess());
    list<Process *>::iterator iter;
    bool isLRU = (page == "lru");
    for(iter=runningProcessList.begin();iter!=runningProcessList.end();iter++) {
        (*iter)->printMemory(memOut, isLRU);
    }
    
    fprintf(memOut, "\n");
}