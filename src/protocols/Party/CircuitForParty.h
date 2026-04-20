#ifndef PPML_HP_PROTOCOLS_CIRCUITFORPARTY_H
#define PPML_HP_PROTOCOLS_CIRCUITFORPARTY_H

#include <memory>
#include <vector>
#include "../../object/Party.h"
#include "../../utils/Timer.h"
#include "GateForParty.h"
#include "InputGateForParty.h"
#include "AddGateForParty.h"
#include "AddConstantGateForParty.h"
#include "SubtractGateForParty.h"
#include "MultiplyGateForParty.h"
#include "ElemMultGateForParty.h"
#include "ElemMultTrunGateForParty.h"
#include "OutputGateForParty.h"
#include "MultTruncGateForParty.h"
#include "PolyGateForParty.h"
#include "PolyTrunGateForParty.h"
#include "Poly2GateForParty.h"
#include "Poly2TrunGateForParty.h"
#include "Conv2DGateForParty.h"
#include "Conv2DTruncGateForParty.h"
#include "AvgPool2DGateForParty.h"
#include "AsteriskElemMultGateForParty.h"
#include "MatrixDotGateForParty.h"
#include "MultScalarGateForParty.h"
#include "Poly2RealGateForParty.h"
#include "PolyFixedGateForParty.h"

namespace ppml_with_hp{
    template <typename T>
    class CircuitForParty{
        public:
        explicit CircuitForParty(Party<T>& party) : party(party) {}
        void addEndPoint(const std::shared_ptr<GateForParty<T>>& gate);
        void runOffline();
        void printStats();
        void runOnline();

        std::shared_ptr<InputGateForParty<T>>
        input(std::size_t ownerId, std::size_t dimRow, std::size_t dimCol);

        std::shared_ptr<AddGateForParty<T>>
        add(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);

        std::shared_ptr<AddConstantGateForParty<T>>
        addConstant(const std::shared_ptr<GateForParty<T>>& input, const std::vector<T>& constant);

        std::shared_ptr<SubtractGateForParty<T>>
        subtract(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);

        std::shared_ptr<MultiplyGateForParty<T>>
        multiply(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);

        std::shared_ptr<ElemMultGateForParty<T>>
        elemMult(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);
        
        std::shared_ptr<ElemMultTrunGateForParty<T>>
        elemMultTrunc(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);
        
        std::shared_ptr<OutputGateForParty<T>>
        output(const std::shared_ptr<GateForParty<T>>& input);

        std::shared_ptr<MultTruncGateForParty<T>>
        multTrunc(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);

        std::shared_ptr<PolyGateForParty<T>>
        poly(const std::shared_ptr<GateForParty<T>>& input , size_t k , std::vector<T> coefficient);

        std::shared_ptr<PolyTrunGateForParty<T>>
        polyTrunc(const std::shared_ptr<GateForParty<T>>& input , size_t k , std::vector<T> coefficient);

        std::shared_ptr<Poly2GateForParty<T>>
        poly2(const std::shared_ptr<GateForParty<T>>& input , size_t k , std::vector<T> coefficient);

        std::shared_ptr<Poly2TrunGateForParty<T>>
        poly2Trunc(const std::shared_ptr<GateForParty<T>>& input, size_t k, std::vector<T> coefficient);

        std::shared_ptr<Conv2DGateForParty<T>>
        conv2D(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y, const Conv2DOp& op);
        
        std::shared_ptr<Conv2DTruncGateForParty<T>>
        conv2DTrunc(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y, const Conv2DOp& op);
        
        std::shared_ptr<AvgPool2DGateForParty<T>>
        avgPool2D(const std::shared_ptr<GateForParty<T>>& input_x, const MaxPoolOp& op);

        std::shared_ptr<AsteriskElemMultGateForParty<T>>
        asteriskElemMult(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);

        std::shared_ptr<MatrixDotGateForParty<T>>
        matrixDot(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y);

        std::shared_ptr<MultScalarGateForParty<T>>
        multScalar(const std::shared_ptr<GateForParty<T>>& input_x, std::vector<T> scalar);

        std::shared_ptr<Poly2RealGateForParty<T>>
        poly2Real(const std::shared_ptr<GateForParty<T>>& input_x, size_t k, std::vector<T> coefficient);

        std::shared_ptr<PolyFixedGateForParty<T>>
        polyFixed(const std::shared_ptr<GateForParty<T>>& p_input_x, std::size_t k, std::vector<T> coefficient, std::size_t truncBits);

        
        std::vector<std::shared_ptr<GateForParty<T>>> gates;
        std::vector<std::shared_ptr<GateForParty<T>>> endPoints;
        Party<T>& party;
        Timer timer;
    };

    template <typename T>
    void CircuitForParty<T>::addEndPoint(const std::shared_ptr<GateForParty<T>>& gate) {
        endPoints.push_back(gate);
    }

    template <typename T>
    void CircuitForParty<T>::runOffline(){
        for (const auto& gate : endPoints) {
            gate->runOffline();
        }
    }

