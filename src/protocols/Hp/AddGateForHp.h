#ifndef PPML_HP_PROTOCOLS_ADDGATEFORHP_H
#define PPML_HP_PROTOCOLS_ADDGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp{
    template <typename T>
    class AddGateForHp : public GateForHp<T> {
        public:

        AddGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                     const std::shared_ptr<GateForHp<T>>& p_input_y);

        void doRunOffline() override;

        void doRunOnline() override;
    };

    template <typename T>
    AddGateForHp<T>::AddGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                  const std::shared_ptr<GateForHp<T>>& p_input_y) 
        : GateForHp<T>(p_input_x,p_input_y) {
        if (p_input_x->dim_row_ != p_input_y->dim_row_ ||
            p_input_x->dim_col_ != p_input_y->dim_col_) {
            throw std::invalid_argument("The inputs of addition gate should have the same dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_;
    }

    template <typename T>
    void AddGateForHp<T>::doRunOffline(){
        // this->maskVecShrForParty.resize(this->hp.numParties);
        // this->maskMacVecShrForParty.resize(this->hp.numParties);
        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty.resize(this->len);
        // for(size_t i = 0 ; i < this->hp.numParties ; i++){
        //     this->maskVecShrForParty[i].resize(this->len);
        //     this->maskMacVecShrForParty[i].resize(this->len);
        // }
        // for(size_t i = 0 ; i < this->hp.numParties ; i++){
        //     this->maskVecShrForParty[i] = matrixAdd(this->input_x_->maskVecShrForParty[i],
        //     this->input_y_->maskVecShrForParty[i]);
        //     this->maskMacVecShrForParty[i] = matrixAdd(this->input_x_->maskMacVecShrForParty[i],
        //     this->input_y_->maskMacVecShrForParty[i]);
        // }
        this->maskClearForParty = matrixAdd(this->input_x_->maskClearForParty,
        this->input_y_->maskClearForParty);
    }

    template <typename T>
    void AddGateForHp<T>::doRunOnline() {
        //do nothing
    }
}

#endif