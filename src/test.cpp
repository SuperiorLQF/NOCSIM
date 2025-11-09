// router_tlm.cpp
// 编译方法： g++ -std=c++11 router_tlm.cpp -lsystemc -o router
// 运行方法： ./router

#include <systemc>
#include <tlm>
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/peq_with_cb_and_phase.h> // 包含内存管理器需要
#include <iostream>
#include <string>
#include <vector>

// ========================
// 自定义内存管理器，用于管理 tlm_generic_payload 对象
// ========================
class MemoryManager : public tlm::tlm_mm_interface {
private:
    std::vector<tlm::tlm_generic_payload*> free_list;
    std::vector<unsigned char*> data_list;

public:
    tlm::tlm_generic_payload* allocate() {
        if (free_list.empty()) {
            // 如果池已空，创建一个新的
            auto* new_payload = new tlm::tlm_generic_payload(this);
            auto* new_data = new unsigned char[16]; // 分配足够大的数据空间
            new_payload->set_data_ptr(new_data);
            data_list.push_back(new_data); // 记录数据指针以便最后释放
            return new_payload;
        } else {
            // 从池中取一个
            tlm::tlm_generic_payload* ptr = free_list.back();
            free_list.pop_back();
            return ptr;
        }
    }

    void free(tlm::tlm_generic_payload* trans) override {
        // 将用完的payload放回池中
        trans->reset();
        free_list.push_back(trans);
    }
    
    // 析构函数，释放所有分配的内存
    ~MemoryManager() {
        for(auto p : free_list) {
            delete[] p->get_data_ptr();
            delete p;
        }
         for(auto p : data_list) { // 确保所有创建的数据区都被删除
            bool in_free_list = false;
            for(auto fp : free_list) {
                if (fp->get_data_ptr() == p) {
                    in_free_list = true;
                    break;
                }
            }
            if(!in_free_list) delete[] p;
        }
    }
};


// ========================
// Router 模块定义 (无变化)
// ========================
SC_MODULE(Router) {
    tlm_utils::simple_target_socket<Router> local_in, north_in, south_in, east_in, west_in;
    tlm_utils::simple_initiator_socket<Router> local_out, north_out, south_out, east_out, west_out;

    SC_CTOR(Router)
        : local_in("local_in"), north_in("north_in"), south_in("south_in"), east_in("east_in"), west_in("west_in"),
          local_out("local_out"), north_out("north_out"), south_out("south_out"), east_out("east_out"), west_out("west_out")
    {
        local_in.register_b_transport(this, &Router::b_transport_local);
        north_in.register_b_transport(this, &Router::b_transport_north);
        south_in.register_b_transport(this, &Router::b_transport_south);
        east_in.register_b_transport(this, &Router::b_transport_east);
        west_in.register_b_transport(this, &Router::b_transport_west);
    }

    void b_transport_local(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) { route("LOCAL", trans, delay); }
    void b_transport_north(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) { route("NORTH", trans, delay); }
    void b_transport_south(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) { route("SOUTH", trans, delay); }
    void b_transport_east(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) { route("EAST", trans, delay); }
    void b_transport_west(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) { route("WEST", trans, delay); }

    void route(const std::string& in_port, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
        unsigned char* data_ptr = trans.get_data_ptr();
        int dest = data_ptr[0];
        std::cout << sc_core::sc_time_stamp() << ": [Router] Received packet from " << in_port << ", dest=" << dest << std::endl;

        switch(dest) {
            case 0: local_out->b_transport(trans, delay); break;
            case 1: north_out->b_transport(trans, delay); break;
            case 2: south_out->b_transport(trans, delay); break;
            case 3: east_out->b_transport(trans, delay); break;
            case 4: west_out->b_transport(trans, delay); break;
            default: 
                std::cerr << "[Router] Unknown dest! Packet dropped." << std::endl;
                // 如果是未知目的地，需要释放payload，防止内存泄漏
                trans.release();
                break;
        }
    }
};

