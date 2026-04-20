#ifndef PPML_HP_PROTOCOLS_CIRCUITFORHP_H
#define PPML_HP_PROTOCOLS_CIRCUITFORHP_H

#include <memory>
#include <vector>
#include "../../utils/Timer.h"
#include "GateForHp.h"
#include "InputGateForHp.h"
#include "AddGateForHp.h"
#include "AddConstantGateForHp.h"
#include "SubtractGateForHp.h"
#include "MultiplyGateForHp.h"
#include "ElemMultGateForHp.h"
#include "ElemMultTrunGateForHp.h"
#include "OutputGateForHp.h"
#include "MultTruncGateForHp.h"
#include "PolyGateForHp.h"
#include "PolyTrunGateForHp.h"
#include "Poly2GateForHp.h"
#include "Poly2TrunGateForHp.h"
#include "Conv2DGateForHp.h"
#include "Conv2DTruncGateForHp.h"
#include "AvgPool2DGateForHp.h"
#include "AsteriskElemMultGateForHp.h"
#include "MatrixDotGateForHp.h"
#include "MultScalarGateForHp.h"
#include "Poly2RealGateForHp.h"
#include "PolyFixedGateForHp.h"

namespace ppml_with_hp{
    template <typename T>
    class CircuitForHp{
        public:
        explicit CircuitForHp(Hp<T>& hp) : hp(hp) {}
        void addEndPoint(const std::shared_ptr<GateForHp<T>>& gate);
        void runOffline();
        void printStats();
        void runOnline();

        std::shared_ptr<InputGateForHp<T>>
        input(std::size_t ownerId, std::size_t dimRow, std::size_t dimCol);

