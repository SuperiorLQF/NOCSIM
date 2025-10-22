#ifndef _NAND2_H
#define _NAND2_H
#include<systemc.h>
SC_MODULE(nand2){
    sc_in<bool>A,B;
    sc_out<bool>F;
    void do_nand(){
        F=!(A&B);
    };
    SC_CTOR(nand2){
        SC_METHOD(do_nand);
        sensitive<<A<<B;
    }
};
#endif