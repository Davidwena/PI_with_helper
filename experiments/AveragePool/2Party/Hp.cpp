#include "../../../src/utils/rand.h"
#include "../../../src/object/Hp.h"
#include <iostream>
#include "../../../src/utils/vectorUtil.h"
#include <string>
#include "../../../src/protocols/Hp/CircuitForHp.h"
#include "../../../src/utils/tensor.h"

using namespace std;
using namespace ppml_with_hp;

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
    //默认Hp的Id为最后一个
    std::size_t numParties = 2;
    std::size_t myId = numParties;
    std::size_t portBase = 5000;
    std::string taskName = "mult2Party";
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    std::string setUpFilePath = "../../../src/taskData/" + taskName + "/" + setupPhase + ".txt";
    ppml_with_hp::Hp<uint64_t> hp(numParties,myId,portBase,taskName,NetworkMode::WAN);
    //读取SetUp阶段生成的数据
    hp.getKeyFromSetUpFile(setUpFilePath);
    hp.genMacKeyAndDis();
    hp.getAllPseudoRandFromFile("../../../src/taskData/" + taskName + "/" + setupPhase);

    //定义电路
    CircuitForHp<uint64_t> circuit(hp);

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

