#include "os.h"

OS::OS() {
    /**
     * Init Run Queues
     */
    for(int i=0;i<10;i++) {
        runQueues.push_back(deque<Process *>());
    }
}

void OS::setDir(string dir) {
    /**
     * Set directory
     */
    if(dir[dir.size()-1] == '/') {
        this->dir = dir;
    } else {
        this->dir = dir + "/";
    }
}

void OS::setPage(string page) {
    /**
     * Set page replacement algorithm
     */
    this->page = page;
}

void OS::setOut() {
    /**
     * Initialize result files
     */
    schedOut = fopen((dir + "scheduler.txt").c_str(), "w");
    memOut = fopen((dir + "memory.txt").c_str(), "w");
}

void OS::init() {
    /**
     * Initialize processes to be executed, I/O jobs to be completed.
     */
    setOut();

    FILE *in = fopen((dir + "input").c_str(), "r");
    int totalEventNum, vmemSize, pmemSize, pageSize;
    fscanf(in, "%d %d %d %d\n", &totalEventNum, &vmemSize, &pmemSize, &pageSize);
    // managed based on page
    vmemSize /= pageSize;
    pmemSize /= pageSize;
    this->pmemSize = pmemSize;
    pmem.assign(pmemSize, UNDEFINED);
    // budy system
    root = new TreeNode();
    root->start = 0;
    root->end = pmemSize;
    root->size = pmemSize;

    int startTime, priority;
    char name[BUF_SIZE];
    for(int i=0;i<totalEventNum;i++) {
        fscanf(in, "%d %s %d", &startTime, name, &priority);
        // new process
        if(strcmp(name, "INPUT") != 0) {
            Process *process = new Process(processCnt, dir, name, priority, startTime);
            process->init(vmemSize);
            totalProcessList.push(process);
            processCnt++;
        // IO job
        } else {
            // IO job
            IO *io = new IO();
            io->time = startTime;
            io->pID = priority;
            IOCatchList.push(io);
        }
        fgetc(in);
    }

    fclose(in);
}

void OS::close() {
    /**
     * Close opened files.
     */
    fclose(schedOut);
    fprintf(memOut,"page fault = %d\n", pageFault);
    fclose(memOut);
}