#ifndef PPML_HP_PROTOCOLS_CONV2DTRUNCGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_CONV2DTRUNCGATEFORPARTY_H

#include "Conv2DGateForParty.h"
#include "../../utils/fixed_point.h"

namespace ppml_with_hp {

    template <typename T>
    class Conv2DTruncGateForParty : public Conv2DGateForParty<T> {
        public:
            Conv2DTruncGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                    const std::shared_ptr<GateForParty<T>>& p_input_y,
                                    const Conv2DOp& op);

            void doRunOffline() override;

            void doRunOnline() override;

            std::vector<T> truncMaskVecShr;

            std::vector<T> truncMaskMacVecShr;
    };

    template <typename T>
    Conv2DTruncGateForParty<T>::
    Conv2DTruncGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                            const std::shared_ptr<GateForParty<T>>& p_input_y,
                            const Conv2DOp& op)
        : Conv2DGateForParty<T>(p_input_x, p_input_y, op) {}

    template <typename T>
    void Conv2DTruncGateForParty<T>::doRunOffline() {
        Conv2DGateForParty<T>::doRunOffline();
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
    void Conv2DTruncGateForParty<T>::doRunOnline() {
        Conv2DGateForParty<T>::doRunOnline();
        truncateClearVecInplace(this->maskedValueVec);
        std::copy(truncMaskVecShr.begin(), truncMaskVecShr.begin()+this->len, this->maskVecShr.begin());
        std::copy(truncMaskMacVecShr.begin(), truncMaskMacVecShr.begin()+this->len, this->maskMacVecShr.begin());
    }

} // namespace ppml_with_hp

#endif // PPML_HP_PROTOCOLS_CONV2DTRUNCGATEFORPARTY_H