#ifndef PPML_HP_PROTOCOLS_AVGPOOL2DGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_AVGPOOL2DGATEFORPARTY_H

#include "GateForParty.h"
#include "../../utils/tensor.h"
#include "../../utils/fixed_point.h"

//Pool gate need truncation

namespace ppml_with_hp {
    template <typename T>
    class AvgPool2DGateForParty : public GateForParty<T> {
        public:
            AvgPool2DGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                  const MaxPoolOp& op);

            void doRunOffline() override;

            void doRunOnline() override;

            MaxPoolOp maxPoolOp;

            T factor; // equals 1 / kernel_size
    };

    template <typename T>
    AvgPool2DGateForParty<T>::
    AvgPool2DGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                            const MaxPoolOp& op)
        : GateForParty<T>(p_input_x, nullptr),
          maxPoolOp(op),
          factor(double2fix<T>(1.0 / op.compute_kernel_size())) {
            this->dim_row_ = maxPoolOp.compute_output_size();
            this->dim_col_ = 1;
    }

    template <typename T>
    void AvgPool2DGateForParty<T>::doRunOffline() {
        //添加同步点 --- parties时间多计算100ms
        this->party.template Receive<T>(this->party.numParties);
        this->party.Send(this->party.numParties , static_cast<T>(0));

        auto start = std::chrono::high_resolution_clock::now();
        this->len = this->dim_row_ * this->dim_col_;
        this->maskVecShr.resize(this->len);
        this->maskMacVecShr.resize(this->len);
        std::vector<std::vector<T>> vec = this->party.partyShHpvVec(this->len);
        for(size_t i = 0 ; i < this->len; i++){
            this->maskVecShr[i] = vec[i][0];
            this->maskMacVecShr[i] = vec[i][1];
        }
        // truncMaskVecShr.resize(this->len);
        // truncMaskMacVecShr.resize(this->len);
        // std::vector<std::vector<T>> vec = this->party.partyShHpvVec(this->len);
        // for(size_t i = 0 ; i < this->len; i++) {
        //     truncMaskVecShr[i] = vec[i][0];
        //     truncMaskMacVecShr[i] = vec[i][1];
        // }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "AvgPool预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void AvgPool2DGateForParty<T>::doRunOnline() {
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

        this->maskedValueVec = sumPool(this->input_x_->maskedValueVec , maxPoolOp);
        matrixScalarAssign(this->maskedValueVec , factor);
        truncateClearVecInplace(this->maskedValueVec);
        // std::copy(truncMaskVecShr.begin(), truncMaskVecShr.begin()+this->len, this->maskVecShr.begin());
        // std::copy(truncMaskMacVecShr.begin(), truncMaskMacVecShr.begin()+this->len, this->maskMacVecShr.begin());
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "AvgPool在线执行时间: " << duration.count() << " 毫秒" << std::endl;

    }
}

#endif