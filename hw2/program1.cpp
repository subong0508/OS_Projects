#include "programs.h"

int main(void) {
    // get input
    cin >> n;
    for(int i=0;i<n;i++) {
        int tmp;
        cin >> tmp;
        v.push_back(tmp);
        sorted.push_back(tmp);
    }
    
    // start performance measurement, clock(): ms
    clock_t start = clock();
    // merge sort
    merge_sort(0, n-1);
    // end of measurement
    clock_t end = clock();

    // print sorted data
    for(int i=0;i<n;i++) {
        cout << sorted[i] << ' ';
    }
    cout << '\n';
    // print the amount of time for merge sort 
    cout << (int)(end - start) << '\n';
    return 0;
}

