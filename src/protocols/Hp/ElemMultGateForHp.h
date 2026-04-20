#ifndef PPML_HP_PROTOCOLS_ELEMMULTGATEFORHP_H
#define PPML_HP_PROTOCOLS_ELEMMULTGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp {
    template <typename T>
    class ElemMultGateForHp : public GateForHp<T> {
        public:
            ElemMultGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                              const std::shared_ptr<GateForHp<T>>& p_input_y);

            void doRunOffline() override;

            void doRunOnline() override;
    };

    template <typename T>
    ElemMultGateForHp<T>::ElemMultGateForHp(const std::shared_ptr<GateForHp<T>>& p_input_x,
                                           const std::shared_ptr<GateForHp<T>>& p_input_y)
        : GateForHp<T>(p_input_x,p_input_y){
        if(p_input_x->dim_row_ != p_input_y->dim_row_ || 
           p_input_x->dim_col_ != p_input_y->dim_col_){
            throw std::invalid_argument("The inputs of ElemMultiplication gate should have compatible dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_y->dim_col_;
    }

    template <typename T>
    void ElemMultGateForHp<T>::doRunOffline(){
        //添加同步点
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();
        
        auto start = std::chrono::high_resolution_clock::now();
        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty.resize(this->len);
        //delta z = delta x*y
        this->maskClearForParty = matrixElemMultiply(this->input_x_->maskClearForParty,
            this->input_y_->maskClearForParty);
        this->hp.hpShHpvVec(this->maskClearForParty);
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "elemMult预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void ElemMultGateForHp<T>::doRunOnline() {
        //do nothing
        // if(GateForHp<T>::verify()){
        //     this->hp.BroadcastExHp(static_cast<T>(1));
        // }
        // else{
        //     this->hp.BroadcastExHp(static_cast<T>(0));
        // }
    }
}

#endif