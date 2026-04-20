#ifndef PPML_HP_PROTOCOLS_GATEFORPARTY_H
#define PPML_HP_PROTOCOLS_GATEFORPARTY_H

#include <memory>
#include <numeric>
#include "../../object/Party.h"
#include "../../utils/linear_algebra.h"

namespace ppml_with_hp{
    template <typename T>
    class GateForParty{
        public :
        //输入门构造函数
        GateForParty(Party<T>& party, std::size_t dimRow, std::size_t dimCol);

        GateForParty(const std::shared_ptr<GateForParty>& p_input_x, const std::shared_ptr<GateForParty>& p_input_y);

        virtual ~GateForParty() = default;

        void runOffline();

        void runOnline();

        bool verify();

        virtual void doRunOffline() = 0;

        virtual void doRunOnline() = 0;

        bool evaluated_offline_ = false;
        bool evaluated_online_ = false;

        Party<T>& party;

        // 每一个门的输入导线
        std::shared_ptr<GateForParty> input_x_{};
        std::shared_ptr<GateForParty> input_y_{};

        //表示每一根导线上的矩阵大小
        std::size_t dim_row_ = 1;
        std::size_t dim_col_ = 1;
        //len = dim_row_*dim_col_
        std::size_t len = 1;

        //存储每个门的输出lambda,输入是一个矩阵，输出也是一个矩阵（适用于单输出门）
        //这里我们将矩阵展平存储
        //[delta v]
        std::vector<T> maskVecShr;
        //[t delta v]
        std::vector<T> maskMacVecShr;
        //mv
        std::vector<T> maskedValueVec;
        //[tmv] is used to verify , only some gates use it
        std::vector<T> maskedValueMacVecShr;
    };

    template <typename T>
    GateForParty<T>::GateForParty(Party<T>& party, std::size_t dimRow, std::size_t dimCol) : party(party), dim_row_(dimRow),dim_col_(dimCol) {}

    template <typename T>
    GateForParty<T>::GateForParty(const std::shared_ptr<GateForParty>& p_input_x, const std::shared_ptr<GateForParty>& p_input_y)
    : party(p_input_x->party),input_x_(p_input_x),input_y_(p_input_y) {}

    template <typename T>
    void GateForParty<T>::runOffline() {
        if(this->evaluated_offline_)
            return;
        if(input_x_ && !input_x_->evaluated_offline_)
            input_x_->runOffline();
        if (input_y_ && !input_y_->evaluated_offline_)
            input_y_->runOffline();
        
        this->doRunOffline();

        this->evaluated_offline_ = true;
    }

    template <typename T>
    void GateForParty<T>::runOnline() {
        if(this->evaluated_online_)
            return;
        if(input_x_ && !input_x_->evaluated_online_)
            input_x_->runOnline();
        if (input_y_ && !input_y_->evaluated_online_)
            input_y_->runOnline();
        
        this->doRunOnline();

        this->evaluated_online_ = true;
    }

    template <typename T>
    bool GateForParty<T>::verify() {
        auto kver = this->party.template Receive<T>(this->party.numParties);
        //[wmzj] = [tmzj] - mzj * [mac]
        auto omigamzj = matrixSubtract(this->maskedValueMacVecShr , 
        matrixScalar<T>(this->maskedValueVec , this->party.macShare));
        std::vector<T> rand(this->maskedValueMacVecShr.size(),0);
        for(size_t i = 0 ; i < rand.size() ; i++){
            rand[i] = getPseudoRand<T>(kver , i);
        }
        //compute [wz] = sum(rand[j] * omigamzj)
        auto omigamzVecShr = matrixElemMultiply(rand , omigamzj);
        auto omigamzShr = std::accumulate(omigamzVecShr.begin(),omigamzVecShr.end(),0ULL);
        this->party.Send(this->party.numParties , omigamzShr);
        return static_cast<bool>(this->party.template Receive<T>(this->party.numParties));
    }
}
#endif