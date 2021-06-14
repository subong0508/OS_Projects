#include "process.h"

Process::Process(int pID, string dir, char *name, int priority, int startTime) {
    this->pID = pID;
    this->name.append(name);
    this->fileName = dir;
    this->fileName.append(name);
    this->priority = priority;
    this->startTime = startTime;
}

/* Getter */
int Process::getPID() const {
    return pID;
}

int Process::getPriority() const {
    return priority;
}

string Process::getName() const {
    return name;
}

bool Process::getBlocked() const {
    return blocked;
}

int Process::getPC() const {
    return pc;
}

int Process::getOpcode() const {
    return opcode;
}

int Process::getArg() const {
    return arg;
}

int Process::getStartTime() const {
    return startTime;
}

int Process::getTimeLeft() const {
    return timeLeft;
}

int Process::getWakeUpTime() const {
    return wakeUpTime;
}

/* Setter */
void Process::setTimeLeft(int timeLeft) {
    this->timeLeft = timeLeft;
}

void Process::setWakeUpTime(int wakeUpTime) {
    this->wakeUpTime = wakeUpTime;
}

void Process::setBlocked(bool blocked) {
    this->blocked = blocked;
}

void Process::init(int vmemSize) {
    /**
     * Initialize process
     */
    this->vmemSize = vmemSize;
    vmem.assign(vmemSize, UNDEFINED);
    
    FILE *fp = fopen(fileName.c_str(), "r");
    int instCnt;
    fscanf(fp, "%d\n", &instCnt);

    int opcode;
    int arg;
    for(int i=0;i<instCnt;i++) {
        fscanf(fp, "%d %d", &opcode, &arg);
        Inst *inst = new Inst();
        inst->opcode = opcode;
        inst->arg = arg;
        insts.push(inst);
        fgetc(fp);
    }

    fclose(fp);
}

bool Process::isDone() {
    /**
     * Return whether all instructions are done
     */
    return insts.empty();
}

void Process::run() {
    /**
     * Run one CPU cycle
     */
    opcode = insts.front()->opcode;
    arg = insts.front()->arg;
    insts.pop();
    pc += 1;
    // only if RR scheduling
    if(priority >= RR) {
        timeLeft -= 1;
    }
}

void Process::vmemAlloc(int pageNum) {
    /**
     * Allocate required pages to virtual memory
     */
    PTE *pte = new PTE();
    pte->pageID = pageCnt;
    pte->pageNum = pageNum; 
    int i = 0;
    // search available virtual memory
    while(i <= vmemSize-pageNum) {
        bool found = true;
        for(int j=i;j<i+pageNum;j++) {
            if(vmem[j] != UNDEFINED) {
                found = false;
                i += findPage(vmem[j])->pageNum;
                break;
            }
        }
        if(found) {
            for(int j=i;j<i+pageNum;j++) {
                vmem[j] = pageCnt;
            }
            pte->pageAddr = i;
            break;
        }
    }
    // set this page to activate page
    setNowPage(pte);
    // insert to page table
    pageTable.insert(make_pair(pageCnt, pte));
    pageCnt++;
}

PTE *Process::getNowPage() const {
    return nowPage;
}

void Process::setNowPage(PTE *pte) {
    nowPage = pte;
}

PTE *Process::findPage(int pageID) {
    /**
     * Find page table entry by page id
     */
    return pageTable.find(pageID)->second;
}

void Process::memRelease(int pageID) {
    /**
     * Release virtual memory and erase from page table
     */
    int startAddr = findPage(pageID)->pageAddr;
    int pageNum = findPage(pageID)->pageNum;
    for(int i=startAddr;i<startAddr+pageNum;i++) {
        vmem[i] = UNDEFINED;
    }
    pageTable.erase(pageID);
}

vector<int> Process::getPages() {
    /**
     * Get all page id list of available pages sorted by allocation id
     */
    vector<PTE *> vecPTE;
    vector<int> vecPID;
    unordered_map<int, PTE *>::iterator iter;
    for(iter=pageTable.begin();iter!=pageTable.end();iter++) {
        vecPTE.push_back(iter->second);
    }
    // sort by allocation id
    sort(vecPTE.begin(), vecPTE.end(), compPTE());
    for(int i=0;i<vecPTE.size();i++) {
        vecPID.push_back(vecPTE[i]->pageID);
    }
    return vecPID;
}

void Process::refreshRefByte() {
    /**
     * Refresh reference byte
     */
    unordered_map<int, PTE *>::iterator iter;
    for(iter=pageTable.begin();iter!=pageTable.end();iter++) {
        PTE *pte = iter->second;
        pte->refByte >>= 1;
        pte->refByte[TIME_INTERVAL-1] = pte->refBit;
        pte->refBit = 0;
    }
}

void Process::printMemory(FILE *out, bool isLRU) {
    /**
     * Print status of page table to memory.txt
     */
    // page id
    fprintf(out, ">> pid(%d) %-20s", pID, "Page Table(PID): ");
    fprintf(out, "|");
    int cnt = 0;
    for(int i=0;i<vmemSize;i++) {
        if(vmem[i] == UNDEFINED) {
            fprintf(out, "-");
        } else {
            fprintf(out, "%d", vmem[i]);
        }
        cnt++;
        if(cnt == 4) {
            cnt = 0;
            fprintf(out, "|");
        }
    }
    fprintf(out, "\n");

    // allocation id
    fprintf(out, ">> pid(%d) %-20s", pID, "Page Table(AID): ");
    fprintf(out, "|");
    cnt = 0;
    for(int i=0;i<vmemSize;i++) {
        if(vmem[i] == UNDEFINED) {
            fprintf(out, "-");
        } else {
            int aid = findPage(vmem[i])->allocID;
            if(aid == UNDEFINED) {
                fprintf(out, "-");
            } else {
                fprintf(out, "%d", aid);
            }
        }
        cnt++;
        if(cnt == 4) {
            cnt = 0;
            fprintf(out, "|");
        }
    }
    fprintf(out, "\n");

    // valid bit
    fprintf(out, ">> pid(%d) %-20s", pID, "Page Table(Valid): ");
    fprintf(out, "|");
    cnt = 0;
    for(int i=0;i<vmemSize;i++) {
        if(vmem[i] == UNDEFINED) {
            fprintf(out, "-");
        } else {
            int valid = findPage(vmem[i])->validBit;
            fprintf(out, "%d", valid);
        }
        cnt++;
        if(cnt == 4) {
            cnt = 0;
            fprintf(out, "|");
        }
    }
    fprintf(out, "\n");

    // ref bit
    fprintf(out, ">> pid(%d) %-20s", pID, "Page Table(Ref): ");
    fprintf(out, "|");
    cnt = 0;
    for(int i=0;i<vmemSize;i++) {
        if(vmem[i] == UNDEFINED || isLRU) {
            fprintf(out, "-");
        } else {
            int ref = findPage(vmem[i])->refBit;
            fprintf(out, "%d", ref);
        }
        cnt++;
        if(cnt == 4) {
            cnt = 0;
            fprintf(out, "|");
        }
    }
    fprintf(out, "\n");
}