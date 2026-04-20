#ifndef PPML_HP_PROTOCOLS_GATEFORHP_H
#define PPML_HP_PROTOCOLS_GATEFORHP_H

#include <memory>
#include <numeric>
#include "../../object/Hp.h"
#include "../../utils/linear_algebra.h"

namespace ppml_with_hp{
    template <typename T>
    class GateForHp{
        public :
        //单输入门构造函数
        GateForHp(Hp<T>& hp, std::size_t dimRow, std::size_t dimCol);

        GateForHp(const std::shared_ptr<GateForHp>& p_input_x, const std::shared_ptr<GateForHp>& p_input_y);

        virtual ~GateForHp() = default;

        void runOffline();

        void runOnline();

        bool verify();

        virtual void doRunOffline() = 0;

        virtual void doRunOnline() = 0;

        bool evaluated_offline_ = false;
        bool evaluated_online_ = false;

        Hp<T>& hp;

        // The input wires of the gate
        std::shared_ptr<GateForHp> input_x_{};
        std::shared_ptr<GateForHp> input_y_{};

        // A gate actually holds a matrix, not a single value
        // The values are stored in a flat std::vector, so we store the dimensions of the matrix
        std::size_t dim_row_ = 1;
        std::size_t dim_col_ = 1;
        //len = dim_row_*dim_col_
        std::size_t len = 1;

        //Hp存储每个参与方的输出lambda,每个参与方有二维输出
        //第一维表示partyId
        // std::vector<std::vector<T>> maskVecShrForParty;
        // std::vector<std::vector<T>> maskMacVecShrForParty;
        std::vector<T> maskClearForParty;
    };

    template <typename T>
    GateForHp<T>::GateForHp(Hp<T>& hp, std::size_t dimRow, std::size_t dimCol) : hp(hp), dim_row_(dimRow),dim_col_(dimCol) {}

    template <typename T>
    GateForHp<T>::GateForHp(const std::shared_ptr<GateForHp>& p_input_x, const std::shared_ptr<GateForHp>& p_input_y)
    : hp(p_input_x->hp),input_x_(p_input_x),input_y_(p_input_y) {}

    template <typename T>
    void GateForHp<T>::runOffline() {
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
    void GateForHp<T>::runOnline() {
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
    bool GateForHp<T>::verify() {
        this->hp.BroadcastExHp(getRand<T>()) ;
        auto omigamzShr = this->hp.template ReceiveFromAllExHp<T>();
        auto omigamz = std::accumulate(omigamzShr.begin(),omigamzShr.end(),0ULL);
        return !static_cast<bool>(omigamz);
    }
}
#endif