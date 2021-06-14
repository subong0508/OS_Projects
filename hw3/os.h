#ifndef __OS_H__
#define __OS_H__

#include <stdio.h>
#include <cstring>
#include <list>
#include <deque>

#include "utils.h"
#include "process.h"

using namespace std;

class OS {
    private:
        // basic configs
        int cycle = 0;
        string dir = "./";
        string page = "lru";

        /* Scheduling */
        // number of processes
        int processCnt = 0;
        // contains process to be executed, sorted by start time
        queue<Process *> totalProcessList;
        // I/O queue, sorted by I/O interrupt time
        queue<IO *> IOCatchList;

        // now running process
        Process *nowProcess = NULL;
        // not terminated process list
        list<Process *> runningProcessList;
        // sleep list  
        list<Process *> sleepList;
        // I/O wait list
        list<Process *> IOWaitList;
        // run queues: Multi-level queue scheduling
        vector<deque<Process *>> runQueues;

        // scheduler.txt
        FILE *schedOut;
        // memory.txt
        FILE *memOut;

        /* Memory */
        // size of physical memory
        int pmemSize = 0;
        // physical memory
        vector<int> pmem;
        // frame count
        int frameCnt = 0;
        // buddy system
        TreeNode *root = NULL;
        // active pages
        list<PTE *> stack;
        // allocation id of clock pointer
        int clockAID = UNDEFINED;
        // page fault count
        int pageFault = 0;
    public:
        // constructor
        OS();

        // setter
        void setDir(string dir);
        void setPage(string page);
        // setter for results
        void setOut();

        // init processes
        void init();

        /* Scheduling */
        // processes all terminated
        bool allDone();
        // run cycle
        void runCycle();
        // check sleep process
        void checkSleepList();
        // check IO
        void checkIOWaitList();
        // create new process
        void createNewProcess();
        // schedule
        bool schedule();
        // execute instruction of the running process
        void exec();
        // delete process
        void deleteProcess();
        // for scheduler.txt
        void schedInfo(bool scheduled);

        /* Memory */
        // page=sampled
        void refreshRefByte();
        // memory access
        void memAccess(int pageID);
        // page replacement
        void memReplace();
        // store new page to the stack
        void storePage(PTE *page);
        // remove page from the tsack
        void removePage(int allocID);
        // memory release
        void memRelease(int pageID);
        // process termination
        void memReleaseAll();
        // for memory.txt
        void memInfo();
        // close file descriptors
        void close();
};

#endif