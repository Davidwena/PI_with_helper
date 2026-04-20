#include "../../../src/utils/rand.h"
#include "../../../src/object/Party.h"
#include <iostream>
#include "../../../src/utils/vectorUtil.h"
#include "../../../src/utils/fixed_point.h"
#include <string>
#include "../../../src/protocols/Party/CircuitForParty.h"
using namespace ppml_with_hp;
using namespace std;

void InitalizePool(std::vector<MaxPoolOp>& Poolop){
    //kernel size stride
    uint32_t kernel_size = 9;
    uint32_t poolstride = 9;
    //input channels height width
    Poolop[0].input_shape_ = {120,9,9};
    //output channels height width
    Poolop[0].output_shape_ = {120,1,1};
    Poolop[0].kernel_shape_ = {kernel_size, kernel_size};
    Poolop[0].strides_ = {poolstride, poolstride};
}

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
    
    //test avgPool2D correctness
    std::vector <MaxPoolOp> Poolop(1);
    InitalizePool(Poolop);
    auto input_image = circuit.input(0 , Poolop[0].input_shape_[0] * Poolop[0].input_shape_[1] , Poolop[0].input_shape_[2]);
    auto out = circuit.avgPool2D(input_image , Poolop[0]);
    auto c = circuit.output(out);
    circuit.addEndPoint(c);
    circuit.runOffline();
    circuit.runOnline();
}

