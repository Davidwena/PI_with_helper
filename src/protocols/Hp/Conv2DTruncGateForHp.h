#ifndef PPML_HP_PROTOCOLS_CONV2DTRUNCGATEFORHP_H
#define PPML_HP_PROTOCOLS_CONV2DTRUNCGATEFORHP_H

#include "Conv2DGateForHp.h"
#include "../../utils/fixed_point.h"

namespace ppml_with_hp {

    template <typename T>
    class Conv2DTruncGateForHp : public Conv2DGateForHp<T> {
        public:
            Conv2DTruncGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                 const std::shared_ptr<GateForHp<T>>& p_input_y,
                                 const Conv2DOp& op);

            void doRunOffline() override;

            void doRunOnline() override;
    };

    template <typename T>
    Conv2DTruncGateForHp<T>::
    Conv2DTruncGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                         const std::shared_ptr<GateForHp<T>>& p_input_y,
                         const Conv2DOp& op)
        : Conv2DGateForHp<T>(p_input_x, p_input_y, op) {}

    template <typename T>
    void Conv2DTruncGateForHp<T>::doRunOffline() {
        Conv2DGateForHp<T>::doRunOffline();
        this->len = this->dim_row_ * this->dim_col_;
        truncateClearVecInplace(this->maskClearForParty);
        this->hp.hpShHpvVec(this->maskClearForParty);
    }

    template <typename T>
    void Conv2DTruncGateForHp<T>::doRunOnline() {
        Conv2DGateForHp<T>::doRunOnline();
    }

} // namespace ppml_with_hp

#endif