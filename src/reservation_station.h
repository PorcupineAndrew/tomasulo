#ifndef _RESERVATION_STATION_H_
#define _RESERVATION_STATION_H_
#include "instruction.h"

class RS_Entry {
public:
    bool Busy;
    bool Ready;
    bool Running;
    Instruction inst;
    int Vj;
    int Vk;
    int Qj;
    int Qk;
    int Address;

    RS_Entry();
    void flush();
    
    static bool compare(RS_Entry *i, RS_Entry *j);
};

class Reservation_Station {
private:
    vector<Instruction> finished;

    int cycle;
    int register_result_status[32];
    
    const int max_RS_Add;
    int avail_FU_Add;
    RS_Entry *RS_Add;
    vector<RS_Entry *> ready_RS_Add;

    const int max_RS_Mult;
    int avail_FU_Mult;
    RS_Entry *RS_Mult;
    vector<RS_Entry *> ready_RS_Mult;

    const int max_RS_Load;
    int avail_FU_Load;
    RS_Entry *RS_Load;
    vector<RS_Entry *> ready_RS_Load;

public:
    Reservation_Station(int n_rs_add, int n_fu_add, int n_rs_mult,
                            int n_fu_mult, int n_rs_load, int n_fu_load);
    ~Reservation_Station();
    
    int RS_Add_avail() const;
    int RS_Mult_avail() const;
    int RS_Load_avail() const;
    bool is_finished() const;

    int inst_issue(Instruction inst);
    void update(int idx);
    void run();

    void print();
};

#endif
