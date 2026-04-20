#ifndef PPML_HP_PROTOCOLS_MULTSCALARGATEFORHP_H
#define PPML_HP_PROTOCOLS_MULTSCALARGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp {
    template <typename T>
    class MultScalarGateForHp : public GateForHp<T> {
        public:
            MultScalarGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                std::vector<T> scalar);

            void doRunOffline() override;

            void doRunOnline() override;

            std::vector<T> scalar;
    };

    template <typename T>
    MultScalarGateForHp<T>::MultScalarGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x, std::vector<T> scalar)
        : GateForHp<T>(p_input_x, nullptr), scalar(scalar) {
        if(p_input_x->dim_row_ * p_input_x->dim_col_ != scalar.size()){
            throw std::invalid_argument("The size of scalar should be equal to the size of input_x");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_;
    }

    template <typename T>
    void MultScalarGateForHp<T>::doRunOffline() {
        this->maskClearForParty = matrixElemMultiply(this->input_x_->maskClearForParty, scalar);
    }

    template <typename T>
    void MultScalarGateForHp<T>::doRunOnline() {
        //do nothing
    }

    
}

#endif // MULTSCALARGATEFORHP_H