#ifndef PPML_HP_PROTOCOLS_SUBTRACTGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_SUBTRACTGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp {
    template <typename T>
    class SubtractGateForParty : public GateForParty<T> {
        public:

        SubtractGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                             const std::shared_ptr<GateForParty<T>>& p_input_y);
        
        void doRunOffline() override;

        void doRunOnline() override;
    };

    template <typename T>
    SubtractGateForParty<T>::SubtractGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                                  const std::shared_ptr<GateForParty<T>>& p_input_y)
        : GateForParty<T>(p_input_x,p_input_y) {
        if (p_input_x->dim_row_ != p_input_y->dim_row_ ||
            p_input_x->dim_col_ != p_input_y->dim_col_) {
            throw std::invalid_argument("The inputs of subtract gate should have the same dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_;
    }

    template <typename T>
    void SubtractGateForParty<T>::doRunOffline() {
        this->len = this->dim_row_ * this->dim_col_;
        this->maskVecShr.resize(this->len);
        this->maskMacVecShr.resize(this->len);
        this->maskVecShr = matrixSubtract(this->input_x_->maskVecShr,this->input_y_->maskVecShr);
        this->maskMacVecShr = matrixSubtract(this->input_x_->maskMacVecShr,this->input_y_->maskMacVecShr);
    }

    template <typename T>
    void SubtractGateForParty<T>::doRunOnline() {
        this->maskedValueVec = matrixSubtract(this->input_x_->maskedValueVec,this->input_y_->maskedValueVec);
    }
}

#endif