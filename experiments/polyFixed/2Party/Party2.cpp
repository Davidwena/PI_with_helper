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

constexpr std::size_t fracBits = 12;

template <typename T>
std::vector<T> buildPolyFixedCoeffLowToHigh(const std::vector<double>& realCoeff, std::size_t k) {
    if(realCoeff.size() != k + 1) {
        throw std::invalid_argument("realCoeff.size() must be k + 1");
    }

    std::vector<T> a_int = double2fixVec<T>(realCoeff);

    std::vector<T> b_coeff(k + 1);
    for(std::size_t i = 0; i <= k; i++) {
        b_coeff[i] = a_int[i] << (fracBits * (k - i));
    }
    return b_coeff;
}

int main() {
    std::size_t numParties = 2;
    std::size_t myId = 1;
    std::size_t portBase = 5000;
    std::string taskName = "mult2Party";
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    std::string setUpFilePath = "../../../src/taskData/" + taskName + "/" + setupPhase + ".txt";

    ppml_with_hp::Party<uint64_t> party2(numParties, myId, portBase, taskName, NetworkMode::WAN);

    party2.getKeyFromSetUpFile(setUpFilePath);
    party2.recvMacShare();
    party2.getAllPseudoRandFromFile("../../../src/taskData/" + taskName + "/" + setupPhase);

    CircuitForParty<uint64_t> circuit(party2);

    const std::size_t N = 14001;

    std::vector<double> realCoeff = {
        0.410888671875,
        0.499755859375,
        0.116943359375,
        0.0,
        -0.0009765625
    };

    std::size_t degree = 4;
    std::vector<uint64_t> coeff_b = buildPolyFixedCoeffLowToHigh<uint64_t>(realCoeff, degree);
    std::size_t truncBits = fracBits * degree;

    auto a = circuit.input(0, 1, N);
    auto c = circuit.polyFixed(a, degree, coeff_b, truncBits);
    auto d = circuit.output(c);

    circuit.addEndPoint(d);
    circuit.runOffline();
    circuit.runOnline();

    return 0;
}