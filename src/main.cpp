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
 *  --output_path   : output file path,         [string]
 **/
int main(int argc, char** argv) {
    signal(SIGABRT, error_handler);
    chrono::high_resolution_clock::time_point t0 = chrono::high_resolution_clock::now();

    string fName = getCmdOption(argv, argv+argc, "--input_file");
    ifstream inFile (fName);
    Reservation_Station rs(N_RS_ADD, N_FU_ADD, N_RS_MUL, N_FU_MUL, N_RS_LOAD, N_FU_LOAD);

    rs.load(inFile);
    rs.run();

    if (cmdOptionExists(argv, argv+argc, "--output_path")) {
        string oName = getCmdOption(argv, argv+argc, "--output_path");
        ofstream outFile(oName);
        rs.output(outFile);
    }

    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time_span = t1 -t0;
    cout << "time elapsed: " << time_span.count() << " miliseconds" << endl;

    return 0;
}
