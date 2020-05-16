#include "instruction.h"

int Instruction::_id = 0;

Instruction::Instruction() {
    this->id = -1;
}

Instruction::Instruction(int line_num, int op, int dst, int src1, int src2,
                int dst_t, int src1_t, int src2_t, string inst_string) {
    this->to_record = true;
    // this->first_access = true;
    this->line_num = line_num;
    // this->id = _id++;
    this->id = -1;
    this->op = op;
    this->dst = dst;
    this->src1 = src1;
    this->src2 = src2;
    this->dst_t = dst_t;
    this->src1_t = src1_t;
    this->src2_t = src2_t;
    this->inst_string = inst_string;
    this->issue_cycle = UNDEFINED;
    this->exec_comp_cycle = UNDEFINED;
    this->write_result_cycle = UNDEFINED;
    this->remaining_cycle = (op == INST_LD ? TIME_LD :
                            (op == INST_ADD ? TIME_ADD : 
                            (op == INST_SUB ? TIME_SUB : 
                            (op == INST_MUL ? TIME_MUL : 
                            (op == INST_DIV ? TIME_DIV : TIME_JUMP)))));
}

void Instruction::print() const {
    cout << inst_string << endl;
    cout << "\tissue: " <<  issue_cycle 
        << ", exec_comp: " <<  exec_comp_cycle 
        << ", write_result: " << write_result_cycle << endl;
}

bool Instruction::compare_line_num(const Instruction& i, const Instruction& j) {
    return i.line_num < j.line_num;
}

Instruction Instruction::parse(int line_num, string inst_string) {
    int op, dst, src1, src2, dst_t, src1_t, src2_t;
    op = dst = src1 = src2 = dst_t = src1_t = src2_t = UNDEFINED;

    vector<string> items;
    regex comma_re(",(?!\\s)", regex_constants::ECMAScript | regex_constants::icase);
    copy(sregex_token_iterator(inst_string.begin(), inst_string.end(), comma_re, -1),
        sregex_token_iterator(), back_inserter(items));

    if (items[0].compare("LD") == 0) {
        op = INST_LD;
    } else if (items[0].compare("ADD") == 0) {
        op = INST_ADD;
    } else if (items[0].compare("MUL") == 0) {
        op = INST_MUL;
    } else if (items[0].compare("DIV") == 0) {
        op = INST_DIV;
    } else if (items[0].compare("SUB") == 0) {
        op = INST_SUB;
    } else if (items[0].compare("JUMP") == 0) {
        op = INST_JUMP;
    }

    switch (op) {
        case INST_LD:
            assert(items.size() == 3);

            assert(items[1][0] == 'R');
            dst_t = INST_REG;
            stringstream(items[1].substr(1)) >> dst;

            assert(items[2][1] == 'x');
            src1_t = INST_INT;
            uint32_t tmp;
            stringstream(items[2]) >> hex >> tmp;
            src1 = COMPLE_INT(tmp);

            break;
        case INST_ADD:
        case INST_MUL:
        case INST_SUB:
        case INST_DIV:
        case INST_JUMP:
            assert(items.size() == 4);
            
            for (int i = 1; i < 4; i++) {
                int& type = (i == 1 ? dst_t : (i == 2 ? src1_t : src2_t));
                int& target = (i == 1 ? dst : (i == 2 ? src1 : src2));

                if (items[i][0] == 'R') {
                    type = INST_REG;
                    stringstream(items[i].substr(1)) >> target;
                } else {
                    assert(items[i][1] == 'x');
                    type = INST_INT;
                    uint32_t tmp;
                    stringstream(items[i]) >> hex >> tmp;
                    target = COMPLE_INT(tmp);
                }
            }

            break;
    }

#if (DEBUG)
    cout << "op: " << op 
        << " dst: " << dst
        << " src1: " << src1
        << " src2: " << src2
        << " dst_t: " << dst_t
        << " src1_t: " << src1_t
        << " src2_t: " << src2_t << endl;
#endif

    return Instruction(line_num, op, dst, src1, src2, dst_t, src1_t, src2_t, inst_string);
}

Instruction Instruction::instantial(Instruction& inst) {
    Instruction ret;
    ret = inst;
    ret.id = _id ++;
    return ret;
}
