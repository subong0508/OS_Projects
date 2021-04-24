#include "miniShell.h"

using namespace std;

int main(void) {
    // user command
    char msg[MAX_LEN];
    Command command; 
    while(true) {
        greeting();
        // initalize msg, and get input from user
        memset(msg, '\0', ARGS_LEN);
        fgets(msg, MAX_LEN-1, stdin);
        msg[strlen(msg)-1] = '\0';
        strcpy(command.msg, msg);
        tokenize(msg, &command);
        // command is filled with white space only
        if(command.argc == 0) {
            continue;
        }
        // if user enters "exit", then return main()
        if(strncmp(command.argv[0], "exit", 4) == 0) { 
            break;
        }
        // background command
        bool bg = is_bg(&command);
        // if user enters 'cd', you have to handle it different way
        if(strncmp(msg, "cd", 2)==0) {
            cd_handler(&command, bg);
            continue;
        }
        int redirect = is_redirect(&command);
        // make new child process
        pid_t pid = fork();
        if(pid < 0) {
            // fork failed
            cout << "Fork failed.\n";
            continue;
        // if child process
        } else if(pid == 0) { 
            // use input/output redirection
            if(redirect != -1 ) { 
                redirect_handler(redirect, &command);
            // else, use execvp system call
            } else { 
                // not successful
                if(execvp(command.argv[0], command.argv) < 0) { 
                    perror(command.msg);
                    exit(1);
                }
            }
            // successfully done
            exit(0);
        // parent process
        } else {
            /* if background execution, parent child has to wait it's child
               else doesn't wait */ 
            if(!bg) {
                waitpid(pid, NULL, 0);
            } 
        }

        // free dynamically allocated memory used in Command struct
        for(int i=0;i<command.argc;i++) {
            free(command.argv[i]);
        }
    }

    return 0;
}

void greeting() {
    /**
     * Job:
     *  Prints greeting when miniShell is in execution.: [HH:MM:SS]'username'@'pwd'$
     * 
     * Reference: 
     *  http://www.cplusplus.com/reference/ctime/strftime/
     */
    // string to be printed
    string hello;
    // get current time and store it to now which is formatted as [HH:MM:SS]
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    string now = "";
    string hour = to_string(timeinfo->tm_hour);
    hour = hour.size() < 2 ? hour = "0" + hour : hour;
    now += "[" + hour + ":";
    string min = to_string(timeinfo->tm_min);
    min = min.size() < 2 ? min = "0" + min : min;
    now += min + ":";
    string sec = to_string(timeinfo->tm_sec);
    sec = sec.size() < 2 ? sec = "0" + sec : sec;
    now += sec + "]";

    // get user name
    string user = "";
    if(getenv("USER")!=NULL) {
        user.append(getenv("USER"));
    }
    // current working directory
    string pwd = "";
    char buf[255];
    if(getcwd(buf, 255)!=NULL) {
        pwd.append(buf);
    }
    hello = now + user + "@" + pwd + "$";
    // print greeting
    cout << hello;
}

void tokenize(char *msg, Command *command) {
    /**
     * Function input:
     *  - char *msg: user command
     *  - Command *command: parsed user command will be stored in it
     * 
     * Job:
     *  Tokenize user command and store it to command.
     */
    // tokenizing based on " "(white space) or ' or "
    stack<char> stack;
    int len = strlen(msg);
    string now = "";
    int argc = 0;
    for(int i=0;i<len;i++) {
        // white space
        if(msg[i] == ' ') {
            // ' or " exists
            if(!stack.empty()) {
                now += msg[i];
            // new arg
            } else if(now != "") { 
                command->argv[argc] = (char *)malloc(sizeof(char) * (strlen(now.c_str())+1));
                strcpy(command->argv[argc], now.c_str());
                argc++;
                // init now string
                now = "";
            }
        } else if(msg[i] == '\'' || msg[i] == '\"') {
            // end of token
            if(!stack.empty()) {
                // invalid input, ex) cd 'd1" => ignore command
                if(stack.top() != msg[i]) {
                    for(int i=0;i<argc;i++) {
                        free(command->argv[i]);
                        command->argv[i] = NULL;
                    }
                    command->argc = 0;
                    return;
                }
                stack.pop();
                command->argv[argc] = (char *)malloc(sizeof(char) * (strlen(now.c_str())+1));
                strcpy(command->argv[argc], now.c_str());
                argc++;
                // init now string
                now = "";
            // start of token
            } else {
                stack.push(msg[i]);
            }
        } else {
            now += msg[i];
        }
    }

    // valid input
    if(stack.empty()) {
        // last argument may exist ex) cd 'd1' d2
        if(now != "") {
            bool whitespace = true;
            for(int i=0;i<now.length();i++) {
                if(now[i] != ' ') {
                    whitespace = false;
                    break;
                }
            }
            // if not filled with whitespace, add the last argument
            if(!whitespace) {
                command->argv[argc] = (char *)malloc(sizeof(char) * (strlen(now.c_str())+1));
                strcpy(command->argv[argc], now.c_str());
                argc++;
            }
        }
        command->argc = argc;
        // the last argument is always NULL
        command->argv[argc] = NULL;
    // invalid input
    } else {
        for(int i=0;i<argc;i++) {
            free(command->argv[i]);
            command->argv[i] = NULL;
        }
        command->argc = 0;
    }
}

