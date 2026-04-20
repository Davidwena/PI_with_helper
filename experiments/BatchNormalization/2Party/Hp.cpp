#include "../../../src/utils/rand.h"
#include "../../../src/object/Hp.h"
#include <iostream>
#include "../../../src/utils/vectorUtil.h"
#include <string>
#include "../../../src/protocols/Hp/CircuitForHp.h"
#include "../../../src/utils/tensor.h"

using namespace std;
using namespace ppml_with_hp;

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

    //批处理层 需要计算均值和方差
    //输入为32 * 32的矩阵
    //均值计算时间近似于求内积，得到[[均值]]，进一步得到[[x - 均值]]
    //方差计算时间近似于elemMult [[x - 均值]] * [[x - 均值]] / n , 本地求和得到 [[方差]]
    //party 协商随机数rand
    //party 得到 [[方差]] ， 本地计算[[方差]] * rand + [[0]],发送给HP，HP聚合得到方差 * rand,计算得到1 / 根号（(方差 * rand)）,得到[[1 / 根号（(方差 * rand)）]]
    //party 得到 [[1 / 根号（(方差 * rand)）]] * 根号（rand） = [[1 / 根号（方差）]] 
    //再做一次elemMult得到 [[x - 均值]] * [[1 / 根号（方差）]] = [[x - 均值 / 根号（方差）]]
    
    //先测试matrixDot
    
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

