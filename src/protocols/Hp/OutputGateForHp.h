#ifndef PPML_HP_PROTOCOLS_OUTPUTGATEFORHP_H
#define PPML_HP_PROTOCOLS_OUTPUTGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp {
    template <typename T>
    class OutputGateForHp : public GateForHp<T> {
        public:

        using ClearType = T;

        OutputGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x);

        void doRunOffline() override;

        void doRunOnline() override;
    };

    template <typename T>
    OutputGateForHp<T>::OutputGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x)
        : GateForHp<T>(p_input_x,nullptr) {
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_; 
    }

    template <typename T>
    void OutputGateForHp<T>::doRunOffline() {
        //do nothing
    }

    template <typename T>
    void OutputGateForHp<T>::doRunOnline() {
        this->hp.BroadcastVecExHp(this->input_x_->maskClearForParty);
    }
}

#endif