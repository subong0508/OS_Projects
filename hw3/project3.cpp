#include "os.h"

int main(int argc, char* argv[]) {
    /* Little OS */
    OS os;

    for(int i=0;i<argc;i++) {
        if(strncmp(argv[i], "-dir", 4) == 0) {
            string tmp(argv[i]);
            os.setDir(tmp.substr(5));
        } else if(strncmp(argv[i], "-page", 5) == 0) {
            string tmp(argv[i]);
            os.setPage(tmp.substr(6));
        }
    }
    // initialization
    os.init();

    do {
        os.runCycle();
    } while(!os.allDone());
    
    os.close();
    return 0;
}