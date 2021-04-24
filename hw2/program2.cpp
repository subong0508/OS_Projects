#include "programs.h"

int main(int argc, char *argv[]) {
    // get input
    total_process_num = atoi(argv[1]);
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
        if(now.size() >= total_process_num) {
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
    // fix total_process_num to now.size()
    total_process_num = now.size();

    /* start indices of divided data
     * ex) now: {2, 2, 2, 2} => offset: {0, 2, 4, 6} */
    vector<int> offset;
    int tmp = 0;
    for(int i=0;i<now.size();i++) {
        offset.push_back(tmp);
        tmp += now[i];
    }

    // save temporary files that store the divided data segment
    for(int i=0;i<total_process_num;i++) {
        ofstream file;
        string file_name = INPUT + to_string(i);
        file.open(file_name.c_str());
        // number of data segment
        file << now[i] << '\n';
        // save data segment
        for(int j=0;j<now[i];j++) {
            file << v[offset[i]+j] << ' ';
        }
        file.close();
    }

    // array for containing child processes
    pid_t pids[total_process_num];
    // start performance measurement, clock(): ms
    clock_t start = clock();
    for(int i=0;i<total_process_num;i++) {
        pids[i] = fork();
        if(pids[i] < 0) {
            cout << "Fork failed.\n";
            exit(1);
        } else if(pids[i] == 0) {
            // temporary input file made just before
            string in_file = INPUT + to_string(i);
            // temporary output file to store sorted data segment
            string out_file = OUTPUT + to_string(i);
            int in, out;
            in = open(in_file.c_str(), O_RDONLY);
            out = open(out_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 
                       S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
            // replace stdin, stdout
            dup2(in, STDIN_FILENO);
            dup2(out, STDOUT_FILENO);
            // close unused files
            close(in);
            close(out);
            // remove temporary input file
            remove(in_file.c_str());
            char *args[] = {(char *) "./program1", NULL};
            // execute program1
            execvp("./program1", args);
            exit(0);
        } 
    }

    // wait all the child processes to be done
    for(int i=0;i<total_process_num;i++) {
        waitpid(pids[i], NULL, 0);
    }

    // take partially sorted data
    int idx = 0;
    for(int i=0;i<total_process_num;i++) {
        // temporary output file that stores sorted data segment
        string out_file = OUTPUT + to_string(i);
        ifstream output;
        output.open(out_file);
        for(int j=0;j<now[i];j++) {
            int tmp;
            output >> tmp;
            v[idx] = tmp;
            sorted[idx] = tmp;
            idx++;
        }
        output.close();
        // remove temporary output file
        remove(out_file.c_str());
    }

    /* By using offset & now, prepare to combine partially sorted data.
       Pushing struct Index that has arguments of merge() function to queue. */
    queue<Index *> q;
    for(int i=0;i<total_process_num-1;i+=2) {
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

