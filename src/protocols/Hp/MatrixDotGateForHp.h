#ifndef PPML_HP_PROTOCOLS_MATRIXDOTGATEFORHP_H
#define PPML_HP_PROTOCOLS_MATRIXDOTGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp {
    template <typename T>
    class MatrixDotGateForHp : public GateForHp<T> {
        public:
            MatrixDotGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                               const std::shared_ptr<GateForHp<T>>& p_input_y);

            void doRunOffline() override;

            void doRunOnline() override;
    };

    template <typename T>
    MatrixDotGateForHp<T>::MatrixDotGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                              const std::shared_ptr<GateForHp<T>>& p_input_y)
        : GateForHp<T>(p_input_x, p_input_y) {
        if (p_input_x->dim_row_ != p_input_y->dim_row_ || p_input_x->dim_col_ != p_input_y->dim_col_) {
            throw std::invalid_argument("The inputs of MatrixDot gate should have compatible dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = 1;
    }

    template <typename T>
    void MatrixDotGateForHp<T>::doRunOffline() {
        //计算 delta y
        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty.resize(this->len);
        matrixElemMultiplyAssign(this->input_x_->maskClearForParty, this->input_y_->maskClearForParty);
        this->maskClearForParty = sumMatrixRows(this->input_x_->maskClearForParty, this->input_x_->dim_row_, this->input_x_->dim_col_);
        this->hp.hpShHpvVec(this->maskClearForParty);
    }

    template <typename T>
    void MatrixDotGateForHp<T>::doRunOnline() {
        //do nothing
    }
}

#endif