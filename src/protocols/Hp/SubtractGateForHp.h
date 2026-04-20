#ifndef PPML_HP_PROTOCOLS_SUBTRACTGATEFORHP_H
#define PPML_HP_PROTOCOLS_SUBTRACTGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp {
    template <typename T>
    class SubtractGateForHp : public GateForHp<T> {
        public :

        SubtractGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                          const std::shared_ptr<GateForHp<T>>& p_input_y);

        void doRunOffline() override;

        void doRunOnline() override;
    };

    template <typename T>
    SubtractGateForHp<T>::SubtractGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                            const std::shared_ptr<GateForHp<T>>& p_input_y)
        : GateForHp<T>(p_input_x,p_input_y){
        if (p_input_x->dim_row_ != p_input_y->dim_row_ ||
            p_input_x->dim_col_ != p_input_y->dim_col_) {
            throw std::invalid_argument("The inputs of addition gate should have the same dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_;
    }

    template <typename T>
    void SubtractGateForHp<T>::doRunOffline(){
        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty.resize(this->len);
        this->maskClearForParty = matrixSubtract(this->input_x_->maskClearForParty,
        this->input_y_->maskClearForParty);
    }

    template <typename T>
    void SubtractGateForHp<T>::doRunOnline() {
        //do nothing
    }
}

#endif