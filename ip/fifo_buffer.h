#ifndef _FIFO_BUFFER_H
#define _FIFO_BUFFER_H
#include <systemc.h>
#include <queue>
template<typename T,uint16_t DEPTH>
SC_MODULE(fifo_buffer){
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

    //important inner sig----------
    std::queue<T> fifo;
    sc_signal<uint16_t> fifo_cnt;
    sc_signal<bool> fifo_empty;
    sc_signal<bool> fifo_full;
    //-----------------------------

    void do_fifo();
    SC_CTOR(fifo_buffer){
        SC_CTHREAD(do_fifo,clk.pos());
        reset_signal_is(rst_n,false);
    }

};
template<typename T,uint16_t DEPTH>
void fifo_buffer<T,DEPTH>::do_fifo(){
    bool push,pop;
    slv_vld.write(0);
    mst_rdy.write(1);
    slv_payload.write(T());
    fifo_empty.write(1);
    fifo_full.write(0);
    fifo_cnt.write(0);
    wait();
    while(true){
        push = mst_vld.read()&&mst_rdy.read();
        pop  = slv_vld.read()&&slv_rdy.read();
        if(push){
            fifo.push(mst_payload.read());
        }
        if(pop){
            fifo.pop();
        }

        fifo_cnt.write(fifo.size());
        fifo_empty.write(fifo.empty());
        fifo_full.write(fifo.size()>=DEPTH);

        slv_vld.write(!fifo.empty());
        mst_rdy.write(fifo.size()<DEPTH);

        if(!fifo.empty()){
            slv_payload.write(fifo.front());
        }

        wait();
    }
}
template<typename T, uint16_t DEPTH>
inline void sc_trace(sc_trace_file* tf, const fifo_buffer<T, DEPTH>& inst, const std::string& name)
{
    sc_trace(tf, inst.clk,        name + ".clk");
    sc_trace(tf, inst.rst_n,      name + ".rst_n");
    sc_trace(tf, inst.mst_vld,    name + ".mst_vld");
    sc_trace(tf, inst.mst_rdy,    name + ".mst_rdy");
    sc_trace(tf, inst.mst_payload,name + ".mst_payload");
    sc_trace(tf, inst.slv_vld,    name + ".slv_vld");
    sc_trace(tf, inst.slv_rdy,    name + ".slv_rdy");
    sc_trace(tf, inst.slv_payload,name + ".slv_payload");
    sc_trace(tf, inst.fifo_cnt,   name + ".fifo_cnt");
    sc_trace(tf, inst.fifo_empty, name + ".fifo_empty");
    sc_trace(tf, inst.fifo_full,  name + ".fifo_full");
}
#endif