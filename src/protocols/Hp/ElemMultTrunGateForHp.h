#ifndef PPML_HP_PROTOCOLS_ELEMMULTTRUNGATEFORHP_H
#define PPML_HP_PROTOCOLS_ELEMMULTTRUNGATEFORHP_H

#include "ElemMultGateForHp.h"

namespace ppml_with_hp {
    template <typename T>
    class ElemMultTrunGateForHp : public ElemMultGateForHp<T> {
        public:
            ElemMultTrunGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                  const std::shared_ptr<GateForHp<T>>& p_input_y);

            void doRunOffline() override;

            void doRunOnline() override;
    };

    template <typename T>
    ElemMultTrunGateForHp<T>::ElemMultTrunGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                                   const std::shared_ptr<GateForHp<T>>& p_input_y)
        : ElemMultGateForHp<T>(p_input_x, p_input_y) {
    }

    template <typename T>
    void ElemMultTrunGateForHp<T>::doRunOffline() {
        ElemMultGateForHp<T>::doRunOffline();
        truncateClearVecInplace(this->maskClearForParty);
        this->hp.hpShHpvVec(this->maskClearForParty);
    }

    template <typename T>
    void ElemMultTrunGateForHp<T>::doRunOnline() {
        ElemMultGateForHp<T>::doRunOnline();
    }

} // namespace ppml_with_hp

#endif
