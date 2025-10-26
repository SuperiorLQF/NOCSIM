#ifndef _TB_FIFO_BUFFER_H
#define _TB_FIFO_BUFFER_H
#include <systemc.h>

template<typename T>
SC_MODULE(tb_fifo_buffer) {
    // 信号
    sc_in_clk    clk;
    sc_out<bool> rst_n;

    sc_out<bool> mst_vld;
    sc_in<bool>  mst_rdy;
    sc_out<T>    mst_payload;

    sc_in<bool>  slv_vld;
    sc_out<bool> slv_rdy;
    sc_in<T>     slv_payload;


    void master_proc();
    void slave_proc();
    void reset_proc();
    void sim_time_ctrl();
    
    SC_CTOR(tb_fifo_buffer)
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
template<typename T>
void tb_fifo_buffer<T>::sim_time_ctrl(){
    wait(100);
    sc_stop();
}
template<typename T>
void tb_fifo_buffer<T>::reset_proc() {
    rst_n.write(0);
    wait(50, SC_NS);
    rst_n.write(1);
}
// master 线程：驱动输入
template<typename T>
void tb_fifo_buffer<T>::master_proc() {
    T    payload_tmp;
    mst_vld.write(false);
    mst_payload.write(T());
    wait();  // 等待一个上升沿

    for (int i = 0; i < 10; i++) {
        //step 随出vld(0~n拍)

        bool vld_random = (rand()%4==0)?false:true;
        mst_vld.write(vld_random);

        while(!vld_random){
            wait();
            vld_random = (rand()%4==0)?false:true;
        }

        mst_vld.write(vld_random);
        payload_tmp.randomize();
        mst_payload.write(payload_tmp);

        do{
            wait();
        }
        while (!(mst_rdy.read()&&mst_vld.read()));
        cout << sc_time_stamp() << " [MASTER] send data = " << payload_tmp << endl;

    }

    mst_vld.write(false);
}

// slave 线程：接受输出
template<typename T>
void tb_fifo_buffer<T>::slave_proc() {
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