void cd_handler(Command *command, bool bg) {
    /**
     * Job: 
     *  Invoked when user enters cd command.
     */
    // if background process, you have to create a new process
    if(bg) {
        pid_t pid = fork();
        if(pid < 0) {
            cout << "Fork failed.\n";
        } else if(pid == 0) {
            execvp(command->argv[0], command->argv);
            exit(0);
        } 
        return;
    }

    // if user entered only "cd"
    if(command->argc == 1) {
        if(getenv("HOME")!=NULL) {
            chdir(getenv("HOME"));
        }
    } else if(command->argc == 2) {
        if (chdir(command->argv[1]) < 0) {
            cout << "cd: no such file or directory: " << command->argv[1] << '\n';
        }
    } else {
        cout << "cd: too many arguments" << '\n';
    }
}

bool is_bg(Command *command) {
    /**
     * Function input:
     *  - Command *command: command which will be used to determine 
     *    whether you have to create a background process.
     * 
     * Job:
     *  Check whether the new process has to be executed in background.
     *  It only checks the last argument you've entered equals to '&'.
     *  If it does, return true else false.
     */
    int argc = command->argc;
    // check the last argument
    if(strcmp(command->argv[argc-1], "&")==0) {
        // free & and reduces the number of arguments in command
        free(command->argv[argc-1]);
        command->argv[argc-1] = NULL;
        command->argc -= 1;
        return true;
    }
    return false;
}

int is_redirect(Command *command)
{
    /**
     * Function input:
     *  - Command *command: command which will be used to determine
     *    the process has to redirect stdin/stdout.
     * 
     * Job:
     *  Return the index of '>' or '<' if any, else return -1.
     */
    for(int i=0;i<command->argc;i++) {
        if(strcmp(command->argv[i], ">") == 0) {
            return i;
        } else if(strcmp(command->argv[i], "<") == 0) {
            return i;
        }
    }
    // stdin/stdout redirection not used
    return -1;
}

void redirect_handler(int redirect, Command *command) {
    /**
     * Function input:
     *  - int redirect: the index of '<' or '>'
     *  - Command *command: includes all the related information to execute a new process
     * 
     * Job:
     *  Handle user command when stdin/stdout redirection has to be implemented.
     */
    // id of file descriptor
    int fid;
    // character to indicate stdint/stdout redirection
    char ch = command->argv[redirect][0];
    // stdout redirection
    if(ch == '>') { 
        // open file after '>'
        fid = open(command->argv[redirect+1], O_WRONLY | O_TRUNC | O_CREAT, 
                   S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
        // exit if the file can't be opened
        if(fid < 0) {
            exit(1);
        }
        // replace stdout to the newly opened file
        dup2(fid, STDOUT_FILENO);
        // discard the args after '>'
        for(int i=redirect;i<command->argc;i++){
            free(command->argv[i]);
        }
        // reduce the number of args
        command->argc -= (command->argc - redirect);
        command->argv[command->argc] = NULL;
        // close unused file descriptor
        close(fid);
        // execute child process
        execvp(command->argv[0], command->argv);  
    // stdin redirection, same as above except it replaces stdin
    } else {
        fid = open(command->argv[redirect+1], O_RDONLY);
        if(fid < 0) {
            exit(1);
        }
        // replace stdin to the newly opened file
        dup2(fid, STDIN_FILENO);
        for(int i=redirect;i<command->argc;i++){
            free(command->argv[i]);
        }
        command->argc -= (command->argc - redirect);
        command->argv[command->argc] = NULL;
        close(fid); 
        execvp(command->argv[0], command->argv); 
    }
}

