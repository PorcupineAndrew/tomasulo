#include "reservation_station.h"

RS_Entry::RS_Entry() {
    this->flush();
}

void RS_Entry::flush() {
    Busy = Ready = Running = false;
    Vj = Vk = Qj = Qk = Address = UNDEFINED;
}

bool RS_Entry::compare(RS_Entry *i, RS_Entry *j) {
    return i->inst.idx < j->inst.idx;
}

Reservation_Station::Reservation_Station(int n_rs_add,
    int n_fu_add, int n_rs_mult, int n_fu_mult, int n_rs_load, int n_fu_load) : 
    max_RS_Add(n_rs_add), max_RS_Mult(n_rs_mult), max_RS_Load(n_rs_load) {

    this->avail_FU_Add = n_fu_load;
    this->RS_Add = new RS_Entry[max_RS_Add];

    this->avail_FU_Mult = n_fu_mult;
    this->RS_Mult = new RS_Entry[max_RS_Mult];

    this->avail_FU_Load = n_fu_load;
    this->RS_Load = new RS_Entry[max_RS_Load];

    this->cycle = 1;
    for (int i = 0; i < 32; i++) {
        register_result_status[i] = UNDEFINED;
    }
}

Reservation_Station::~Reservation_Station() {
    delete [] this->RS_Add;
    delete [] this->RS_Mult;
    delete [] this->RS_Load;
}

int Reservation_Station::RS_Add_avail() const {
    int i;
    for (i = 0; i < max_RS_Add; i++) {
        if (!RS_Add[i].Busy) return i;
    }
    return -1;
}

int Reservation_Station::RS_Mult_avail() const {
    int i;
    for (i = 0; i < max_RS_Mult; i++) {
        if (!RS_Mult[i].Busy) return i;
    }
    return -1;
}

int Reservation_Station::RS_Load_avail() const {
    int i;
    for (i = 0; i < max_RS_Load; i++) {
        if (!RS_Load[i].Busy) return i;
    }
    return -1;
}

bool Reservation_Station::is_finished() const {
    for (int k = 0; k < 3; k++) {
        const int& max_RS = (k == 0 ? max_RS_Load : (k == 1 ? max_RS_Add : max_RS_Mult));
        RS_Entry *RS = (k == 0 ? RS_Load : (k == 1 ? RS_Add : RS_Mult));
        for (int i = 0; i < max_RS; i++) {
            if (RS[i].Busy) return false;
        }
    }
    return true;
}

int Reservation_Station::inst_issue(Instruction inst) {
    int idx;
    switch(inst.op) {
        case INST_LD:
            if ((idx = RS_Load_avail()) != -1) {
                RS_Load[idx].Busy = true;
                register_result_status[inst.dst] = inst.idx;
                inst.issue_cycle = cycle;
                RS_Load[idx].inst = inst;
            }
            break;
        case INST_ADD:
        case INST_SUB:
            if ((idx = RS_Add_avail()) != -1) {
                RS_Add[idx].Busy = true;
                if (inst.src1_t == INST_REG) {
                    RS_Add[idx].Qj = register_result_status[inst.src1];
                }
                if (inst.src2_t == INST_REG) {
                    RS_Add[idx].Qk = register_result_status[inst.src2];
                }
                register_result_status[inst.dst] = inst.idx;
                inst.issue_cycle = cycle;
                RS_Add[idx].inst = inst;
            }
            break;
        case INST_MUL:
        case INST_DIV:
            if ((idx = RS_Mult_avail()) != -1) {
                RS_Mult[idx].Busy = true;
                if (inst.src1_t == INST_REG) {
                    RS_Mult[idx].Qj = register_result_status[inst.src1];
                }
                if (inst.src2_t == INST_REG) {
                    RS_Mult[idx].Qk = register_result_status[inst.src2];
                }
                register_result_status[inst.dst] = inst.idx;
                inst.issue_cycle = cycle;
                RS_Mult[idx].inst = inst;
            }
            break;
        case INST_JUMP:
            // TODO: ??
            idx = -1;
            break;
        default:
            idx = -1;
            break;
    }
    return idx;
}


