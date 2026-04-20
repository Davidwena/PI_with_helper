#ifndef PPML_HP_PARTY_H
#define PPML_HP_PARTY_H

#include <vector>
#include <iostream>
#include "../utils/rand.h"
#include "../utils/uint128_io.h"
#include "PartyBase.h"
#include "../utils/vectorUtil.h"

namespace ppml_with_hp{

template <std::integral T>
class Party : public PartyBase{
public:
    //继承构造函数
    using PartyBase::PartyBase;
    // Hp类不能被复制构造函数以及复制赋值运算符
    Party(const Party&) = delete;
    Party& operator=(const Party&) = delete;
    Party(std::size_t n,std::size_t myId,std::size_t portBase,std::string taskName,NetworkMode mode = NetworkMode::LAN, T ctrAllParty = 0, T ctrP2Hp = 0) 
    : PartyBase(n,myId,portBase,taskName,mode),ctrAllParty(ctrAllParty),ctrP2Hp(ctrP2Hp) {}
    //定义Hp拥有的秘钥
    T kAll,ki,kp,macShare;
    std::vector<T> kAllPseudoRand, kiPseudoRand, kpPseudoRand;
    //ctrAllParty:party之间的公共计数器  ctrP2Hp:party和HP之间的计数器
    T ctrAllParty,ctrP2Hp; 
    bool genKp();
    //接收MacShare
    void recvMacShare();
    //Pi执行shPdRand
    std::pair<std::vector<T>,T> partyShPdRand(size_t idPd);
    //Pi执行shPdRand workbench
    std::pair<std::vector<T>,std::vector<T>> partyShPdRandVec(size_t idPd , size_t len);
    //Pi执行shHpRand
    std::vector<T> partyShHpRand();
    //Pi执行shHpv
    std::vector<T> partyShHpv();
    //Pi执行shHpv workbench
    std::vector<std::vector<T>> partyShHpvVec(size_t len);
    //Pi执行shHpRand workbench
    std::vector<std::vector<T>> partyShHpRandVec(size_t len);
    //读取setUp数据
    void getKeyFromSetUpFile(std::string filePath);
    //读取伪随机数
    void getAllPseudoRandFromFile(std::string filePath);

};

    template<std::integral T>
    void Party<T>::getAllPseudoRandFromFile(std::string filePath){
        kAllPseudoRand = readDataFromFile<T>(filePath + "-kAll.txt");
        kiPseudoRand = readDataFromFile<T>(filePath + "-ki.txt");
        kpPseudoRand = readDataFromFile<T>(filePath + "-kp.txt");
    }

    template<std::integral T>
    void Party<T>::getKeyFromSetUpFile(std::string filePath){
        std::vector<T> vec = ppml_with_hp::readVectorFromFile<T>(filePath);
        kAll = vec[0]; 
        ki = vec[1];
        kp = vec[2];
    }

    template<std::integral T>
    bool Party<T>::genKp(){
        //生成随机数
        T initKp = getRand<T>();
        //将随机数发送给其他方
        BroadcastExHp<T>(initKp);
        //从所有参与方接收[kp]
        std::vector<T> res = ReceiveFromAllExHp<T>();
        for(T num : res){
            initKp += num;
        }
        //使用PRF进行评估
        T evalKp = getPseudoRand<T>(initKp,ctrAllParty);
        //发送给Hp
        Send<T>(numParties , evalKp);
        //接收Hp的消息
        std::string str = ReceiveString(numParties);
        if(str.compare("continue") == 0){
            std::cout<<"SetUp phase finish"<<std::endl;
            kp = initKp;
            std::cout<<"kp = "<<kp<<std::endl;
            return true;
        }
        else if (str.compare("abort") == 0){
            std::cout<<"SetUp phase Abort"<<std::endl;
            return false;
        }
        std::cout<<"SetUp phase fail"<<std::endl;
        return false;
    }

    template<std::integral T>
    void Party<T>::recvMacShare(){
        macShare = Receive<T>(numParties);
    #ifdef PPML_HP_DEBUG_PREPROCESS
        std::cout<<"macShare = "<<macShare<<std::endl;
    #endif
    }

    template<std::integral T>
    std::pair<std::vector<T>,T> Party<T>::partyShPdRand(size_t idPd){
        T v = 0;
        std::vector<T> res(2,0);
        if(idPd == myId){
            // v = kiPseudoRand[ctrP2Hp++];
            v = kiPseudoRand[0];
            ctrP2Hp++;
            // v = getPseudoRand<T>(ki,ctrP2Hp++);
        }
        if(myId != numParties - 1){
            // res[0] = kiPseudoRand[ctrP2Hp++];
            // res[1] = kiPseudoRand[ctrP2Hp++];
            res[0] = kiPseudoRand[0];
            res[1] = kiPseudoRand[0];
            ctrP2Hp+=2;
            // res[0] = getPseudoRand<T>(ki,ctrP2Hp++);
            // res[1] = getPseudoRand<T>(ki,ctrP2Hp++);
        }
        else{
            res = ReceiveVec<T>(numParties , res.size());
        }
        return {res,v};
    }

