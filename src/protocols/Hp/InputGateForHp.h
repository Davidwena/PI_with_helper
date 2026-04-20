#ifndef PPML_HP_PROTOCOLS_INPUTGATEFORHP_H
#define PPML_HP_PROTOCOLS_INPUTGATEFORHP_H

#include "GateForHp.h"

namespace ppml_with_hp{
    template <typename T>
    class InputGateForHp : public GateForHp<T> {
        public:

        InputGateForHp(Hp<T>& hp,std::size_t dimRow, std::size_t dimCol, std::size_t ownerId);
        
        void doRunOffline() override;

        void doRunOnline() override;

        std::size_t owner_id_;
    };

    template <typename T>
    InputGateForHp<T>::InputGateForHp(Hp<T>& hp,std::size_t dimRow, std::size_t dimCol, std::size_t ownerId)
    : GateForHp<T>(hp, dimRow, dimCol), owner_id_(ownerId) {}

    template <typename T>
    void InputGateForHp<T>::doRunOffline() {
        //设置同步点
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();

        auto start = std::chrono::high_resolution_clock::now();
        // this->maskVecShrForParty.resize(this->hp.numParties);
        // this->maskMacVecShrForParty.resize(this->hp.numParties);
        this->len = this->dim_row_ * this->dim_col_;
        this->maskClearForParty.resize(this->len);
        // for(size_t i = 0 ; i < this->hp.numParties ; i++){
        //     this->maskVecShrForParty[i].resize(this->len);
        //     this->maskMacVecShrForParty[i].resize(this->len);
        // }
        //hp需要存储每个参与方的每一个MacShare信息
        this->maskClearForParty = this->hp.hpShPdRandVec(owner_id_ , this->len);
        // for(size_t i = 0 ; i < this->len ; i++){
        //     this->maskClearForParty[i] = this->hp.hpShPdRand(owner_id_);
        //     // for(size_t j = 0 ; j < this->hp.numParties ; j++){
        //     //     this->maskVecShrForParty[j][i] = vec[j][0];
        //     //     this->maskMacVecShrForParty[j][i] = vec[j][1];
        //     //     // #ifdef PPML_HP_DEBUG_PREPROCESS
        //     //     // std::cout<<"maskVecShrForParty["<<j<<"]["<<i<<"] = "<<this->maskVecShrForParty[j][i]<<std::endl;
        //     //     // std::cout<<"maskMacVecShrForParty["<<j<<"]["<<i<<"] = "<<this->maskMacVecShrForParty[j][i]<<std::endl;
        //     //     // #endif
        //     // }
        // }
        //在input门结尾做一次同步
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "input预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void InputGateForHp<T>::doRunOnline() {
        //设置同步点
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();

        auto start = std::chrono::high_resolution_clock::now();
        auto qv = this->hp.template ReceiveVec<T>(owner_id_,this->len);
        this->hp.BroadcastVecExHp(qv);
        // #ifdef PPML_HP_DEBUG_PREPROCESS
        // for(size_t i = 0 ; i < len ; i++){
        //     std::cout<<"qv["<<i<<"] = "<<qv[i]<<std::endl;
        // }
        // #endif
        //在input门结尾做一次同步
        this->hp.BroadcastExHp(static_cast<T>(0));
        this->hp.template ReceiveFromAllExHp<T>();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "input在线执行时间: " << duration.count() << " 毫秒" << std::endl;
    }
}

#endif