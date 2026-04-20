#ifndef PPML_HP_PROTOCOLS_ADDGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_ADDGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp{
    template <typename T>
    class AddGateForParty : public GateForParty<T> {
        public:

        AddGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                        const std::shared_ptr<GateForParty<T>>& p_input_y);

        void doRunOffline() override;

        void doRunOnline() override;
    };

    template <typename T>
    AddGateForParty<T>::AddGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                  const std::shared_ptr<GateForParty<T>>& p_input_y) 
        : GateForParty<T>(p_input_x,p_input_y) {
        if (p_input_x->dim_row_ != p_input_y->dim_row_ ||
            p_input_x->dim_col_ != p_input_y->dim_col_) {
            throw std::invalid_argument("The inputs of addition gate should have the same dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_;
    }

    template <typename T>
    void AddGateForParty<T>::doRunOffline(){
        this->len = this->dim_row_ * this->dim_col_;
        this->maskVecShr.resize(this->len);
        this->maskMacVecShr.resize(this->len);
        this->maskVecShr = matrixAdd(this->input_x_->maskVecShr,this->input_y_->maskVecShr);
        this->maskMacVecShr = matrixAdd(this->input_x_->maskMacVecShr,this->input_y_->maskMacVecShr);
    }

    template <typename T>
    void AddGateForParty<T>::doRunOnline() {
        this->maskedValueVec = matrixAdd(this->input_x_->maskedValueVec,this->input_y_->maskedValueVec);
    }
}

#endif