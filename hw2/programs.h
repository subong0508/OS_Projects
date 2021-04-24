#ifndef __PROGRAMS_H__
#define __PROGRAMS_H__

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <mutex>
#include <vector>
#include <queue>
#include <time.h>

using namespace std;

/* global vars for program1, program2 and program3 */
// number of data
int n = 0; 
// input data
vector<int> v; 
// sorted data
vector<int> sorted; 

/* program2 */
// number of processes
int total_process_num = 0; 
// input temporary file name
const string INPUT = "HW2_MYINPUT"; 
// output temporary file name
const string OUTPUT = "HW2_MYOUTPUT"; 
/* program2, program3: used for merging partially sorted array */
typedef struct _Index {
    int start;
    int mid;
    int end;
} Index;

/* program3 */
// number of threads
int total_thread_num = 0;
// mutex variable for synchronization
pthread_mutex_t mutex_lock;
// (void *)arg for pthread_create
typedef struct _Args {
    int start;
    int end;
} Args;
// function for pthread_create()
void *merge_sort(void *args);

void merge(int start, int mid, int end, bool mutex) {
    /**
     * Function input:
     *  - int start: start index of array
     *  - int mid: middle between start and end
     *  - int end: end index of array
     *  - bool mutex: flag to use mutex synchronization
     * 
     * Job:
     *  Merge two sorted array.
     *  ex) [2, 3, 4, 1, 3] => [1, 2, 3, 3, 4]
     *  where v[start..mid] = [2, 3, 4] and v[mid+1..end] = [1, 3]
     */
    // mutex: true => locking
    if(mutex) {
        pthread_mutex_lock(&mutex_lock);
    }
    
    int i = start;
    int j = mid+1;
    int k = start;
    while(i <= mid && j <= end) {
        // sorted in descending order
        if(v[i] >= v[j]) {
            sorted[k++] = v[i++];
        } else{
            sorted[k++] = v[j++];
        }
    }

    // putting leftover to sorted
    while(i <= mid) {
        sorted[k++] = v[i++];
    }
    while(j <= end) {
        sorted[k++] = v[j++];
    }
    // copy sorted to v
    for(int i=start;i<=end;i++) {
        v[i] = sorted[i];
    }

    // unlock mutex
    if(mutex) {
        pthread_mutex_unlock(&mutex_lock);
    }
}

void merge_sort(int start, int end) {
    /**
     * Function input:
     *  - int start: start index of array to be sorted
     *  - int end: end index of array to be sorted
     * 
     * Job:
     *  - Sort v[start..end] based on merge sort algorithm.
     */
    if(start < end) {
        int mid = (start + end) / 2;
        // left merge sort
        merge_sort(start, mid);
        // right merge sort
        merge_sort(mid+1, end);
        // merge left and right
        // by default, mutex is false(program1, program2)
        merge(start, mid, end, false);
    }
}

#endif
