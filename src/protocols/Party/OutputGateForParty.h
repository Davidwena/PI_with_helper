#ifndef PPML_HP_PROTOCOLS_OUTPUTGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_OUTPUTGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp{
    template <typename T>
    class OutputGateForParty : public GateForParty<T> {
        public:

        using ClearType = T;
        
        OutputGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x);

        void doRunOffline() override;

        void doRunOnline() override;

        std::vector<T> getClear() const;

        std::vector<T> maskClearVec;
    };

    template <typename T>
    OutputGateForParty<T>::OutputGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x)
        : GateForParty<T>(p_input_x,nullptr) {
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_x->dim_col_;
    }

    template <typename T>
    void OutputGateForParty<T>::doRunOffline() {
        //do nothing
    }

    template <typename T>
    void OutputGateForParty<T>::doRunOnline() {
        std::cout<<"123"<<std::endl;
        maskClearVec = this->party.template ReceiveVec<T>(this->party.numParties,this->dim_row_ * this->dim_col_);
        std::cout<<"1234"<<std::endl;
        
        // maskClearVec.clear();
        // maskClearVec.shrink_to_fit();
    }

    //no truncation
    template <typename T>
    std::vector<T> OutputGateForParty<T>::getClear() const {
        std::cout<<"234"<<std::endl;
        auto output = matrixSubtract(this->input_x_->maskedValueVec,maskClearVec);
        return std::vector<T>(output.begin(), output.end());
        std::cout<<"345"<<std::endl;
    }

}
#endif