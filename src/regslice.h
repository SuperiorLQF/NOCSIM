#ifndef _REGSLICE_H
#define _REGSLICE_H
#include <systemc.h>
template<typename T>
SC_MODULE(regslice){
    //port-------------------------
    sc_in_clk       clk;
    sc_in<bool>     rst_n;

    sc_in<bool>     mst_vld;
    sc_out<bool>    mst_rdy;
    sc_in<T>        mst_payload;

    sc_out<bool>    slv_vld;
    sc_in<bool>     slv_rdy;
    sc_out<T>       slv_payload;
    //-----------------------------

    void do_regslice();
    SC_CTOR(regslice){
        SC_CTHREAD(do_regslice,clk.pos());
        reset_signal_is(rst_n,false);
    }

};
template<typename T>
void regslice<T>::do_regslice(){
    bool push,pop;
    slv_vld.write(0);
    mst_rdy.write(1);
    slv_payload.write(T());
    wait();
    while(true){
        push = mst_vld.read()&&mst_rdy.read();
        pop  = slv_vld.read()&&slv_rdy.read();
        if(push&&pop){
            slv_payload.write(mst_payload.read());
            slv_vld.write(1);
            mst_rdy.write(1);
        }
        else if(push&&(!pop)){
            slv_payload.write(mst_payload.read());
            slv_vld.write(1);
            mst_rdy.write(0);            
        }
        else if((!push)&&pop){
            slv_vld.write(0);  
            mst_rdy.write(1);          
        }
        wait();
    }
}
#endif