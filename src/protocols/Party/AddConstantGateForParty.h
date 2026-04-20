#ifndef PPML_HP_PROTOCOLS_ADDCONSTANTGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_ADDCONSTANTGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp {
    template <typename T> 
    class AddConstantGateForParty : public GateForParty<T> {
        public:

        AddConstantGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                const std::vector<T>& constant);
        
        void doRunOffline() override;

        void doRunOnline() override;

        std::vector<T> constant;
    };

    template <typename T>
    AddConstantGateForParty<T>::AddConstantGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                                        const std::vector<T>& constant)
        : GateForParty<T>(p_input_x, nullptr),constant(constant) {
        if(p_input_x->dim_row_ * p_input_x->dim_col_ != constant.size()) {
            throw std::invalid_argument("The constant of addConstant gate should have the same dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_; 
    }

    template <typename T>
    void AddConstantGateForParty<T>::doRunOffline() {
        this->maskVecShr = this->input_x_->maskVecShr;
        this->maskMacVecShr = this->input_x_->maskMacVecShr;
    }

    template <typename T>
    void AddConstantGateForParty<T>::doRunOnline() {
        this->maskedValueVec = matrixAdd(this->input_x_->maskedValueVec,constant);
    }
}

#endif