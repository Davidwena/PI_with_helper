#ifndef PPML_HP_PROTOCOLS_MULTSCALARGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_MULTSCALARGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp {
    template <typename T>
    class MultScalarGateForParty : public GateForParty<T> {
        public:
            MultScalarGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                   std::vector<T> scalar);

            void doRunOffline() override;

            void doRunOnline() override;

            std::vector<T> scalar;
    };

    template <typename T>
    MultScalarGateForParty<T>::MultScalarGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x, std::vector<T> scalar)
        : GateForParty<T>(p_input_x, nullptr), scalar(scalar) {

        if(p_input_x->dim_row_ * p_input_x->dim_col_ != scalar.size()){
            throw std::invalid_argument("The size of scalar should be equal to the size of input_x");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_;
    }
    template <typename T>
    void MultScalarGateForParty<T>::doRunOffline() {
        this->maskVecShr = matrixElemMultiply(this->input_x_->maskVecShr, scalar);
        this->maskMacVecShr = matrixElemMultiply(this->input_x_->maskMacVecShr, scalar);
    }
    template <typename T>
    void MultScalarGateForParty<T>::doRunOnline() {
        this->maskedValueVec = matrixElemMultiply(this->input_x_->maskedValueVec, scalar);
    }
} // namespace ppml_with_hp
#endif // MULTSCALARGATEFORPARTY_H