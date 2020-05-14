#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_
#include "macro.h"

class Instruction {
private:
    static int _id;
public:
    int id;
    int op;
    int dst;
    int src1;
    int src2;

    int dst_t;
    int src1_t;
    int src2_t;

    int issue_cycle;
    int exec_comp_cycle;
    int write_result_cycle;

    bool to_record;
    // bool first_access;

    int remaining_cycle;

    string inst_string;

    Instruction();
    Instruction(int op, int dst, int src1, int src2,
                    int dst_t, int src1_t, int src2_t, string inst_string);
    void print() const;
    static bool compare(const Instruction& i, const Instruction& j);

    static Instruction parse(string inst_string);
    // static Instruction cpy(Instruction& inst);
};

#endif
