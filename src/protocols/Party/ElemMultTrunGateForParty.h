#ifndef PPML_HP_PROTOCOLS_ELEMMULTTRUNGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_ELEMMULTTRUNGATEFORPARTY_H

#include "ElemMultGateForParty.h"
#include "../../utils/fixed_point.h"

namespace ppml_with_hp {
    template <typename T>
    class ElemMultTrunGateForParty : public ElemMultGateForParty<T> {
        public:
            ElemMultTrunGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                     const std::shared_ptr<GateForParty<T>>& p_input_y);

            void doRunOffline() override;

            void doRunOnline() override;

            std::vector<T> truncMaskVecShr;

            std::vector<T> truncMaskMacVecShr;
    };

    template <typename T>
    ElemMultTrunGateForParty<T>::ElemMultTrunGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                                         const std::shared_ptr<GateForParty<T>>& p_input_y)
        : ElemMultGateForParty<T>(p_input_x, p_input_y) {
    }

    template <typename T>
    void ElemMultTrunGateForParty<T>::doRunOffline() {
        ElemMultGateForParty<T>::doRunOffline();
        this->len = this->dim_row_ * this->dim_col_;
        truncMaskVecShr.resize(this->len);
        truncMaskMacVecShr.resize(this->len);
        std::vector<std::vector<T>> vec = this->party.partyShHpvVec(this->len);
        for(size_t i = 0 ; i < this->len; i++) {
            truncMaskVecShr[i] = vec[i][0];
            truncMaskMacVecShr[i] = vec[i][1];
        }
    }

    template <typename T>
    void ElemMultTrunGateForParty<T>::doRunOnline() {
        ElemMultGateForParty<T>::doRunOnline();
        //mz' = mz / 2^d
        truncateClearVecInplace(this->maskedValueVec);
        std::copy(truncMaskVecShr.begin(), truncMaskVecShr.begin()+this->len, this->maskVecShr.begin());
        std::copy(truncMaskMacVecShr.begin(), truncMaskMacVecShr.begin()+this->len, this->maskMacVecShr.begin());
    }
}

#endif