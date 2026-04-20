#ifndef PPML_HP_PROTOCOLS_AVGPOOL2DGATEFORHP_H
#define PPML_HP_PROTOCOLS_AVGPOOL2DGATEFORHP_H

#include "GateForHp.h"
#include "../../utils/tensor.h"
#include "../../utils/fixed_point.h"

//Pool gate need truncation

namespace ppml_with_hp {

    template <typename T>
    class AvgPool2DGateForHp : public GateForHp<T> {
        public:
            AvgPool2DGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                               const MaxPoolOp& op);

            void doRunOffline() override;

            void doRunOnline() override;

            MaxPoolOp maxPoolOp;

            T factor; // equals 1 / kernel_size
    };

    template <typename T>
    AvgPool2DGateForHp<T>::
    AvgPool2DGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                        const MaxPoolOp& op)
        : GateForHp<T>(p_input_x, nullptr),
          maxPoolOp(op),
          factor(double2fix<T>(1.0 / op.compute_kernel_size())) {
            this->dim_row_ = maxPoolOp.compute_output_size();
            this->dim_col_ = 1;
    }

    template <typename T>
    void AvgPool2DGateForHp<T>::doRunOffline() {
        //添加同步点
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();

        auto start = std::chrono::high_resolution_clock::now();

        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty = sumPool(this->input_x_->maskClearForParty , maxPoolOp);
        matrixScalarAssign(this->maskClearForParty , factor);
        // this->hp.hpShHpvVec(this->maskClearForParty);
        truncateClearVecInplace(this->maskClearForParty);
        this->hp.hpShHpvVec(this->maskClearForParty);

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "AvgPool预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void AvgPool2DGateForHp<T>::doRunOnline() {
        //do nothing
    }
}

#endif