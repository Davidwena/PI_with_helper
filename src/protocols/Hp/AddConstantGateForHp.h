#ifndef PPML_HP_PROTOCOLS_ADDCONSTANTGATEFORHP_H
#define PPML_HP_PROTOCOLS_ADDCONSTANTGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp {
    template <typename T>
    class AddConstantGateForHp : public GateForHp<T> {
        public:

        AddConstantGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                             const std::vector<T>& constant);
        
        void doRunOffline() override;

        void doRunOnline() override;

        std::vector<T> constant;
    };

    template <typename T>
    AddConstantGateForHp<T>::AddConstantGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                                  const std::vector<T>& constant)
        : GateForHp<T>(p_input_x,nullptr),constant(constant) {
        if(p_input_x->dim_row_ * p_input_x->dim_col_ != constant.size()) {
            throw std::invalid_argument("The constant of addConstant gate should have the same dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_; 
    }

    template <typename T>
    void AddConstantGateForHp<T>::doRunOffline() {
        this->maskClearForParty = this->input_x_->maskClearForParty;
    }

    template <typename T>
    void AddConstantGateForHp<T>::doRunOnline() {
        //do nothing
    }
}

#endif