void Reservation_Station::update(int idx) {
    for (int i = 0; i < max_RS_Add; i++) {
        if (RS_Add[i].Busy && !RS_Add[i].Ready) {
            if (RS_Add[i].Qj == idx) {
                RS_Add[i].Qj = UNDEFINED;
                // TODO: ??
            }
            if (RS_Add[i].Qk == idx) {
                RS_Add[i].Qk = UNDEFINED;
                // TODO: ??
            }
        }
    }
    for (int i = 0; i < max_RS_Mult; i++) {
        if (RS_Mult[i].Busy && !RS_Mult[i].Ready) {
            if (RS_Mult[i].Qj == idx) {
                RS_Mult[i].Qj = UNDEFINED;
                // TODO: ??
            }
            if (RS_Mult[i].Qk == idx) {
                RS_Mult[i].Qk = UNDEFINED;
                // TODO: ??
            }
        }
    }
}

void Reservation_Station::run() {
    // write result
    for (int k = 0; k < 3; k++) {
        const int& max_RS = (k == 0 ? max_RS_Load : (k == 1 ? max_RS_Add : max_RS_Mult));
        int& avail_FU = (k == 0 ? avail_FU_Load : (k == 1 ? avail_FU_Add : avail_FU_Mult));
        RS_Entry *RS = (k == 0 ? RS_Load : (k == 1 ? RS_Add : RS_Mult));

        for (int i = 0; i < max_RS; i++) {
            if (RS[i].Running && RS[i].inst.remaining_cycle == 0) { 
                RS[i].inst.write_result_cycle = cycle;
                assert(RS[i].inst.write_result_cycle == RS[i].inst.exec_comp_cycle + 1);
                finished.push_back(RS[i].inst);
                update(RS[i].inst.idx);
                if (register_result_status[RS[i].inst.dst] == RS[i].inst.idx) {
                    register_result_status[RS[i].inst.dst] = UNDEFINED;
                }
                avail_FU ++;
                RS[i].flush();
            }
        }
    }
    
    // run for ready
    for (int k = 0; k < 3; k++) {
        int& avail_FU = (k == 0 ? avail_FU_Load : (k == 1 ? avail_FU_Add : avail_FU_Mult));
        vector<RS_Entry *>& ready_RS = (k == 0 ? ready_RS_Load : (k == 1 ? ready_RS_Add : ready_RS_Mult));
        
        while (avail_FU > 0 && !ready_RS.empty()) {
            ready_RS[0]->Running = true;
            ready_RS.erase(ready_RS.begin());
            avail_FU --;
        }
    }

    // check ready
    vector<RS_Entry *> new_ready_list;
    for (int k = 0; k < 3; k++) {
        new_ready_list.clear();
        const int& max_RS = (k == 0 ? max_RS_Load : (k == 1 ? max_RS_Add : max_RS_Mult));
        RS_Entry *RS = (k == 0 ? RS_Load : (k == 1 ? RS_Add : RS_Mult));
        vector<RS_Entry *>& ready_RS = (k == 0 ? ready_RS_Load : (k == 1 ? ready_RS_Add : ready_RS_Mult));

        for (int i = 0; i < max_RS; i++) {
            if (RS[i].Busy) {
                if (RS[i].Running) {
                    if ((-- RS[i].inst.remaining_cycle) == 0) {
                        RS[i].inst.exec_comp_cycle = cycle;
                    }
                } else if (!RS[i].Ready) {
                    if (RS[i].Qj == UNDEFINED && RS[i].Qk == UNDEFINED) {
                        RS[i].Ready = true;
                        new_ready_list.push_back(RS+i);
                    }
                }
            }
        }
        sort(new_ready_list.begin(), new_ready_list.end(), RS_Entry::compare);
        for (RS_Entry *i : new_ready_list) {
            ready_RS.push_back(i);
        }
        assert(ready_RS.size() <= max_RS_Add);
    }

    cycle ++;
}

void Reservation_Station::print() {
    sort(finished.begin(), finished.end(), Instruction::compare);
    for (const Instruction& i : finished) {
        i.print();
    }
}
