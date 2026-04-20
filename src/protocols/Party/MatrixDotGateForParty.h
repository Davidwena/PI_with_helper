#ifndef PPML_HP_PROTOCOLS_MATRIXDOTGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_MATRIXDOTGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp {
    template <typename T>
    class MatrixDotGateForParty : public GateForParty<T> {
        public:
            MatrixDotGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                  const std::shared_ptr<GateForParty<T>>& p_input_y);

            void doRunOffline() override;

            void doRunOnline() override;
    };

    template <typename T>
    MatrixDotGateForParty<T>::MatrixDotGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                                    const std::shared_ptr<GateForParty<T>>& p_input_y)
        : GateForParty<T>(p_input_x, p_input_y) {
        if (p_input_x->dim_row_ != p_input_y->dim_row_ || p_input_x->dim_col_ != p_input_y->dim_col_) {
            throw std::invalid_argument("The inputs of MatrixDot gate should have compatible dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = 1;
    }

    template <typename T>
    void MatrixDotGateForParty<T>::doRunOffline() {
        this->len = this->dim_row_ * this->dim_col_;
        this->maskVecShr.resize(this->len);
        this->maskMacVecShr.resize(this->len);
        std::vector<std::vector<T>> vec = this->party.partyShHpvVec(this->len);
        for(size_t i = 0 ; i < this->len; i++){
            this->maskVecShr[i] = vec[i][0];
            this->maskMacVecShr[i] = vec[i][1];
        }
    }

    template <typename T>
    void MatrixDotGateForParty<T>::doRunOnline() {
        auto mxmy = matrixElemMultiply(this->input_x_->maskedValueVec , this->input_y_->maskedValueVec);
        sumMatrixRowsAssign(mxmy, this->input_x_->dim_row_, this->input_x_->dim_col_);
        auto mxdeltayi = matrixElemMultiply(this->input_x_->maskedValueVec , this->input_y_->maskVecShr);
        sumMatrixRowsAssign(mxdeltayi, this->input_x_->dim_row_, this->input_x_->dim_col_);
        auto mydeltaxi = matrixElemMultiply(this->input_x_->maskVecShr , this->input_y_->maskedValueVec);
        sumMatrixRowsAssign(mydeltaxi, this->input_x_->dim_row_, this->input_x_->dim_col_);
        auto deltaxy2 = matrixScalar<T>(this->maskVecShr , 2);
        

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
        
    }
}

#endif