#include "os.h"

bool OS::allDone() {
    /** 
     * Indicate whether all of the process are done
     */
    return totalProcessList.empty() && runningProcessList.empty();
}

void OS::checkSleepList() {
    /**
     * Check sleeping process and wake it up
     */
    list<Process *>::iterator iter = sleepList.begin();
    for(iter=sleepList.begin();iter!=sleepList.end();) {
        if((*iter)->getWakeUpTime() == cycle) {
            Process *process = *iter;
            runQueues[process->getPriority()].push_back(process);
            iter = sleepList.erase(iter);
        } else {
            iter++;
        }
    }
}

void OS::checkIOWaitList() {
    /**
     * Check I/O waiting process and wake it up
     */
    while(!IOCatchList.empty() && (IOCatchList.front()->time == cycle)){
        list<Process *>::iterator iter;
        for(iter=IOWaitList.begin();iter!=IOWaitList.end();iter++) {
            if((*iter)->getPID() == IOCatchList.front()->pID) {
                Process *process = *iter;
                runQueues[process->getPriority()].push_back(process);
                IOWaitList.erase(iter);
                break;
            }
        }
        IOCatchList.pop();
    }
}

void OS::createNewProcess() {
    /**
     * Create new process
     */
    while(!totalProcessList.empty() && (totalProcessList.front()->getStartTime() == cycle)) {
        Process *process = totalProcessList.front();
        totalProcessList.pop();
        runQueues[process->getPriority()].push_back(process);
        runningProcessList.push_back(process);
    }
}

bool OS::schedule() {
    /**
     * Multi-level Scheduling
     */
    for(int i=0;i<runQueues.size();i++) {
        if(!runQueues[i].empty()) {
            if(nowProcess == NULL) {
                nowProcess = runQueues[i].front();
                runQueues[i].pop_front();
                nowProcess->setBlocked(false);
                return true;
            } else if(nowProcess->getPriority() > i) {
                // block now process
                nowProcess->setTimeLeft(TIME_QUANTUM);
                runQueues[nowProcess->getPriority()].push_back(nowProcess);
                nowProcess->setBlocked(true);
                // get new process
                nowProcess = runQueues[i].front();
                runQueues[i].pop_front();
                nowProcess->setBlocked(false);
                return true;
            }
        }
    }
    return false;
}

void OS::exec() {
    /**
     * Execute the instruction of the running process
     */
    // no process to be executed
    if(nowProcess == NULL) {
        return;
    }
    int opcode, arg;
    nowProcess->run();
    opcode = nowProcess->getOpcode();
    arg = nowProcess->getArg();
    // last instruction?
    bool last =  nowProcess->isDone();
    // memory allocation
    if(opcode == 0) {
        nowProcess->vmemAlloc(arg);
    // memory access
    } else if(opcode == 1) {
        memAccess(arg);
    // memory release
    } else if(opcode == 2) {
        memRelease(arg);
    // non-memory instruction
    } else if(opcode == 3) {
        // NO-OP
    // sleep
    } else if(opcode == 4 && !last) {
        nowProcess->setWakeUpTime(cycle + arg);
        nowProcess->setTimeLeft(TIME_QUANTUM);
        nowProcess->setBlocked(true);
        sleepList.push_back(nowProcess);
    // IOWait
    } else if(opcode == 5 && !last) {
        nowProcess->setTimeLeft(TIME_QUANTUM);
        nowProcess->setBlocked(true);
        IOWaitList.push_back(nowProcess);
    }
    // time quantum all used, blocked
    if((!last) && (nowProcess->getTimeLeft() == 0) && (!nowProcess->getBlocked())) {
        nowProcess->setTimeLeft(TIME_QUANTUM);
        nowProcess->setBlocked(true);
        runQueues[nowProcess->getPriority()].push_back(nowProcess);
    }
}

void OS::runCycle() {
    /**
     * Run one CPU cycle
     */
    checkSleepList();
    checkIOWaitList();
    createNewProcess();
    bool scheduled = schedule();
    refreshRefByte();
    exec();
    schedInfo(scheduled);
    memInfo();
    // process termination
    if(nowProcess != NULL && nowProcess->isDone()) {
        memReleaseAll();
        deleteProcess();
        nowProcess = NULL;
    }
    // process is blocked
    if(nowProcess != NULL && nowProcess->getBlocked()) {
        nowProcess = NULL;
    }
    cycle += 1;
}


void OS::deleteProcess() {
    /**
     * Delete now process from running process list
     */
    list<Process *>::iterator iter;
    for(iter=runningProcessList.begin();iter!=runningProcessList.end();iter++) {
        if((*iter)->getPID() == nowProcess->getPID()) {
            runningProcessList.erase(iter);
            break;
        }
    }
}

void OS::schedInfo(bool scheduled) {
    /**
     * Print result for scheduler.txt
     */
    fprintf(schedOut, "[%d Cycle] Scheduled Process: ", cycle);
    if(scheduled) {
        fprintf(schedOut, "%d %s (priority %d)\n", nowProcess->getPID(), nowProcess->getName().c_str(), nowProcess->getPriority());
    } else {
        fprintf(schedOut, "None\n");
    }

    fprintf(schedOut, "Running Process: ");
    if(nowProcess != NULL) {
        fprintf(schedOut, "Process#%d(%d) running code %s line %d(op %d, arg %d)\n", nowProcess->getPID(), nowProcess->getPriority(), nowProcess->getName().c_str(),
            nowProcess->getPC(), nowProcess->getOpcode(), nowProcess->getArg());
    } else {
        fprintf(schedOut, "None\n");
    }

    for(int p=0;p<10;p++) {
        fprintf(schedOut, "RunQueue %d: ", p);
        if(runQueues[p].empty()) {
            fprintf(schedOut, "Empty") ;
        } else {
            for(int i=0;i<runQueues[p].size();i++) {
                Process *process = runQueues[p][i];
                fprintf(schedOut, "%d(%s) ", process->getPID(), process->getName().c_str());
            }
        }
        fprintf(schedOut, "\n");
    }
    
    list<Process *>::iterator iter;
    fprintf(schedOut, "SleepList: ");
    if(!sleepList.empty()) {
        for(iter=sleepList.begin();iter!=sleepList.end();iter++) {
            Process *process = *iter;
            fprintf(schedOut, "%d(%s) ", process->getPID(), process->getName().c_str());
        }
    } else {
        fprintf(schedOut, "Empty");
    }
    fprintf(schedOut, "\n");

    fprintf(schedOut, "IOWait List: ");
    if(!IOWaitList.empty()) {
        for(iter=IOWaitList.begin();iter!=IOWaitList.end();iter++) {
            Process *process = *iter;
            fprintf(schedOut, "%d(%s) ", process->getPID(), process->getName().c_str());
        } 
    } else {
        fprintf(schedOut, "Empty");
    }
    fprintf(schedOut, "\n");

    fprintf(schedOut, "\n");
}