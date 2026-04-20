#include "../../../src/utils/rand.h"
#include "../../../src/object/Party.h"
#include <iostream>
#include "../../../src/utils/vectorUtil.h"
#include "../../../src/utils/fixed_point.h"
#include <string>
#include "../../../src/protocols/Party/CircuitForParty.h"
#include "../../../src/utils/tensor.h"
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
    std::size_t myId = 0;
    std::size_t portBase = 5000;
    std::string taskName = "mult2Party";
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    std::string setUpFilePath = "../../../src/taskData/" + taskName + "/" + setupPhase + ".txt";
    ppml_with_hp::Party<uint64_t> party1(numParties,myId,portBase,taskName, NetworkMode::LAN);
    //读取SetUp阶段生成的数据
    party1.getKeyFromSetUpFile(setUpFilePath);
    //接收MACSHARE
    party1.recvMacShare();
    party1.getAllPseudoRandFromFile("../../../src/taskData/" + taskName + "/" + setupPhase);
    //定义电路
    CircuitForParty<uint64_t> circuit(party1);
    //test conv2D & conv2DTrunc correctness
    size_t rows = 3;
    size_t cols = 3;
    size_t input_channels = 384;
    size_t output_channels = 256;
    auto input_image = circuit.input(0 , input_channels * rows , cols);
    std::vector<double> input_x (input_channels * rows * cols , 1);
    auto input_x1 = double2fixVec<uint64_t>(input_x);
    input_image->setInput(input_x1);
    //设置卷积参数
    std::vector <Conv2DOp> Convop(1);
    InitializeConv(Convop);
    auto kernel1 = circuit.input(0, Convop[0].kernel_shape_[0] * Convop[0].kernel_shape_[1],
        Convop[0].kernel_shape_[2] * Convop[0].kernel_shape_[3]);
    std::vector<double> kernel1_x (output_channels * input_channels * Convop[0].kernel_shape_[2] * Convop[0].kernel_shape_[3] , 1);
    auto kernel1_x1 = double2fixVec<uint64_t>(kernel1_x);
    kernel1->setInput(kernel1_x1);
    auto out = circuit.conv2DTrunc(input_image , kernel1 , Convop[0]);
    auto c = circuit.output(out);
    circuit.addEndPoint(c);
    circuit.runOffline();
    circuit.runOnline();

}