    template <typename T>
    void CircuitForParty<T>::runOnline(){
        timer.start();
        for (const auto& gate : endPoints) {
            gate->runOnline();
        }
        timer.stop();
        printStats();
    }

    template <typename T>
    void CircuitForParty<T>::printStats() {
        std::cout
            << "Spent " << timer.elapsed() << " ms\n"
            << "Sent " << party.bytesSent << " bytes\n";
    }

    template <typename T>
    std::shared_ptr<InputGateForParty<T>> CircuitForParty<T>::
    input(std::size_t ownerId, std::size_t dimRow, std::size_t dimCol) {
        auto gate = std::make_shared<InputGateForParty<T>>(party, dimRow, dimCol, ownerId);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AddGateForParty<T>> CircuitForParty<T>::
    add(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<AddGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AddConstantGateForParty<T>> CircuitForParty<T>::
    addConstant(const std::shared_ptr<GateForParty<T>>& input, const std::vector<T>& constant) {
        auto gate = std::make_shared<AddConstantGateForParty<T>>(input, constant);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<SubtractGateForParty<T>> CircuitForParty<T>::
    subtract(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<SubtractGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MultiplyGateForParty<T>> CircuitForParty<T>::
    multiply(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<MultiplyGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<ElemMultGateForParty<T>> CircuitForParty<T>::
    elemMult(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<ElemMultGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<ElemMultTrunGateForParty<T>> CircuitForParty<T>::
    elemMultTrunc(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<ElemMultTrunGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }
    
    template <typename T>
    std::shared_ptr<OutputGateForParty<T>> CircuitForParty<T>::
    output(const std::shared_ptr<GateForParty<T>>& input) {
        auto gate = std::make_shared<OutputGateForParty<T>>(input);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MultTruncGateForParty<T>> CircuitForParty<T>::
    multTrunc(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<MultTruncGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<PolyGateForParty<T>> CircuitForParty<T>::
    poly(const std::shared_ptr<GateForParty<T>>& input , size_t k , std::vector<T> coefficient) {
        auto gate = std::make_shared<PolyGateForParty<T>>(input , k ,coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<PolyTrunGateForParty<T>> CircuitForParty<T>::
    polyTrunc(const std::shared_ptr<GateForParty<T>>& input , size_t k , std::vector<T> coefficient) {
        auto gate = std::make_shared<PolyTrunGateForParty<T>>(input , k ,coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Poly2GateForParty<T>> CircuitForParty<T>::
    poly2(const std::shared_ptr<GateForParty<T>>& input , size_t k , std::vector<T> coefficient) {
        auto gate = std::make_shared<Poly2GateForParty<T>>(input , k ,coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Poly2TrunGateForParty<T>> CircuitForParty<T>::
    poly2Trunc(const std::shared_ptr<GateForParty<T>>& input, size_t k, std::vector<T> coefficient) {
        auto gate = std::make_shared<Poly2TrunGateForParty<T>>(input , k ,coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Conv2DGateForParty<T>> CircuitForParty<T>::
    conv2D(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y, const Conv2DOp& op) {
        auto gate = std::make_shared<Conv2DGateForParty<T>>(input_x, input_y, op);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Conv2DTruncGateForParty<T>> CircuitForParty<T>::
    conv2DTrunc(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y, const Conv2DOp& op) {
        auto gate = std::make_shared<Conv2DTruncGateForParty<T>>(input_x, input_y, op);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AvgPool2DGateForParty<T>> CircuitForParty<T>::
    avgPool2D(const std::shared_ptr<GateForParty<T>>& input_x, const MaxPoolOp& op) {
        auto gate = std::make_shared<AvgPool2DGateForParty<T>>(input_x, op);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<AsteriskElemMultGateForParty<T>> CircuitForParty<T>::
    asteriskElemMult(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<AsteriskElemMultGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MatrixDotGateForParty<T>> CircuitForParty<T>::
    matrixDot(const std::shared_ptr<GateForParty<T>>& input_x, const std::shared_ptr<GateForParty<T>>& input_y) {
        auto gate = std::make_shared<MatrixDotGateForParty<T>>(input_x, input_y);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<MultScalarGateForParty<T>> CircuitForParty<T>::
    multScalar(const std::shared_ptr<GateForParty<T>>& input_x, std::vector<T> scalar) {
        auto gate = std::make_shared<MultScalarGateForParty<T>>(input_x, scalar);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<Poly2RealGateForParty<T>> CircuitForParty<T>::
    poly2Real(const std::shared_ptr<GateForParty<T>>& input_x, size_t k, std::vector<T> coefficient) {
        auto gate = std::make_shared<Poly2RealGateForParty<T>>(input_x, k, coefficient);
        gates.push_back(gate);
        return gate;
    }

    template <typename T>
    std::shared_ptr<PolyFixedGateForParty<T>> CircuitForParty<T>::
    polyFixed(const std::shared_ptr<GateForParty<T>>& p_input_x, std::size_t k, std::vector<T> coefficient, std::size_t truncBits) {
        auto gate = std::make_shared<PolyFixedGateForParty<T>>(p_input_x, k, coefficient, truncBits);
        gates.push_back(gate);
        return gate;
    }
}

#endif