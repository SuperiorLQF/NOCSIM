#include <systemc.h>
#include "tb.h"  
#include "regslice.h"      // DUT

int sc_main(int argc, char* argv[]) {

    // 信号定义
    sc_clock        clk("clk", 10, SC_NS);     // 100 MHz
    sc_signal<bool> rst_n;

    sc_signal<bool> mst_vld;
    sc_signal<bool> mst_rdy;
    sc_signal<int>  mst_payload;

    sc_signal<bool> slv_vld;
    sc_signal<bool> slv_rdy;
    sc_signal<int>  slv_payload;

    // 实例化 DUT
    regslice<int> u_dut("u_dut");
    u_dut.clk(clk);
    u_dut.rst_n(rst_n);
    u_dut.mst_vld(mst_vld);
    u_dut.mst_rdy(mst_rdy);
    u_dut.mst_payload(mst_payload);
    u_dut.slv_vld(slv_vld);
    u_dut.slv_rdy(slv_rdy);
    u_dut.slv_payload(slv_payload);

    // 实例化 TBench
    tb_regslice tb("tb");
    tb.clk(clk);
    tb.rst_n(rst_n);
    tb.mst_vld(mst_vld);
    tb.mst_rdy(mst_rdy);
    tb.mst_payload(mst_payload);
    tb.slv_vld(slv_vld);
    tb.slv_rdy(slv_rdy);
    tb.slv_payload(slv_payload);

    // ====== 波形 trace ======
    sc_trace_file* tf = sc_create_vcd_trace_file("regslice_wave");
    tf->set_time_unit(1, SC_NS);

    sc_trace(tf, clk,         "clk");
    sc_trace(tf, rst_n,       "rst_n");
    sc_trace(tf, mst_vld,     "mst_vld");
    sc_trace(tf, mst_rdy,     "mst_rdy");
    sc_trace(tf, mst_payload, "mst_payload");
    sc_trace(tf, slv_vld,     "slv_vld");
    sc_trace(tf, slv_rdy,     "slv_rdy");
    sc_trace(tf, slv_payload, "slv_payload");

    // ========== 启动仿真 ==========
    cout << "=== Simulation Start ===" << endl;
    sc_start();
    cout << "=== Simulation End   ===" << endl;
   
    sc_close_vcd_trace_file(tf);

    return 0;

}