#include "../../../src/utils/rand.h"
#include "../../../src/object/Party.h"
#include <iostream>
#include "../../../src/utils/vectorUtil.h"
#include "../../../src/utils/fixed_point.h"
#include <string>
#include "../../../src/protocols/Party/CircuitForParty.h"
using namespace ppml_with_hp;
using namespace std;


int main() {
    std::size_t numParties = 2;
    std::size_t myId = 1;
    std::size_t portBase = 5000;
    std::string taskName = "mult2Party";
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    std::string setUpFilePath = "../../../src/taskData/" + taskName + "/" + setupPhase + ".txt";
    ppml_with_hp::Party<uint64_t> party2(numParties,myId,portBase,taskName, NetworkMode::WAN);
    //读取SetUp阶段生成的数据
    party2.getKeyFromSetUpFile(setUpFilePath);
    //接收MACSHARE
    party2.recvMacShare();
    party2.getAllPseudoRandFromFile("../../../src/taskData/" + taskName + "/" + setupPhase);
    //定义电路
    CircuitForParty<uint64_t> circuit(party2);

    // auto a = circuit.input(0 , 32 * 32 , 1);
    // auto b = circuit.matrixDot(a, a);
    // auto c = circuit.output(b);
    // circuit.addEndPoint(c); 
    // auto start = std::chrono::high_resolution_clock::now();
    // circuit.runOffline();
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // std::cout << "预处理执行时间: " << duration.count() << " 毫秒" << std::endl;

    // start = std::chrono::high_resolution_clock::now();
    // circuit.runOnline();
    // end = std::chrono::high_resolution_clock::now();
    // duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // std::cout << "在线执行时间: " << duration.count() << " 毫秒" << std::endl;

    auto a = circuit.input(0 , 384 , 1);
    auto b = circuit.matrixDot(a, a);
    circuit.addEndPoint(b);
    auto c = circuit.elemMult(a , a);
    auto d = circuit.elemMult(c , a);
    circuit.addEndPoint(d);
    auto start = std::chrono::high_resolution_clock::now();
    circuit.runOffline();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "预处理执行时间: " << duration.count() << " 毫秒" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    circuit.runOnline();
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "在线执行时间: " << duration.count() << " 毫秒" << std::endl;
}

