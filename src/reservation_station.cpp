#include "reservation_station.h"

RS_Entry::RS_Entry() {
    this->flush();
}

void RS_Entry::flush() {
    Busy = Ready = Running = false;
    Vj = Vk = Qj = Qk = Address = UNDEFINED;
}

bool RS_Entry::compare(RS_Entry *i, RS_Entry *j) {
    return i->inst.id < j->inst.id;
}

Reservation_Station::Reservation_Station(int n_rs_add,
    int n_fu_add, int n_rs_mult, int n_fu_mult, int n_rs_load, int n_fu_load) : 
    max_RS_Add(n_rs_add), max_RS_Mult(n_rs_mult), max_RS_Load(n_rs_load) {

    this->next_idx = 0;
    this->freeze = false;

    this->avail_FU_Add = n_fu_add;
    this->RS_Add = new RS_Entry[max_RS_Add];

    this->avail_FU_Mult = n_fu_mult;
    this->RS_Mult = new RS_Entry[max_RS_Mult];

    this->avail_FU_Load = n_fu_load;
    this->RS_Load = new RS_Entry[max_RS_Load];

    this->cycle = 1;
    for (int i = 0; i < 32; i++) {
        register_result_status[i] = UNDEFINED;
        register_value[i] = 0;
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

int Reservation_Station::inst_issue(Instruction& _inst) {
    if (freeze) return -1;
    int idx;
    Instruction inst = Instruction::instantial(_inst);
    switch(inst.op) {
        case INST_LD:
            if ((idx = RS_Load_avail()) != -1) {
                RS_Load[idx].Busy = true;
                register_result_status[inst.dst] = inst.id;
                inst.issue_cycle = cycle;
                RS_Load[idx].Address = inst.src1;
                RS_Load[idx].inst = inst;
            }
            break;
        case INST_JUMP:
            assert(inst.dst_t == INST_INT);
            assert(inst.src1_t == INST_REG);
            assert(inst.src2_t == INST_INT);
            if ((idx = RS_Add_avail()) != -1) {
                RS_Add[idx].Busy = true;
                RS_Add[idx].Vj = inst.dst;
                if ((RS_Add[idx].Qj = register_result_status[inst.src1]) == UNDEFINED) {
                    RS_Add[idx].Vj -= register_value[inst.src1]; // NOTE here
                }
                RS_Add[idx].Vk = inst.src2;
                inst.issue_cycle = cycle;
                RS_Add[idx].inst = inst;
                RS_Add[idx].Address = next_idx;
                freeze = true;
            }
            break;
        case INST_ADD:
        case INST_SUB:
            if ((idx = RS_Add_avail()) != -1) {
                RS_Add[idx].Busy = true;
                if (inst.src1_t == INST_REG) {
                    if ((RS_Add[idx].Qj = register_result_status[inst.src1]) == UNDEFINED) {
                        RS_Add[idx].Vj = register_value[inst.src1];
                    }
                } else {
                    RS_Add[idx].Qj = UNDEFINED;
                    RS_Add[idx].Vj = inst.src1;
                }
                if (inst.src2_t == INST_REG) {
                    if ((RS_Add[idx].Qk = register_result_status[inst.src2]) == UNDEFINED) {
                        RS_Add[idx].Vk = register_value[inst.src2];
                    }
                } else {
                    RS_Add[idx].Qk = UNDEFINED;
                    RS_Add[idx].Vk = inst.src2;
                }
                register_result_status[inst.dst] = inst.id;
                inst.issue_cycle = cycle;
                RS_Add[idx].inst = inst;
            }
            break;
        case INST_MUL:
        case INST_DIV:
            if ((idx = RS_Mult_avail()) != -1) {
                RS_Mult[idx].Busy = true;
                if (inst.src1_t == INST_REG) {
                    if ((RS_Mult[idx].Qj = register_result_status[inst.src1]) == UNDEFINED) {
                        RS_Mult[idx].Vj = register_value[inst.src1];
                    }
                } else {
                    RS_Mult[idx].Qj = UNDEFINED;
                    RS_Mult[idx].Vj = inst.src1;
                }
                if (inst.src2_t == INST_REG) {
                    if ((RS_Mult[idx].Qk = register_result_status[inst.src2]) == UNDEFINED) {
                        RS_Mult[idx].Vk = register_value[inst.src2];
                    }
                } else {
                    RS_Mult[idx].Qk = UNDEFINED;
                    RS_Mult[idx].Vk = inst.src2;
                }
                register_result_status[inst.dst] = inst.id;
                inst.issue_cycle = cycle;
                RS_Mult[idx].inst = inst;
            }
            break;
        default:
            cout << "invalid op" << endl;
            exit(-1);
    }
    if (idx != -1) {
        issued_list.push_back(_inst.inst_string);
        _inst.to_record = false;
    }
    return idx;
}

void Reservation_Station::update(int id, int val) {
    for (int i = 0; i < max_RS_Add; i++) {
        if (RS_Add[i].Busy && !RS_Add[i].Ready) {
            if (RS_Add[i].Qj == id) {
                RS_Add[i].Qj = UNDEFINED;
                if (RS_Add[i].inst.op == INST_JUMP) { // NOTE here
                    RS_Add[i].Vj -= val;
                    continue;
                } else RS_Add[i].Vj = val;
            }
            if (RS_Add[i].Qk == id) {
                RS_Add[i].Qk = UNDEFINED;
                RS_Add[i].Vk = val;
            }
        }
    }
    for (int i = 0; i < max_RS_Mult; i++) {
        if (RS_Mult[i].Busy && !RS_Mult[i].Ready) {
            if (RS_Mult[i].Qj == id) {
                RS_Mult[i].Qj = UNDEFINED;
                RS_Mult[i].Vj = val;
            }
            if (RS_Mult[i].Qk == id) {
                RS_Mult[i].Qk = UNDEFINED;
                RS_Mult[i].Vk = val;
            }
        }
    }
}

void Reservation_Station::execute() {
    // write result
    int FU_release[3] = {0};
    for (int k = 0; k < 3; k++) {
        const int& max_RS = (k == 0 ? max_RS_Load : (k == 1 ? max_RS_Add : max_RS_Mult));
        // int& avail_FU = (k == 0 ? avail_FU_Load : (k == 1 ? avail_FU_Add : avail_FU_Mult));
        RS_Entry *RS = (k == 0 ? RS_Load : (k == 1 ? RS_Add : RS_Mult));

        for (int i = 0; i < max_RS; i++) {
            if (RS[i].Running && RS[i].inst.remaining_cycle == 0) { 
                write_list.push_back(RS[i].inst.inst_string);
                RS[i].inst.write_result_cycle = cycle;
                assert(RS[i].inst.write_result_cycle == RS[i].inst.exec_comp_cycle + 1);
                if (RS[i].inst.to_record) finished.push_back(RS[i].inst);
                int val = calculate(RS+i);
                // if (RS[i].inst.op == INST_JUMP) { // NOTE here
                //     freeze = false;
                //     next_idx = val;
                // }
                if (register_result_status[RS[i].inst.dst] == RS[i].inst.id) {
                    register_result_status[RS[i].inst.dst] = UNDEFINED;
                    register_value[RS[i].inst.dst] = val;
                }
                update(RS[i].inst.id, val);
                // avail_FU ++;
                FU_release[k] ++;
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
                        finished_list.push_back(RS[i].inst.inst_string);
                        if (RS[i].inst.op == INST_JUMP) { // NOTE here
                            freeze = false;
                            next_idx = calculate(RS+i);
                        }
                    }
                } else if (!RS[i].Ready) {
                    if (RS[i].Qj == UNDEFINED && RS[i].Qk == UNDEFINED) {
                        RS[i].Ready = true;
                        ready_list.push_back(RS[i].inst.inst_string);
                        if (RS[i].inst.op == INST_DIV && RS[i].Vk == 0) { // div by 0
                            RS[i].inst.remaining_cycle = TIME_DIV_0;
                        }
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

    for (int k = 0; k < 3; k++) {
        int& avail_FU = (k == 0 ? avail_FU_Load : (k == 1 ? avail_FU_Add : avail_FU_Mult));
        avail_FU += FU_release[k];
    }
    cycle ++;
}

void Reservation_Station::sort_finished() {
    sort(finished.begin(), finished.end(), Instruction::compare_line_num);
}

void Reservation_Station::print() const {
    for (const Instruction& i : finished) {
        i.print();
    }
}

void Reservation_Station::print_reg() const {
    for (int i = 0; i < 32; i++) {
        cout << "R" << i << ": " << register_value[i] << "\t" << "S:" << register_result_status[i] << endl;
    }
}

void Reservation_Station::load(ifstream& infile) {
    if (infile.is_open()) {
        int line_num = 0;
        string line;
        while (getline(infile, line)) {
            inst_pool.push_back(Instruction::parse(line_num++, line));
        }
    } else 
        cerr << "failed to open file" << endl;
}

void Reservation_Station::step() {
    if (inst_issue(inst_pool[next_idx]) != -1) {
        next_idx ++;
    }
    execute();
}

void Reservation_Station::run() {
    assert(next_idx == 0);
    do {
        step();
        // print_step();
    } while (next_idx < inst_pool.size() && next_idx >= 0);

    while(!is_finished()) {
        execute();
        // print_step();
    }

    sort_finished();
    // print();
    // print_reg();
}

int Reservation_Station::calculate(RS_Entry *rs) {
    int ret;
    switch (rs->inst.op) {
        case INST_LD:
            ret = rs->Address;
            break;
        case INST_ADD:
            ret = rs->Vj + rs->Vk;
            break;
        case INST_SUB:
            ret = rs->Vj - rs->Vk;
            break;
        case INST_MUL:
            ret = rs->Vj * rs->Vk;
            break;
        case INST_DIV:
            ret = (rs->Vk == 0 ? rs->Vj : (rs->Vj / rs->Vk));
            break;
        case INST_JUMP:
            ret = (rs->Vj == 0 ? (rs->Address + rs->Vk) : (rs->Address + 1));
            break;
        default:
            cout << "invalid op" << endl;
            exit(-1);
    }
    return ret;
}

void Reservation_Station::flush_list() {
    issued_list.clear();
    ready_list.clear();
    finished_list.clear();
    write_list.clear();
    event_list.clear();
}

void Reservation_Station::print_step() {
    cout << "Cycle " << cycle-1 << endl;
    cout << "next_idx " << next_idx << endl;

    cout << "issued:";
    for (const string& i : issued_list)
        cout << " " << i;
    cout << endl;

    cout << "ready:";
    for (const string& i : ready_list)
        cout << " " << i;
    cout << endl;

    cout << "finished:";
    for (const string& i : finished_list)
        cout << " " << i;
    cout << endl;

    cout << "write:";
    for (const string& i : write_list)
        cout << " " << i;
    cout << endl;

    cout << "event:" << endl;
    for (const string& i : event_list)
        cout << "- " << i << endl;

    cout << "--------------------------------------" << endl;

    print_reg();

    string x;
    cin >> x;
    flush_list();
}

void Reservation_Station::output(ofstream& outfile) {
    if (outfile.is_open()) {
        for (const Instruction& i : finished) {
            outfile << i.issue_cycle << " " 
                << i.exec_comp_cycle << " " 
                << i.write_result_cycle << endl;
        }
    } else
        cerr << "failed to open file" << endl;
}
