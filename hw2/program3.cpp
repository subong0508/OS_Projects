#include "programs.h"

int main(int argc, char *argv[]) {
    // get input
    total_thread_num = atoi(argv[1]);
    cin >> n;
    for(int i=0;i<n;i++) {
        int tmp;
        cin >> tmp;
        v.push_back(tmp);
        sorted.push_back(tmp);
    }

    // vector that stores the size of divided data before calling merge sort
    vector<int> prev;
    prev.push_back(n);
    // vector that stores the size of divided data after calling merge sort
    /**
     * ex) prev: {8}, now: {4, 4} => prev: {4, 4}, now: {2, 2, 2, 2}
     */
    vector<int> now(prev);
    while(true) {
        // if the number of data segments is greater than or equal to process number
        if(now.size() >= total_thread_num) {
            break;
        }
        // init
        now.clear();
        // indicate whether prev can be divided again
        bool possible = false;
        for(int i=0;i<prev.size();i++) {
            // you can't divide more
            if(prev[i]<=2){
                now.push_back(prev[i]);
            } else {
                // prev can be divided again
                possible = true;
                /* push the size of divided data to now vector
                   always divided into two parts */
                if(prev[i] % 2 == 0) {
                    now.push_back(prev[i] / 2);
                    now.push_back(prev[i] / 2);
                } else {
                    now.push_back(prev[i] / 2);
                    now.push_back(prev[i] / 2 + 1);
                }
            }
        }
        prev = now;
        // you can't divide more...
        if(!possible) {
            break;
        }
    }
    // fix total_thread_num to now.size()
    total_thread_num = now.size();

    /* start indices of divided data
     * ex) now: {2, 2, 2, 2} => offset: {0, 2, 4, 6} */
    vector<int> offset;
    int tmp = 0;
    for(int i=0;i<now.size();i++) {
        offset.push_back(tmp);
        tmp += now[i];
    }

    // array for containing threads
    pthread_t tids[total_thread_num];
    // init mutex to NULL
    pthread_mutex_init(&mutex_lock, NULL);
    // start performance measurement, clock(): ms
    clock_t start = clock();
    for(int i=0;i<total_thread_num;i++) {
        int start = offset[i];
        int end = start+now[i]-1;
        Args *args= new Args();
        args->start = start;
        args->end = end;
        pthread_create(&(tids[i]), NULL, merge_sort, args);
    }

    // wait all the threads to be done
    for(int i=0;i<total_thread_num;i++) {
        pthread_join(tids[i], NULL);
    }

    /* By using offset & now, prepare to combine partially sorted data.
       Pushing struct Index that has arguments of merge() function to queue. */
    queue<Index *> q;
    for(int i=0;i<total_thread_num-1;i+=2) {
        int start = offset[i];
        int end = offset[i+1]+now[i+1]-1;
        int mid = start+now[i]-1;
        Index *idx = new Index();
        idx->start = start;
        idx->mid = mid;
        idx->end = end;
        q.push(idx);
    }

    // merge until size of queue == 1, which means start=0, end=n-1
    while(!q.empty()) {
        Index *idxs1 = q.front();
        q.pop();
        merge(idxs1->start, idxs1->mid, idxs1->end, false);
        // merge complete: start=0, end=n-1
        if(q.empty()) {
            break;
        }
        Index *idxs2 = q.front();
        q.pop();
        merge(idxs2->start, idxs2->mid, idxs2->end, false);
        int start = idxs1->start;
        int end = idxs2->end;
        int mid = idxs1->end;
        // now push merged data to queue again => backward of merge sort
        Index *idx = new Index();
        idx->start = start;
        idx->mid = mid;
        idx->end = end;
        q.push(idx);
    }
    
    // end of measurement
    clock_t end = clock();
    for(int i=0;i<n;i++) {
        cout << sorted[i] << ' ';
    }
    cout << '\n';
    cout << (int)(end - start) << '\n';
    return 0;
}

void *merge_sort(void *args) {
    Args *myargs = (Args *) args;
    int start = myargs->start;
    int end = myargs->end;
    if(start < end) {
        int mid = (start + end) / 2;
        Args *newargs = new Args();
        // left merge sort
        newargs->start = start;
        newargs->end = mid;
        merge_sort(newargs);
        // right merge sort
        newargs->start = mid+1;
        newargs->end = end;
        merge_sort(newargs);
        /* merge left and right; set mutex = true since
           threads share global data */
        merge(start, mid, end, true);
    }

    return NULL;
}

