#ifndef PPML_HP_PROTOCOLS_ELEMMULTGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_ELEMMULTGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp {
    template <typename T>
    class ElemMultGateForParty : public GateForParty<T> {
        public:
            ElemMultGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                 const std::shared_ptr<GateForParty<T>>& p_input_y);

            void doRunOffline() override;

            void doRunOnline() override;
    };

    template <typename T>
    ElemMultGateForParty<T>::ElemMultGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                                 const std::shared_ptr<GateForParty<T>>& p_input_y)
        : GateForParty<T>(p_input_x,p_input_y) {
        if(p_input_x->dim_row_ != p_input_y->dim_row_ || 
           p_input_x->dim_col_ != p_input_y->dim_col_) {
            throw std::invalid_argument("The inputs of ElemMultiplication gate should have compatible dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_y->dim_col_;
    }

    template <typename T>
    void ElemMultGateForParty<T>::doRunOffline() {
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
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "elemMult预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void ElemMultGateForParty<T>::doRunOnline() {
        //添加同步点 --- 除pking 在线时间多计算100ms
        // if(this->party.myId == 0){
        //     this->party.BroadcastExHp(static_cast<T>(0));
        //     this->party.template ReceiveFromAllExHp<T>();
        // }
        // else{
        //     this->party.template Receive<T>(0);
        //     this->party.Send(0 , static_cast<T>(0));
        // }

        auto start = std::chrono::high_resolution_clock::now();
        //设pking id = 0;
        //[tmz] = [mac]*mx*my-mx*[tdeltay]-my*[tdeltax]+2*[tdeltaxy] 
        //[mz]1 = mx*my - mx*[deltay]1 - my*[deltax]1 + 2*[deltaxy]1
        auto mxmy = matrixElemMultiply(this->input_x_->maskedValueVec , this->input_y_->maskedValueVec);
        auto mxdeltayi = matrixElemMultiply(this->input_x_->maskedValueVec , this->input_y_->maskVecShr);
        auto mydeltaxi = matrixElemMultiply(this->input_x_->maskVecShr , this->input_y_->maskedValueVec);
        auto deltaxy2 = matrixScalar<T>(this->maskVecShr , 2);
        //compute [tmz] = maskedValueMacVecShr
        this->maskedValueMacVecShr = matrixScalar(mxmy , this->party.macShare);
        matrixSubtractAssign(this->maskedValueMacVecShr , matrixElemMultiply(this->input_x_->maskedValueVec,
            this->input_y_->maskMacVecShr));
        matrixSubtractAssign(this->maskedValueMacVecShr , matrixElemMultiply(this->input_x_->maskMacVecShr,
            this->input_y_->maskedValueVec));
        matrixAddAssign(this->maskedValueMacVecShr, matrixScalar<T>(this->maskMacVecShr , 2));
        if(this->party.myId == 0){
            this->maskedValueVec = matrixSubtract(mxmy , mxdeltayi);
            matrixSubtractAssign(this->maskedValueVec , mydeltaxi);
            matrixAddAssign(this->maskedValueVec , deltaxy2);
            auto allMzShri = this->party.template ReceiveVecFromAllExHp<T>(this->len);
            //reconstruct mz and mz is stored in this->maskedValueVec
            for(size_t i = 1 ; i < allMzShri.size() ; i++){
                matrixAddAssign(this->maskedValueVec,allMzShri[i]);
            }
            this->party.BroadcastVecExHp(this->maskedValueVec);
        }
        else{
            auto mzShri = matrixSubtract(deltaxy2 , mxdeltayi);
            matrixSubtractAssign(mzShri , mydeltaxi);
            this->party.SendVec(0 , mzShri);
            this->maskedValueVec = this->party.template ReceiveVec<T>(0 , this->len);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "elemMult在线执行时间: " << duration.count() << " 毫秒" << std::endl;
        // if(GateForParty<T>::verify()){
        //     std::cout<<"verify right"<<std::endl;
        // }
        // else{
        //     throw std::invalid_argument("verify fail");
        // }
    }
}

#endif