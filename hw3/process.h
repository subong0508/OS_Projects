#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <string>
#include <queue>
#include <unordered_map>

#include "utils.h"

using namespace std;

// ADT of instruction
typedef struct _Inst {
    int opcode;
    int arg;
} Inst;

class Process {
    private:
        // process id
        int pID;
        // priority level
        int priority;
        // name
        string name = "";
        // file name
        string fileName = "";
        // blocked?
        bool blocked = false;
        
        /* Scheduling, Execution */
        // instructions
        queue<Inst *> insts;
        // Program Counter
        int pc = 0;
        // opcode, arg
        int opcode;
        int arg;
        // start time
        int startTime;
        // time left
        int timeLeft = TIME_QUANTUM;
        // wake up time
        int wakeUpTime = UNDEFINED;

        /* Memory, Paging */
        int vmemSize = 0;
        // virtual memory
        vector<int> vmem;
        // page count
        int pageCnt = 0;
        // active page
        PTE *nowPage;
        // page table: page id to page table entry
        unordered_map<int, PTE *> pageTable;
    public:
        Process(int pID, string dir, char *fileName, int priority, int startTime);

        /* Scheduling, Execution */
        // getter
        int getPID() const;
        int getPriority() const;
        string getName() const;
        bool getBlocked() const;
        int getPC() const;
        int getOpcode() const;
        int getArg() const;
        int getStartTime() const;
        int getTimeLeft() const;
        int getWakeUpTime() const;

        // setter
        void setTimeLeft(int timeLeft);
        void setWakeUpTime(int wakeUpTime);
        void setBlocked(bool blocked);

        // init
        void init(int vmemSize);
        // process is done?
        bool isDone();
        // do instruction
        void run();

        /* Memory, Paging */
        // allocate pages to virtual memory
        void vmemAlloc(int pageNum);
        // getter, setter
        PTE *getNowPage() const;
        void setNowPage(PTE *pte);
        // find page by page id
        PTE *findPage(int pageID);
        // release page by page id
        void memRelease(int pageID);
        // get all allocated pages
        vector<int> getPages();
        // refresh reference byte
        void refreshRefByte();
        // for memory.txt
        void printMemory(FILE *out, bool isLRU);
};

// comparator
struct compProcess {
    bool operator()(Process *p1, Process *p2) {
        return p1->getPID() < p2->getPID();
    }
};

#endif