#ifndef PPML_HP_PROTOCOLS_CONV2DGATEFORHP_H
#define PPML_HP_PROTOCOLS_CONV2DGATEFORHP_H

#include "GateForHp.h"
#include "../../utils/tensor.h"

namespace ppml_with_hp {

    template <typename T>
    class Conv2DGateForHp : public GateForHp<T> {
        public:
            Conv2DGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                            const std::shared_ptr<GateForHp<T>>& p_input_y,
                            const Conv2DOp& op);

            void doRunOffline() override;

            void doRunOnline() override;

            Conv2DOp conv_op_;
    };

    template <typename T>
    Conv2DGateForHp<T>::
    Conv2DGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                    const std::shared_ptr<GateForHp<T>>& p_input_y,
                    const Conv2DOp& op)
        : GateForHp<T>(p_input_x, p_input_y), conv_op_(op) {
        this->dim_row_ = conv_op_.compute_output_size();
        this->dim_col_ = 1;
    }

    template <typename T>
    void Conv2DGateForHp<T>::doRunOffline() {
        //添加同步点
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();

        auto start = std::chrono::high_resolution_clock::now();
        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty = convolution(this->input_x_->maskClearForParty , this->input_y_->maskClearForParty , conv_op_);
        this->hp.hpShHpvVec(this->maskClearForParty);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Conv2D预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void Conv2DGateForHp<T>::doRunOnline() {
        // if(GateForHp<T>::verify()){
        //     this->hp.BroadcastExHp(static_cast<T>(1));
        // }
        // else{
        //     this->hp.BroadcastExHp(static_cast<T>(0));
        // }
    }
}

#endif // CONV2DGATEFORHP_H