    template <std::integral T>
    std::pair<std::vector<T>,std::vector<T>> Party<T>::partyShPdRandVec(size_t idPd , size_t len) {
        std::vector<T> vVec(len , 0);
        std::vector<T> res(2 * len , 0);
        if(idPd == myId) {
            for(size_t i = 0 ; i < len ; i++) {
                // vVec[i] = kiPseudoRand[ctrP2Hp++];
                vVec[i] = kiPseudoRand[0];
                ctrP2Hp++;
                // vVec[i] = getPseudoRand<T>(ki , ctrP2Hp++);
            }
        }
        if(myId != numParties - 1) {
            for(size_t i = 0 ; i < len ; i++) {
                // res[i * 2] = kiPseudoRand[ctrP2Hp++];
                // res[i * 2 + 1] = kiPseudoRand[ctrP2Hp++];
                res[i * 2] = kiPseudoRand[0];
                res[i * 2 + 1] = kiPseudoRand[0];
                ctrP2Hp+=2;
                // res[i * 2] = getPseudoRand<T>(ki,ctrP2Hp++);
                // res[i * 2 + 1] = getPseudoRand<T>(ki,ctrP2Hp++);
            }
        }
        else {
            res = ReceiveVec<T>(numParties , res.size());
        }
        return {res , vVec};
    }

    template<std::integral T>
    std::vector<T> Party<T>::partyShHpRand(){
        std::vector<T> res(2,0);
        if(myId != numParties - 1){
            // res[0] = kiPseudoRand[ctrP2Hp++];
            // res[1] = kiPseudoRand[ctrP2Hp++];
            res[0] = kiPseudoRand[0];
            res[1] = kiPseudoRand[0];
            ctrP2Hp+=2;
            // res[0] = getPseudoRand<T>(ki,ctrP2Hp++);
            // res[1] = getPseudoRand<T>(ki,ctrP2Hp++);
        }
        else{
            // res[0] = kiPseudoRand[ctrP2Hp++];
            res[0] = kiPseudoRand[0];
            ctrP2Hp++;
            // res[0] = getPseudoRand<T>(ki,ctrP2Hp++);
            res[1] = Receive<T>(numParties);
        }
        return res;
        // #ifdef PPML_HP_DEBUG_PREPROCESS
        // std::cout<<"res[0] = "<<res[0]<<std::endl;
        // std::cout<<"res[1] = "<<res[1]<<std::endl;
        // #endif
    }

    template<std::integral T>
    std::vector<std::vector<T>> Party<T>::partyShHpRandVec(size_t len) {
        std::vector<std::vector<T>> res(len, std::vector<T>(2, 0));
        if(myId != numParties - 1) {
            for(size_t i = 0 ; i < len ; i++) {
                // res[i][0] = kiPseudoRand[ctrP2Hp++];
                // res[i][1] = kiPseudoRand[ctrP2Hp++];
                res[i][0] = kiPseudoRand[0];
                res[i][1] = kiPseudoRand[0];
                ctrP2Hp+=2;
                // res[i][0] = getPseudoRand<T>(ki,ctrP2Hp++);
                // res[i][1] = getPseudoRand<T>(ki,ctrP2Hp++);
            }
        }
        else {
            for(size_t i = 0 ; i < len ; i++) {
                // res[i][0] = kiPseudoRand[ctrP2Hp++];
                res[i][0] = kiPseudoRand[0];
                ctrP2Hp++;
                // res[i][0] = getPseudoRand<T>(ki,ctrP2Hp++);
            }
            auto vec = ReceiveVec<T>(numParties , len);
            for(size_t i = 0 ; i < len ; i++) {
                res[i][1] = vec[i];
            }
        }
        return res;
    }

    template <std::integral T>
    std::vector<std::vector<T>> Party<T>::partyShHpvVec(size_t len) {
        std::vector<std::vector<T>> res(len, std::vector<T>(2, 0));
        if(myId != numParties - 1) {
            for(size_t i = 0 ; i < len ; i++) {
                // res[i][0] = kiPseudoRand[ctrP2Hp++];
                // res[i][1] = kiPseudoRand[ctrP2Hp++];
                res[i][0] = kiPseudoRand[0];
                res[i][1] = kiPseudoRand[0];
                ctrP2Hp+=2;
                // res[i][0] = getPseudoRand<T>(ki,ctrP2Hp++);
                // res[i][1] = getPseudoRand<T>(ki,ctrP2Hp++);
            }
        }
        else {
            auto vec = ReceiveVec<T>(numParties , len * 2);
            for(size_t i = 0 ; i < len ; i++) {
                res[i][0] = vec[i * 2];
                res[i][1] = vec[i * 2 + 1];
            }
        }
        return res;
    }

    template<std::integral T>
    std::vector<T> Party<T>::partyShHpv(){
        std::vector<T> res(2,0);
        if(myId != numParties - 1){
            // res[0] = kiPseudoRand[ctrP2Hp++];
            // res[1] = kiPseudoRand[ctrP2Hp++];
            res[0] = kiPseudoRand[0];
            res[1] = kiPseudoRand[0];
            ctrP2Hp+=2;
            // res[0] = getPseudoRand<T>(ki,ctrP2Hp++);
            // res[1] = getPseudoRand<T>(ki,ctrP2Hp++);
        }
        else{
            res = ReceiveVec<T>(numParties,res.size());
        }
        return res;
        // #ifdef PPML_HP_DEBUG_PREPROCESS
        // std::cout<<"res[0] = "<<res[0]<<std::endl;
        // std::cout<<"res[1] = "<<res[1]<<std::endl;
        // #endif
    }
}

#endif