        std::shared_ptr<AddGateForHp<T>>
        add(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<AddConstantGateForHp<T>>
        addConstant(const std::shared_ptr<GateForHp<T>>& input, const std::vector<T>& constant);

        std::shared_ptr<SubtractGateForHp<T>>
        subtract(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<MultiplyGateForHp<T>>
        multiply(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<ElemMultGateForHp<T>>
        elemMult(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<ElemMultTrunGateForHp<T>>
        elemMultTrunc(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<OutputGateForHp<T>>
        output(const std::shared_ptr<GateForHp<T>>& input);

        std::shared_ptr<MultTruncGateForHp<T>>
        multTrunc(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<PolyGateForHp<T>>
        poly(const std::shared_ptr<GateForHp<T>>& input, size_t k, std::vector<T> coefficient);

        std::shared_ptr<Poly2GateForHp<T>>
        poly2(const std::shared_ptr<GateForHp<T>>& input, size_t k, std::vector<T> coefficient);

        std::shared_ptr<PolyTrunGateForHp<T>>
        polyTrunc(const std::shared_ptr<GateForHp<T>>& input, size_t k, std::vector<T> coefficient);

        std::shared_ptr<Poly2TrunGateForHp<T>>
        poly2Trunc(const std::shared_ptr<GateForHp<T>>& input, size_t k, std::vector<T> coefficient);

        std::shared_ptr<Conv2DGateForHp<T>>
        conv2D(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y, const Conv2DOp& op);
        
        std::shared_ptr<Conv2DTruncGateForHp<T>>
        conv2DTrunc(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y, const Conv2DOp& op);
        
        std::shared_ptr<AvgPool2DGateForHp<T>>
        avgPool2D(const std::shared_ptr<GateForHp<T>>& input_x, const MaxPoolOp& op);

        std::shared_ptr<AsteriskElemMultGateForHp<T>>
        asteriskElemMult(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<MatrixDotGateForHp<T>>
        matrixDot(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y);

        std::shared_ptr<MultScalarGateForHp<T>>
        multScalar(const std::shared_ptr<GateForHp<T>>& input_x, std::vector<T> scalar);

        std::shared_ptr<Poly2RealGateForHp<T>>
        poly2Real(const std::shared_ptr<GateForHp<T>>& input, size_t k, std::vector<T> coefficient);

        std::shared_ptr<PolyFixedGateForHp<T>>
        polyFixed(const std::shared_ptr<GateForHp<T>>& p_input_x, std::size_t k, std::vector<T> coefficient, std::size_t truncBits);
        
        std::vector<std::shared_ptr<GateForHp<T>>> gates;
        std::vector<std::shared_ptr<GateForHp<T>>> endPoints;
        Hp<T>& hp;
        Timer timer;
    };

    template <typename T>
    void CircuitForHp<T>::addEndPoint(const std::shared_ptr<GateForHp<T>>& gate) {
        endPoints.push_back(gate);
    }

    template <typename T>
    void CircuitForHp<T>::runOffline(){
        for (const auto& gate : endPoints) {
            gate->runOffline();
        }
    }

    template <typename T>
    void CircuitForHp<T>::runOnline() {
        timer.start();
        for (const auto& gate : endPoints) {
            gate->runOnline();
        }
        timer.stop();
    }

    template <typename T>
    void CircuitForHp<T>::printStats() {
        std::cout
            << "Spent " << timer.elapsed() << " ms\n"
            << "Sent " << hp.bytesSent << " bytes\n";
    }

    template <typename T>
    std::shared_ptr<InputGateForHp<T>> CircuitForHp<T>::
    input(std::size_t ownerId, std::size_t dimRow, std::size_t dimCol) {
        auto gate = std::make_shared<InputGateForHp<T>>(hp, dimRow, dimCol, ownerId);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AddGateForHp<T>> CircuitForHp<T>::
    add(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<AddGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AddConstantGateForHp<T>> CircuitForHp<T>::
    addConstant(const std::shared_ptr<GateForHp<T>>& input, const std::vector<T>& constant) {
        auto gate = std::make_shared<AddConstantGateForHp<T>>(input, constant);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<SubtractGateForHp<T>> CircuitForHp<T>::
    subtract(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<SubtractGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MultiplyGateForHp<T>> CircuitForHp<T>::
    multiply(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<MultiplyGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<ElemMultGateForHp<T>> CircuitForHp<T>::
    elemMult(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<ElemMultGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<ElemMultTrunGateForHp<T>> CircuitForHp<T>::
    elemMultTrunc(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<ElemMultTrunGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }
    
    template <typename T>
    std::shared_ptr<OutputGateForHp<T>> CircuitForHp<T>::
    output(const std::shared_ptr<GateForHp<T>>& input) {
        auto gate = std::make_shared<OutputGateForHp<T>>(input);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MultTruncGateForHp<T>> CircuitForHp<T>::
    multTrunc(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<MultTruncGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<PolyGateForHp<T>> CircuitForHp<T>::
    poly(const std::shared_ptr<GateForHp<T>>& input , size_t k , std::vector<T> coefficient) {
        auto gate = std::make_shared<PolyGateForHp<T>>(input , k , coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<PolyTrunGateForHp<T>> CircuitForHp<T>::
    polyTrunc(const std::shared_ptr<GateForHp<T>>& input , size_t k , std::vector<T> coefficient) {
        auto gate = std::make_shared<PolyTrunGateForHp<T>>(input , k , coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Poly2GateForHp<T>> CircuitForHp<T>::
    poly2(const std::shared_ptr<GateForHp<T>>& input , size_t k , std::vector<T> coefficient) {
        auto gate = std::make_shared<Poly2GateForHp<T>>(input , k , coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Poly2TrunGateForHp<T>> CircuitForHp<T>::
    poly2Trunc(const std::shared_ptr<GateForHp<T>>& input , size_t k , std::vector<T> coefficient) {
        auto gate = std::make_shared<Poly2TrunGateForHp<T>>(input , k , coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Conv2DGateForHp<T>> CircuitForHp<T>::
    conv2D(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y, const Conv2DOp& op) {
        auto gate = std::make_shared<Conv2DGateForHp<T>>(input_x, input_y, op);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Conv2DTruncGateForHp<T>> CircuitForHp<T>::
    conv2DTrunc(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y, const Conv2DOp& op) {
        auto gate = std::make_shared<Conv2DTruncGateForHp<T>>(input_x, input_y, op);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AvgPool2DGateForHp<T>> CircuitForHp<T>::
    avgPool2D(const std::shared_ptr<GateForHp<T>>& input_x, const MaxPoolOp& op) {
        auto gate = std::make_shared<AvgPool2DGateForHp<T>>(input_x, op);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AsteriskElemMultGateForHp<T>> CircuitForHp<T>::
    asteriskElemMult(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<AsteriskElemMultGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MatrixDotGateForHp<T>> CircuitForHp<T>::
    matrixDot(const std::shared_ptr<GateForHp<T>>& input_x, const std::shared_ptr<GateForHp<T>>& input_y) {
        auto gate = std::make_shared<MatrixDotGateForHp<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MultScalarGateForHp<T>> CircuitForHp<T>::
    multScalar(const std::shared_ptr<GateForHp<T>>& input_x, std::vector<T> scalar) {
        auto gate = std::make_shared<MultScalarGateForHp<T>>(input_x, scalar);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Poly2RealGateForHp<T>> CircuitForHp<T>::
    poly2Real(const std::shared_ptr<GateForHp<T>>& input, size_t k, std::vector<T> coefficient) {
        auto gate = std::make_shared<Poly2RealGateForHp<T>>(input, k, coefficient);
        gates.push_back(gate);
        return gate;
    }


    template <typename T>
    std::shared_ptr<PolyFixedGateForHp<T>> CircuitForHp<T>::
    polyFixed(const std::shared_ptr<GateForHp<T>>& p_input_x, std::size_t k, std::vector<T> coefficient, std::size_t truncBits) {
        auto gate = std::make_shared<PolyFixedGateForHp<T>>(p_input_x, k, coefficient, truncBits);
        gates.push_back(gate);
        return gate;
    }
}

#endif