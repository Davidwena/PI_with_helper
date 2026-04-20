#include "../../../src/utils/rand.h"
#include "../../../src/object/Hp.h"
#include <iostream>
#include "../../../src/utils/vectorUtil.h"
#include "../../../src/utils/fixed_point.h"
#include <string>
#include "../../../src/protocols/Hp/CircuitForHp.h"
#include "../../../src/utils/tensor.h"

using namespace std;
using namespace ppml_with_hp;

constexpr std::size_t fracBits = 12;

// 代码当前语义：coeff[0] 是常数项，按低次到高次排列
template <typename T>
std::vector<T> buildPolyFixedCoeffLowToHigh(const std::vector<double>& realCoeff, std::size_t k) {
    if(realCoeff.size() != k + 1) {
        throw std::invalid_argument("realCoeff.size() must be k + 1");
    }

    // a_i = floor(a'_i * 2^d)
    std::vector<T> a_int = double2fixVec<T>(realCoeff);

    // b_i = a_i * 2^{d(k-i)}
    std::vector<T> b_coeff(k + 1);
    for(std::size_t i = 0; i <= k; i++) {
        b_coeff[i] = a_int[i] << (fracBits * (k - i));
    }
    return b_coeff;
}

int main() {
    std::size_t numParties = 2;
    std::size_t myId = numParties;
    std::size_t portBase = 5000;
    std::string taskName = "mult2Party";
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    std::string setUpFilePath = "../../../src/taskData/" + taskName + "/" + setupPhase + ".txt";

    ppml_with_hp::Hp<uint64_t> hp(numParties, myId, portBase, taskName, NetworkMode::WAN);

    hp.getKeyFromSetUpFile(setUpFilePath);
    hp.genMacKeyAndDis();
    hp.getAllPseudoRandFromFile("../../../src/taskData/" + taskName + "/" + setupPhase);

    CircuitForHp<uint64_t> circuit(hp);

    // 区间 [-7, 7] 上取样
    const std::size_t N = 14001; // 步长 0.001

    // 低到高：c0 + c1 x + c2 x^2 + c3 x^3 + c4 x^4
    std::vector<double> realCoeff = {
        0.410888671875,
        0.499755859375,
        0.116943359375,
        0.0,
        -0.0009765625
    };

    std::size_t degree = 4;
    std::vector<uint64_t> coeff_b = buildPolyFixedCoeffLowToHigh<uint64_t>(realCoeff, degree);

    // 注意：这里建议使用 fracBits * degree
    std::size_t truncBits = fracBits * degree;

    auto a = circuit.input(0, 1, N);
    auto c = circuit.polyFixed(a, degree, coeff_b, truncBits);
    auto d = circuit.output(c);

    circuit.addEndPoint(d);
    circuit.runOffline();
    circuit.runOnline();

    return 0;
}