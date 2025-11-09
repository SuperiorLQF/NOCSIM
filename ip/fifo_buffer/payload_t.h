#ifndef _PAYLOAD_T_H
#define _PAYLOAD_T_H
#include <systemc.h>
#include <iostream>

struct payload_default {
    int data;

    payload_default() : data(0) {}  // 默认构造函数，SystemC信号必须有

    void randomize() { data = rand() % 256; }

    // 输出到 ostream（调试打印）
    friend std::ostream& operator<<(std::ostream& os, const payload_default& p) {
        os << "{"<<"data:"<<p.data<<"}";
        return os;
    }

    // 比较运算符：sc_signal<payload_default> 需要 == 操作
    bool operator==(const payload_default& rhs) const {
        return data == rhs.data;
    }
};

// ----------------------------------------------------------
// 关键：trace 支持函数（SystemC 需要的外部函数）
// ----------------------------------------------------------
inline void sc_trace(sc_trace_file* tf, const payload_default& p, const std::string& name)
{
    // 因为payload_default只是封装了一个int，所以我们实际上追踪p.data
    // 如果payload中有更多字段，可以在这里分别trace出来
    sc_trace(tf, p.data, name + ".data");
}

#endif