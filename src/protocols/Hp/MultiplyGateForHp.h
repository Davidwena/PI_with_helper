#ifndef PPML_HP_PROTOCOLS_MULTIPLYGATEFORHP_H
#define PPML_HP_PROTOCOLS_MULTIPLYGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp{
    template <typename T>
    class MultiplyGateForHp : public GateForHp<T> {
        public:

        MultiplyGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                          const std::shared_ptr<GateForHp<T>>& p_input_y);

        void doRunOffline() override;

        void doRunOnline() override;

        std::size_t dim_mid_;
    };

    template <typename T>
    MultiplyGateForHp<T>::MultiplyGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                            const std::shared_ptr<GateForHp<T>>& p_input_y)
        : GateForHp<T>(p_input_x,p_input_y), dim_mid_(p_input_x->dim_col_){
        if(p_input_x->dim_col_ != p_input_y->dim_row_){
            throw std::invalid_argument("The inputs of multiplication gate should have compatible dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_y->dim_col_;
    }

    template <typename T>
    void MultiplyGateForHp<T>::doRunOffline(){
        // this->maskVecShrForParty.resize(this->hp.numParties);
        // this->maskMacVecShrForParty.resize(this->hp.numParties);
        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty.resize(this->len);
        // for(size_t i = 0 ; i < this->hp.numParties ; i++){
        //     this->maskVecShrForParty[i].resize(this->len);
        //     this->maskMacVecShrForParty[i].resize(this->len);
        // }
        //delta z = delta x*y
        this->maskClearForParty = matrixMultiply(this->input_x_->maskClearForParty,
            this->input_y_->maskClearForParty,this->dim_row_,this->dim_mid_,this->dim_col_);
        // for(size_t i = 0 ; i < this->len ; i++){
        //     // std::vector<std::vector<T>> res = this->hp.hpShHpv(this->maskClearForParty[i]);
        //     this->hp.hpShHpv(this->maskClearForParty[i]);
        //     // for(size_t j = 0 ; j < this->hp.numParties ; j++){
        //     //     this->maskVecShrForParty[j][i] = res[j][0];
        //     //     this->maskMacVecShrForParty[j][i] = res[j][1];
        //     // }
        // }
        this->hp.hpShHpvVec(this->maskClearForParty);
    }

    template <typename T>
    void MultiplyGateForHp<T>::doRunOnline() {
        //do nothing
        // if(GateForHp<T>::verify()){
        //     this->hp.BroadcastExHp(static_cast<T>(1));
        // }
        // else{
        //     this->hp.BroadcastExHp(static_cast<T>(0));
        // }
    }
}

#endif