#include "instruction.h"
#include "reservation_station.h"

void error_handler(int sig) {
    void *arr[10];
    size_t size = backtrace(arr, 10);
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(arr, size, STDERR_FILENO);
    exit(1);
}

string getCmdOption(char** begin, char** end, const string& option) {
    char** iter = find(begin, end, option);
    if (iter != end && ++iter != end) {
        return string(*iter);
    }
    return "";
}

bool cmdOptionExists(char** begin, char** end, const string& option) {
    return find(begin, end, option) != end;
}

/* Arguments:
 *  --input_file    : input file path,          [string]
 **/
int main(int argc, char** argv) {
    signal(SIGABRT, error_handler);

    string fName = getCmdOption(argv, argv+argc, "--input_file");
    ifstream inFile (fName);
    Reservation_Station rs(N_RS_ADD, N_FU_ADD, N_RS_MUL, N_FU_MUL, N_RS_LOAD, N_FU_LOAD);

    if (inFile.is_open()) {
        string line;
        while (getline(inFile, line)) {
            Instruction inst = Instruction::parse(line);
            int ret;
            do {
                ret = rs.inst_issue(inst);
                rs.run();
            } while (ret == -1);
        }
        while (!rs.is_finished()) rs.run();
        rs.print();
    } else 
        cerr << "failed to open file " << fName << endl;

    return 0;
}
