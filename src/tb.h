#ifndef _TBRS_H
#define _TBRS_H
#include <systemc.h>

SC_MODULE(tb_regslice) {
    // 信号
    sc_in_clk       clk;
    sc_out<bool>    rst_n;

    sc_out<bool> mst_vld;
    sc_in<bool>  mst_rdy;
    sc_out<int>  mst_payload;

    sc_in<bool>  slv_vld;
    sc_out<bool> slv_rdy;
    sc_in<int>   slv_payload;


    void master_proc();
    void slave_proc();
    void reset_proc();
    void sim_time_ctrl();
    
    SC_CTOR(tb_regslice)
    {
        SC_THREAD(reset_proc);

        SC_CTHREAD(sim_time_ctrl,clk.pos());
        reset_signal_is(rst_n,false);        

        SC_CTHREAD(master_proc,clk.pos());
        reset_signal_is(rst_n,false);

        SC_CTHREAD(slave_proc,clk.pos());
        reset_signal_is(rst_n,false);

    }


};
void tb_regslice::sim_time_ctrl(){
    wait(100);
    sc_stop();
}
void tb_regslice::reset_proc() {
    rst_n.write(0);
    wait(50, SC_NS);
    rst_n.write(1);
}
// master 线程：驱动输入
void tb_regslice::master_proc() {
    mst_vld.write(false);
    mst_payload.write(0);
    wait();  // 等待一个上升沿

    for (int i = 0; i < 10; i++) {
        mst_payload.write(i);
        mst_vld.write(true);
        do {
            wait(); // 等待直到 regslice 接受数据
        } while (!mst_rdy.read());
        cout << sc_time_stamp() << " [MASTER] send data = " << i << endl;
        wait(); // 数据已推送成功

    }

    mst_vld.write(false);
}

// slave 线程：接受输出
void tb_regslice::slave_proc() {
    slv_rdy.write(false);

    while (true) {

        if (slv_vld.read() && slv_rdy.read()) {
            cout << sc_time_stamp() << " [SLAVE ] got  data = " << slv_payload.read() << endl;
        }

        // 随机制造 backpressure
        if (rand() % 4 == 0) {
            slv_rdy.write(false);
        } else {
            slv_rdy.write(true);
        }
        wait();
    }
}

#endif