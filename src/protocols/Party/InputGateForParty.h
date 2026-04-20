#ifndef PPML_HP_PROTOCOLS_INPUTGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_INPUTGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp{
    template <typename T>
    class InputGateForParty : public GateForParty<T> {
        public:
        
        using ClearType = T;

        InputGateForParty(Party<T>& party,std::size_t dimRow, std::size_t dimCol, std::size_t ownerId);
        
        void doRunOffline() override;

        void doRunOnline() override;

        void setInput(const std::vector<ClearType>& input_value);

        std::size_t owner_id_;

        std::vector<T> input_value_;
        
        //仅输入方存在该值 , 非输入方该值为0
        std::vector<T> maskClearVec;
    };

    template <typename T>
    InputGateForParty<T>::InputGateForParty(Party<T>& party,std::size_t dimRow, std::size_t dimCol, std::size_t ownerId)
    : GateForParty<T>(party, dimRow, dimCol), owner_id_(ownerId) {}

    template <typename T>
    void InputGateForParty<T>::doRunOffline() {
        //设置同步点
        this->party.template Receive<T>(this->party.numParties);
        this->party.Send(this->party.numParties , static_cast<T>(0));

        auto start = std::chrono::high_resolution_clock::now();

        this->len = this->dim_row_ * this->dim_col_;
        this->maskVecShr.resize(this->len);
        this->maskMacVecShr.resize(this->len);
        this->maskClearVec.resize(this->len);
        auto [res , vVec] = this->party.partyShPdRandVec(owner_id_ , this->len);
        this->maskClearVec = vVec;
        for(size_t i = 0 ; i < this->len ; i++) {
            this->maskVecShr[i] = res[i * 2];
            this->maskMacVecShr[i] = res[i * 2 + 1];
        }
        // for(size_t i = 0 ; i < this->len; i++){
        //     auto [res,v] = this->party.partyShPdRand(owner_id_);
        //     this->maskVecShr[i] = res[0];
        //     this->maskMacVecShr[i] = res[1];
        //     this->maskClearVec[i] = v;
        //     // #ifdef PPML_HP_DEBUG_PREPROCESS
        //     // std::cout<<"maskVecShr["<<i<<"] = "<<this->maskVecShr[i]<<std::endl;
        //     // std::cout<<"maskMacVecShr["<<i<<"] = "<<this->maskMacVecShr[i]<<std::endl;
        //     // #endif
        // }
        //在input门结尾做一次同步
        //添加同步点 --- parties时间多计算100ms
        this->party.template Receive<T>(this->party.numParties);
        this->party.Send(this->party.numParties , static_cast<T>(0));

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "input预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    //设置输入门的输入
    template <typename T>
    void InputGateForParty<T>::setInput(const std::vector<ClearType>& input_value) {
        if(this->party.myId != owner_id_)
            throw std::logic_error("Not the owner of input gate, cannot set input");
        if(input_value.size() != this->dim_row_ * this->dim_col_)
            throw std::invalid_argument("Input vector and gate doesn't match in size");
        
        input_value_ = std::vector<T>(input_value.begin(), input_value.end());
    }

    template <typename T>
    void InputGateForParty<T>::doRunOnline() {
        //设置同步点
        this->party.template Receive<T>(this->party.numParties);
        this->party.Send(this->party.numParties , static_cast<T>(0));

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<T> r(this->len);
        this->maskedValueVec.resize(this->len);
        for(size_t i = 0 ; i < this->len ; i++){
            r[i] = this->party.kpPseudoRand[0];
            this->party.ctrAllParty++;
            // r[i] = this->party.kpPseudoRand[this->party.ctrAllParty++];
            // r[i] = getPseudoRand<T>(this->party.kp,this->party.ctrAllParty++);
        }
        if(this->party.myId == owner_id_){
            //qv = v + deltav + r
            auto qv = matrixAdd(input_value_,maskClearVec);
            matrixAddAssign(qv,r);
            //send to Hp
            this->party.SendVec(this->party.numParties, qv);
        }
        this->maskedValueVec = this->party.template ReceiveVec<T>(this->party.numParties, this->len);
        // #ifdef PPML_HP_DEBUG_PREPROCESS
        // for(size_t i = 0 ; i < this->this->len ; i++){
        //     std::cout<<"qv["<<i<<"] = "<<this->maskedValueVec[i]<<std::endl;
        // }
        // #endif
        matrixSubtractAssign(this->maskedValueVec, r);
        // #ifdef PPML_HP_DEBUG_PREPROCESS
        // for(size_t i = 0 ; i < this->len ; i++){
        //     std::cout<<"mv["<<i<<"] = "<<this->maskedValueVec[i]<<std::endl;
        // }
        // #endif
        
        //在input门结尾做一次同步
        //添加同步点 --- parties时间多计算100ms
        this->party.template Receive<T>(this->party.numParties);
        this->party.Send(this->party.numParties , static_cast<T>(0));
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "input在线执行时间: " << duration.count() << " 毫秒" << std::endl;

        input_value_.clear();
        input_value_.shrink_to_fit();
        maskClearVec.clear();
        maskClearVec.shrink_to_fit();
    }
}

#endif