#include "../../../src/utils/rand.h"
#include "../../../src/object/Party.h"
#include <iostream>
#include "../../../src/utils/vectorUtil.h"
#include "../../../src/utils/fixed_point.h"
#include <string>
#include "../../../src/protocols/Party/CircuitForParty.h"
using namespace ppml_with_hp;
using namespace std;

void InitializeConv(std::vector<Conv2DOp>& Convop){
    //out channels input channels height width
    Convop[0].kernel_shape_ = {256,384,3,3};
    //input channels height width
    Convop[0].input_shape_ = {384,3,3};
    //output channels height width
    Convop[0].output_shape_ = {256,3,3};
    Convop[0].dilations_ ={1,1};
    Convop[0].pads_ ={1,1,1,1};
    Convop[0].strides_ ={1,1}; 
}

int main() {
    std::size_t numParties = 2;
    std::size_t myId = 1;
    std::size_t portBase = 5000;
    std::string taskName = "mult2Party";
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    std::string setUpFilePath = "../../../src/taskData/" + taskName + "/" + setupPhase + ".txt";
    ppml_with_hp::Party<uint64_t> party2(numParties,myId,portBase,taskName, NetworkMode::LAN);
    //读取SetUp阶段生成的数据
    party2.getKeyFromSetUpFile(setUpFilePath);
    //接收MACSHARE
    party2.recvMacShare();
    party2.getAllPseudoRandFromFile("../../../src/taskData/" + taskName + "/" + setupPhase);
    //定义电路
    CircuitForParty<uint64_t> circuit(party2);

    //test conv2D & conv2DTrunc correctness
    size_t rows = 3;
    size_t cols = 3;
    size_t input_channels = 384;
    size_t output_channels = 256;
    auto input_image = circuit.input(0 , input_channels * rows , cols);
    //设置卷积参数
    std::vector <Conv2DOp> Convop(1);
    InitializeConv(Convop);
    auto kernel1 = circuit.input(0, Convop[0].kernel_shape_[0] * Convop[0].kernel_shape_[1],
        Convop[0].kernel_shape_[2] * Convop[0].kernel_shape_[3]);
    auto out = circuit.conv2DTrunc(input_image , kernel1 , Convop[0]);
    auto c = circuit.output(out);
    circuit.addEndPoint(c);
    circuit.runOffline();
    circuit.runOnline();

    //test avgPool2D correctness
    // std::vector <MaxPoolOp> Poolop(1);
    // InitalizePool(Poolop);
    // auto input_image = circuit.input(0 , Poolop[0].input_shape_[0] * Poolop[0].input_shape_[1] , Poolop[0].input_shape_[2]);
    // auto out = circuit.avgPool2D(input_image , Poolop[0]);
    // auto c = circuit.output(out);
    // circuit.addEndPoint(c);
    // circuit.runOffline();
    // circuit.runOnline();
    // auto o = c->getClear();
    // cout << "output: ";
    // for(size_t i = 0 ; i < o.size(); i++){
    //     cout << fix2double<uint64_t>(o[i]) << " ";
    // }
    // cout<<endl;
}

