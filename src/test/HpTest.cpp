#include "../utils/rand.h"
#include "../object/Hp.h"
#include <iostream>
#include "../utils/vectorUtil.h"
#include <string>
#include "../protocols/Hp/CircuitForHp.h"
#include "../utils/tensor.h"

using namespace std;
using namespace ppml_with_hp;

// void InitializeConv(std::vector<Conv2DOp>& Convop){
//     //out channels input channels height width
//     Convop[0].kernel_shape_ = {2,3,3,3};
//     //input channels height width
//     Convop[0].input_shape_ = {3,4,4};
//     //output channels height width
//     Convop[0].output_shape_ = {2,2,2};
//     Convop[0].dilations_ ={1,1};
//     Convop[0].pads_ ={0,0,0,0};
//     Convop[0].strides_ ={1,1}; 
// }

void InitalizePool(std::vector<MaxPoolOp>& Poolop){
    //kernel size stride
    uint32_t kernel_size = 2;
    uint32_t poolstride = 2;
    //input channels height width
    Poolop[0].input_shape_ = {2, 4, 4};
    //output channels height width
    Poolop[0].output_shape_ = {2, 2, 2};
    Poolop[0].kernel_shape_ = {kernel_size, kernel_size};
    Poolop[0].strides_ = {poolstride, poolstride};
}

int main() {
    //默认Hp的Id为最后一个
    std::size_t numParties = 2;
    std::size_t myId = numParties;
    std::size_t portBase = 5000;
    std::string taskName = "test";
    std::string phase = "Preprocess-" + std::to_string(myId);
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    //先判断是否已经生成了Preprocess文件，如果生成了 删掉
    std::string filePath = "../taskData/" + taskName + "/" + phase + ".txt";
    removeFile(filePath);
    std::string setUpFilePath = "../taskData/" + taskName + "/" + setupPhase + ".txt";
    ppml_with_hp::Hp<uint64_t> hp(numParties,myId,portBase,taskName);
    //读取SetUp阶段生成的数据
    hp.getKeyFromSetUpFile(setUpFilePath);
    hp.genMacKeyAndDis();
    //定义电路
    CircuitForHp<uint64_t> circuit(hp);

    // //test PolyTrunc correctness
    // //compute y = 1 + 2x + 3x^2
    // std::vector<uint64_t> coefficient = {double2fix<uint64_t>(0.123),
    //                                      double2fix<uint64_t>(0.25),
    //                                      double2fix<uint64_t>(0),
    //                                      double2fix<uint64_t>(3.2),
    //                                      double2fix<uint64_t>(0.2578)};
    
    // size_t k = 4;
    // auto a = circuit.input(0,1,2);
    // // auto b = circuit.poly(a , k ,coefficient);
    // auto b = circuit.polyTrunc(a , k , coefficient);
    // auto c = circuit.output(b); 
    // circuit.addEndPoint(c);
    // //生成预处理数据
    // circuit.runOffline();
    // //执行在线阶段
    // circuit.runOnline();

    //test input and multTrunc correctness ---right
    // auto a = circuit.input(0,2,2);
    // auto b = circuit.input(0,2,2);
    // auto c = circuit.input(1,2,2);
    // auto d = circuit.multTrunc(a , b);
    // auto e = circuit.multTrunc(b , c);
    // auto f = circuit.multTrunc(d , e);
    // auto g = circuit.output(f);
    // circuit.addEndPoint(g);
    // circuit.runOffline();
    // circuit.runOnline();

    //test poly2 correctness
    // auto a = circuit.input(0,2,2);
    // auto b = circuit.poly2(a , 4 , {1,2,3,4,5});
    // auto c = circuit.output(b);
    // circuit.addEndPoint(c);
    // circuit.runOffline();
    // circuit.runOnline();

    //test ElemMult correctness
    // auto a = circuit.input(0,2,2);
    // auto b = circuit.input(1,2,2);
    // auto c = circuit.elemMult(a , b);
    // auto d = circuit.output(c);
    // circuit.addEndPoint(d);
    // circuit.runOffline();
    // circuit.runOnline();
    
    //test poly2Trun correctness
    // auto a = circuit.input(0,1,2);
    // std::vector<uint64_t> coefficient = {double2fix<uint64_t>(1),
    //                                      double2fix<uint64_t>(2),
    //                                      double2fix<uint64_t>(3),
    //                                      double2fix<uint64_t>(4),
    //                                      double2fix<uint64_t>(5)};
    // auto b = circuit.poly2Trunc(a , 4 , coefficient);
    // auto c = circuit.output(b);
    // circuit.addEndPoint(c);
    // circuit.runOffline();
    // circuit.runOnline();

    //test ElemMultTrunc correctness
    // auto a = circuit.input(0,2,2);
    // auto b = circuit.input(1,2,2);
    // auto c = circuit.elemMultTrunc(a , b);
    // auto d = circuit.output(c);
    // circuit.addEndPoint(d);
    // circuit.runOffline();
    // circuit.runOnline();

    //test conv2D & conv2DTrunc correctness
    // size_t rows = 4;
    // size_t cols = 4;
    // auto input_image = circuit.input(0 , 3 * rows , cols);
    // //设置卷积参数
    // std::vector <Conv2DOp> Convop(1);
    // InitializeConv(Convop);
    // auto kernel1 = circuit.input(0, Convop[0].kernel_shape_[0] * Convop[0].kernel_shape_[1],
    //     Convop[0].kernel_shape_[2] * Convop[0].kernel_shape_[3]);
    // auto out = circuit.conv2DTrunc(input_image , kernel1 , Convop[0]);
    // auto c = circuit.output(out);
    // circuit.addEndPoint(c);
    // circuit.runOffline();
    // circuit.runOnline();

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

