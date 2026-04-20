#ifndef PPML_HP_PROTOCOLS_CONV2DGATEFORPARTY_H
#define PPML_HP_PROTOCOLS_CONV2DGATEFORPARTY_H

#include "GateForParty.h"
#include "../../utils/tensor.h"

namespace ppml_with_hp {
    template <typename T>
    class Conv2DGateForParty : public GateForParty<T> {
        public:
           Conv2DGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                              const std::shared_ptr<GateForParty<T>>& p_input_y,
                              const Conv2DOp& op);

           void doRunOffline() override;

           void doRunOnline() override;

           Conv2DOp conv_op_;
    };

    template <typename T>
    Conv2DGateForParty<T>::
    Conv2DGateForParty(const std::shared_ptr<GateForParty<T>>& p_input_x,
                        const std::shared_ptr<GateForParty<T>>& p_input_y,
                        const Conv2DOp& op)
        : GateForParty<T>(p_input_x, p_input_y), conv_op_(op) {
        this->dim_row_ = conv_op_.compute_output_size();
        this->dim_col_ = 1;
    }

    template <typename T>
    void Conv2DGateForParty<T>::doRunOffline() {
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
        std::cout << "Conv2D预处理执行时间: " << duration.count() << " 毫秒" << std::endl;
    }

    template <typename T>
    void Conv2DGateForParty<T>::doRunOnline() {
        //添加同步点 --- 除pking 在线时间多计算100ms
        if(this->party.myId == 0){
            this->party.BroadcastExHp(static_cast<T>(0));
            this->party.template ReceiveFromAllExHp<T>();
        }
        else{
            this->party.template Receive<T>(0);
            this->party.Send(0 , static_cast<T>(0));
        }
        auto start = std::chrono::high_resolution_clock::now();
        auto mxmy = convolution(this->input_x_->maskedValueVec , this->input_y_->maskedValueVec , conv_op_);
        auto mxdeltayi = convolution(this->input_x_->maskedValueVec , this->input_y_->maskVecShr , conv_op_);
        auto mydeltaxi = convolution(this->input_x_->maskVecShr , this->input_y_->maskedValueVec , conv_op_);
        auto deltaxy2 = matrixScalar<T>(this->maskVecShr , 2);
        //compute [tmz] = maskedValueMacVecShr
        this->maskedValueMacVecShr = matrixScalar(mxmy , this->party.macShare);
        matrixSubtractAssign(this->maskedValueMacVecShr , convolution(this->input_x_->maskedValueVec,
            this->input_y_->maskMacVecShr , conv_op_));
        matrixSubtractAssign(this->maskedValueMacVecShr , convolution(this->input_x_->maskMacVecShr,
            this->input_y_->maskedValueVec , conv_op_));
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
        std::cout << "Conv2D在线执行时间: " << duration.count() << " 毫秒" << std::endl;

        // if(GateForParty<T>::verify()){
        //     std::cout<<"verify right"<<std::endl;
        // }
        // else{
        //     throw std::invalid_argument("verify fail");
        // }
    }
}



#endif