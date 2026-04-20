#ifndef PPML_HP_PROTOCOLS_MULTIPLYGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_MULTIPLYGATEFORPARTY_H

#include "GateForParty.h"

namespace ppml_with_hp{
    template <typename T>
    class MultiplyGateForParty : public GateForParty<T> {
        public: 

        MultiplyGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                             const std::shared_ptr<GateForParty<T>>& p_input_y);
        
        void doRunOffline() override;

        void doRunOnline() override;

        std::size_t dim_mid_;

    };

    template <typename T>
    MultiplyGateForParty<T>::MultiplyGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                                                  const std::shared_ptr<GateForParty<T>>& p_input_y)
        : GateForParty<T>(p_input_x,p_input_y), dim_mid_(p_input_x->dim_col_){
        if(p_input_x->dim_col_ != p_input_y->dim_row_){
            throw std::invalid_argument("The inputs of multiplication gate should have compatible dimensions");
        }
        this->dim_row_ = p_input_x->dim_row_;
        this->dim_col_ = p_input_y->dim_col_;
    }

    template <typename T>
    void MultiplyGateForParty<T>::doRunOffline(){
        this->len = this->dim_row_ * this->dim_col_;
        this->maskVecShr.resize(this->len);
        this->maskMacVecShr.resize(this->len);
        // for(size_t i = 0 ; i < this->len; i++){
        //     std::vector<T> vec = this->party.partyShHpv();
        //     this->maskVecShr[i] = vec[0];
        //     this->maskMacVecShr[i] = vec[1];
        // }
        std::vector<std::vector<T>> vec = this->party.partyShHpvVec(this->len);
        for(size_t i = 0 ; i < this->len; i++){
            this->maskVecShr[i] = vec[i][0];
            this->maskMacVecShr[i] = vec[i][1];
        }
    }

    template <typename T>
    void MultiplyGateForParty<T>::doRunOnline() {
        //设pking id = 0;
        //[tmz] = [mac]*mx*my-mx*[tdeltay]-my*[tdeltax]+2*[tdeltaxy] 
        //[mz]1 = mx*my - mx*[deltay]1 - my*[deltax]1 + 2*[deltaxy]1
        auto mxmy = matrixMultiply(this->input_x_->maskedValueVec , this->input_y_->maskedValueVec,
        this->dim_row_,this->dim_mid_,this->dim_col_);
        auto mxdeltayi = matrixMultiply(this->input_x_->maskedValueVec , this->input_y_->maskVecShr,
        this->dim_row_,this->dim_mid_,this->dim_col_);
        auto mydeltaxi = matrixMultiply(this->input_x_->maskVecShr , this->input_y_->maskedValueVec,
        this->dim_row_,this->dim_mid_,this->dim_col_);
        auto deltaxy2 = matrixScalar<T>(this->maskVecShr , 2);
        //compute [tmz] = maskedValueMacVecShr
        this->maskedValueMacVecShr.resize(this->len);
        this->maskedValueMacVecShr = matrixScalar(mxmy , this->party.macShare);
        matrixSubtractAssign(this->maskedValueMacVecShr , matrixMultiply(this->input_x_->maskedValueVec,
        this->input_y_->maskMacVecShr,this->dim_row_,this->dim_mid_,this->dim_col_));
        matrixSubtractAssign(this->maskedValueMacVecShr , matrixMultiply(this->input_x_->maskMacVecShr,
        this->input_y_->maskedValueVec,this->dim_row_,this->dim_mid_,this->dim_col_));
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
        // if(GateForParty<T>::verify()){
        //     std::cout<<"verify right"<<std::endl;
        // }
        // else{
        //     throw std::invalid_argument("verify fail");
        // }

    }
}

#endif