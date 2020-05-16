#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_
#include "macro.h"

class Instruction {
private:
    static int _id;
public:
    int line_num;
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
    Instruction(int line_num, int op, int dst, int src1, int src2,
                    int dst_t, int src1_t, int src2_t, string inst_string);
    void print() const;
    static bool compare_line_num(const Instruction& i, const Instruction& j);

    static Instruction parse(int line_num, string inst_string);
    static Instruction instantial(Instruction& inst);
};

#endif
