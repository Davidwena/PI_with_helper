#ifndef PPML_HP_PROTOCOLS_MULTTRUNCGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_MULTTRUNCGATEFORPARTY_H

#include "MultiplyGateForParty.h"
#include "../../utils/fixed_point.h"

namespace ppml_with_hp {
    template <typename T>
    class MultTruncGateForParty : public MultiplyGateForParty<T> {
        public :

        MultTruncGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                              const std::shared_ptr<GateForParty<T>>& p_input_y);

        void doRunOffline() override;

        void doRunOnline() override;

        std::vector<T> truncMaskVecShr;

        std::vector<T> truncMaskMacVecShr;
    };

    template <typename T>
    MultTruncGateForParty<T>::MultTruncGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                                    const std::shared_ptr<GateForParty<T>>& p_input_y)
        : MultiplyGateForParty<T>(p_input_x,p_input_y){
    }

    template <typename T>
    void MultTruncGateForParty<T>::doRunOffline() {
        //添加同步点 --- parties时间多计算100ms
        this->party.template Receive<T>(this->party.numParties);
        this->party.Send(this->party.numParties , static_cast<T>(0));

        auto start = std::chrono::high_resolution_clock::now();
        MultiplyGateForParty<T>::doRunOffline();
        this->len = this->dim_row_ * this->dim_col_;
        truncMaskVecShr.resize(this->len);
        truncMaskMacVecShr.resize(this->len);
        std::vector<std::vector<T>> vec = this->party.partyShHpvVec(this->len);
        for(size_t i = 0 ; i < this->len; i++) {
            truncMaskVecShr[i] = vec[i][0];
            truncMaskMacVecShr[i] = vec[i][1];
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "MultTrunc预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void MultTruncGateForParty<T>::doRunOnline() {
        //添加同步点 --- 除pking 在线时间多计算100ms
        if(this->party.myId == 0){
            this->party.BroadcastExHp(static_cast<T>(0));
            this->party.template ReceiveFromAllExHp<T>();
        }
        else{
            this->party.template Receive<T>(0);
            this->party.Send(0 , static_cast<T>(0));
        }

        auto start = std::chrono::high_resolution_clock::now();
        MultiplyGateForParty<T>::doRunOnline();
        //mz' = mz / 2^d
        truncateClearVecInplace(this->maskedValueVec);
        std::copy(truncMaskVecShr.begin(), truncMaskVecShr.begin()+this->len, this->maskVecShr.begin());
        std::copy(truncMaskMacVecShr.begin(), truncMaskMacVecShr.begin()+this->len, this->maskMacVecShr.begin());
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "MultTrunc在线执行时间: " << duration.count() << " 毫秒" << std::endl;
    }
}

#endif