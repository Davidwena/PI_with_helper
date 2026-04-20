#include "../../../src/utils/rand.h"
#include "../../../src/object/Party.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#include "../../../src/utils/vectorUtil.h"
#include "../../../src/utils/fixed_point.h"
#include <string>
#include "../../../src/protocols/Party/CircuitForParty.h"
#include "../../../src/utils/tensor.h"
#include <cstring>

using namespace ppml_with_hp;
using namespace std;

constexpr std::size_t fracBits = 12;
constexpr std::size_t lBits = 16;

// 当前代码要求：低到高排列
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

// 用 double 精确计算多项式值（低到高）
double evalPolyLowToHigh(double x, const std::vector<double>& coeff) {
    double ans = 0.0;
    double xp = 1.0;
    for(std::size_t i = 0; i < coeff.size(); i++) {
        ans += coeff[i] * xp;
        xp *= x;
    }
    return ans;
}

double decodeFixSigned(uint64_t v, std::size_t fracBits) {
    int64_t sv;
    std::memcpy(&sv, &v, sizeof(uint64_t));  // 按位解释，不做数值转换
    return static_cast<double>(sv) / static_cast<double>(1ULL << fracBits);
}

int main() {
    std::size_t numParties = 2;
    std::size_t myId = 0;
    std::size_t portBase = 5000;
    std::string taskName = "mult2Party";
    std::string setupPhase = "SetUp-" + std::to_string(myId);
    std::string setUpFilePath = "../../../src/taskData/" + taskName + "/" + setupPhase + ".txt";

    ppml_with_hp::Party<uint64_t> party1(numParties, myId, portBase, taskName, NetworkMode::WAN);

    party1.getKeyFromSetUpFile(setUpFilePath);
    party1.recvMacShare();
    party1.getAllPseudoRandFromFile("../../../src/taskData/" + taskName + "/" + setupPhase);

    CircuitForParty<uint64_t> circuit(party1);

    // =========================================================
    // 测试区间 [-7, 7]
    // 采样点数 N = 14001，对应步长 0.001
    // =========================================================
    const std::size_t N = 14001;
    const double L = -7.0;
    const double R = 7.0;
    const double step = (R - L) / static_cast<double>(N - 1);

    // 低到高排列：c0 + c1 x + c2 x^2 + ...
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

    // 构造测试输入
    std::vector<double> x_real(N);
    for(std::size_t i = 0; i < N; i++) {
        x_real[i] = L + step * static_cast<double>(i);
    }

    auto x_fix = double2fixVec<uint64_t>(x_real);
    a->setInput(x_fix);

    auto c = circuit.polyFixed(a, degree, coeff_b, truncBits);
    auto d = circuit.output(c);

    circuit.addEndPoint(d);
    circuit.runOffline();
    circuit.runOnline();

    auto o = d->getClear();

    // =========================================================
    // 误差统计
    // =========================================================
    double max_abs_err = 0.0;
    double mean_abs_err = 0.0;
    double rmse = 0.0;

    std::size_t max_err_idx = 0;

    for(std::size_t i = 0; i < N; i++) {
        double exact = evalPolyLowToHigh(x_real[i], realCoeff);
        double polyfixed_val = decodeFixWithLBits(o[i], fracBits, lBits);
        double abs_err = std::fabs(exact - polyfixed_val);

        mean_abs_err += abs_err;
        rmse += abs_err * abs_err;

        if(abs_err > max_abs_err) {
            max_abs_err = abs_err;
            max_err_idx = i;
        }
    }

    mean_abs_err /= static_cast<double>(N);
    rmse = std::sqrt(rmse / static_cast<double>(N));

    cout << std::fixed << std::setprecision(12);
    cout << "========================================" << endl;
    cout << "PolyFixed test on [-7, 7]" << endl;
    cout << "degree = " << degree << endl;
    cout << "fracBits = " << fracBits << endl;
    cout << "truncBits = " << truncBits << endl;
    cout << "N = " << N << endl;
    cout << "========================================" << endl;

    cout << "max_abs_err  = " << max_abs_err << endl;
    cout << "mean_abs_err = " << mean_abs_err << endl;
    cout << "rmse         = " << rmse << endl;

    cout << "----------------------------------------" << endl;
    cout << "Worst point:" << endl;
    cout << "x            = " << x_real[max_err_idx] << endl;
    cout << "exact        = " << evalPolyLowToHigh(x_real[max_err_idx], realCoeff) << endl;
    cout << "polyFixed    = " << decodeFixWithLBits(o[max_err_idx], fracBits, lBits) << endl;
    cout << "raw output   = " << o[max_err_idx] << endl;
    cout << "abs err      = "
         << std::fabs(evalPolyLowToHigh(x_real[max_err_idx], realCoeff) - decodeFixWithLBits(o[max_err_idx], fracBits, lBits))
         << endl;

    cout << "----------------------------------------" << endl;
    cout << "Sample points:" << endl;

    std::size_t sample_idx[] = {0, N / 4, N / 2, 3 * N / 4, N - 1};
    for(std::size_t t = 0; t < 5; t++) {
        std::size_t i = sample_idx[t];
        double exact = evalPolyLowToHigh(x_real[i], realCoeff);
        double polyfixed_val = decodeFixWithLBits(o[i], fracBits, lBits);
        double abs_err = std::fabs(exact - polyfixed_val);

        cout << "x = " << x_real[i]
             << ", exact = " << exact
             << ", polyFixed = " << polyfixed_val
             << ", raw = " << o[i]
             << ", abs_err = " << abs_err << endl;
    }

    return 0;
}