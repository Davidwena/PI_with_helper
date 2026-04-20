#ifndef PPML_HP_PROTOCOLS_MULTTRUNCGATEFORHP_H
#define PPML_HP_PROTOCOLS_MULTTRUNCGATEFORHP_H

#include "MultiplyGateForHp.h"
#include "../../utils/fixed_point.h"

namespace ppml_with_hp{
    template <typename T>
    class MultTruncGateForHp : public MultiplyGateForHp<T> {
        public :

        MultTruncGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                           const std::shared_ptr<GateForHp<T>>& p_input_y);
        
        void doRunOffline() override;

        void doRunOnline() override;

        // std::vector<std::vector<T>> truncMaskVecShrForParty;
        // std::vector<std::vector<T>> truncMaskMacVecShrForParty;
        // std::vector<T> truncMaskClearForParty;
    };

    template <typename T>
    MultTruncGateForHp<T>::MultTruncGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                              const std::shared_ptr<GateForHp<T>>& p_input_y)
        : MultiplyGateForHp<T>(p_input_x,p_input_y){
    }

    template <typename T>
    void MultTruncGateForHp<T>::doRunOffline() {
        //添加同步点
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();
        
        auto start = std::chrono::high_resolution_clock::now();
        MultiplyGateForHp<T>::doRunOffline();
        // truncMaskVecShrForParty.resize(this->hp.numParties);
        // truncMaskMacVecShrForParty.resize(this->hp.numParties);
        this->len = this->dim_row_ * this->dim_col_;
        // for(size_t i = 0 ; i < this->hp.numParties ; i++){
        //     this->truncMaskVecShrForParty[i].resize(this->len);
        //     this->truncMaskMacVecShrForParty[i].resize(this->len);
        // }
        truncateClearVecInplace(this->maskClearForParty);
        //这里记得改成workbench
        this->hp.hpShHpvVec(this->maskClearForParty);
        // for(size_t i = 0 ; i < this->len ; i++){
        //     this->hp.hpShHpv(this->maskClearForParty[i]);
        //     // for(size_t j = 0 ; j < this->hp.numParties ; j++){
        //     //     this->truncMaskVecShrForParty[j][i] = res[j][0];
        //     //     this->truncMaskMacVecShrForParty[j][i] = res[j][1];
        //     // }
        // }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "MultTrunc预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void MultTruncGateForHp<T>::doRunOnline() {
        MultiplyGateForHp<T>::doRunOnline();
    }
}
#endif