// ========================
// 测试模块，集成了内存管理
// ========================
SC_MODULE(TestNode) {
    tlm_utils::simple_target_socket<TestNode> target_socket;
    tlm_utils::simple_initiator_socket<TestNode> initiator_socket;
    MemoryManager* mm; // 指向内存管理器的指针

    SC_CTOR(TestNode)
        : target_socket("target_socket"), initiator_socket("initiator_socket"), mm(nullptr)
    {
        target_socket.register_b_transport(this, &TestNode::b_transport);
    }

    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
        unsigned char* data_ptr = trans.get_data_ptr();
        std::cout << sc_core::sc_time_stamp() << ": [" << name() << "] Received packet, dest=" << (int)data_ptr[0] << std::endl;
        
        // 作为最终目的地，我们负责释放payload
        trans.release();
    }

    void send_packet(int dest) {
        // 从内存管理器获取一个payload
        tlm::tlm_generic_payload* trans = mm->allocate();
        trans->acquire(); // 增加引用计数

        sc_core::sc_time delay = sc_core::SC_ZERO_TIME;
        unsigned char* data = trans->get_data_ptr();
        
        data[0] = dest;
        data[1] = 0xAA;
        data[2] = 0xBB;
        data[3] = 0xCC;

        trans->set_data_length(4);
        trans->set_write();

        std::cout << sc_core::sc_time_stamp() << ": [" << name() << "] Sending packet with dest=" << dest << std::endl;
        initiator_socket->b_transport(*trans, delay);
    }
};

// ========================
// 桩模块，用于终结未使用的端口
// ========================
SC_MODULE(StubNode) {
    tlm_utils::simple_target_socket<StubNode> target_socket;
    tlm_utils::simple_initiator_socket<StubNode> initiator_socket;

    SC_CTOR(StubNode) : target_socket("target_socket"), initiator_socket("initiator_socket") {
        target_socket.register_b_transport(this, &StubNode::b_transport);
    }

    // 接收到数据后释放payload
    void b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay) {
        std::cout << sc_core::sc_time_stamp() << ": [" << name() << "] Received and absorbed a packet." << std::endl;
        trans.release(); // 释放payload
    }
};


// ========================
// 主函数：搭建完整的连接测试
// ========================
int sc_main(int argc, char* argv[]) {
    // 1. 实例化所有模块
    Router router("router");
    TestNode node_local("node_local");
    TestNode node_north("node_north");
    StubNode stub_south("stub_south");
    StubNode stub_east("stub_east");
    StubNode stub_west("stub_west");
    
    // 实例化内存管理器
    MemoryManager mm;

    // 2. 将内存管理器指针设置给所有会发送数据的节点
    node_local.mm = &mm;
    node_north.mm = &mm;
    // 桩模块不需要发送，所以不用设置

    // 3. 建立完整的双向连接
    // Local
    node_local.initiator_socket.bind(router.local_in);
    router.local_out.bind(node_local.target_socket);
    
    // North
    node_north.initiator_socket.bind(router.north_in);
    router.north_out.bind(node_north.target_socket);

    // South (连接到桩)
    stub_south.initiator_socket.bind(router.south_in);
    router.south_out.bind(stub_south.target_socket);
    
    // East (连接到桩)
    stub_east.initiator_socket.bind(router.east_in);
    router.east_out.bind(stub_east.target_socket);

    // West (连接到桩)
    stub_west.initiator_socket.bind(router.west_in);
    router.west_out.bind(stub_west.target_socket);


    // 4. 开始仿真和测试
    sc_core::sc_start(sc_core::SC_ZERO_TIME);
    
    std::cout << "--- Test 1: Sending packet from LOCAL to NORTH ---" << std::endl;
    node_local.send_packet(1); // 目的: North

    sc_core::sc_start(1, sc_core::SC_NS);

    std::cout << "\n--- Test 2: Sending packet from NORTH to LOCAL ---" << std::endl;
    node_north.send_packet(0); // 目的: Local

    sc_core::sc_start(1, sc_core::SC_NS);
    
    std::cout << "\n--- Test 3: Sending packet from LOCAL to EAST (will be absorbed by stub) ---" << std::endl;
    node_local.send_packet(3); // 目的: East

    sc_core::sc_start(1, sc_core::SC_NS);

    std::cout << "\n--- Simulation finished ---" << std::endl;
    return 0;
}