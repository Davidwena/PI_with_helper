#ifndef PPML_HP_HP_H
#define PPML_HP_HP_H

#include <vector>
#include <iostream>
#include "../utils/vectorUtil.h"
#include "../utils/rand.h"
#include "../utils/uint128_io.h"
#include "PartyBase.h"

namespace ppml_with_hp{

template <std::integral T>
class Hp : public PartyBase{
public:
    //继承构造函数
    using PartyBase::PartyBase;
    // Hp类不能被复制构造函数以及复制赋值运算符
    Hp(const Hp&) = delete;
    Hp& operator=(const Hp&) = delete;
    Hp(std::size_t n,std::size_t myId,std::size_t portBase,std::string taskName,NetworkMode mode = NetworkMode::LAN, T mac = 0) 
    : PartyBase(n,myId,portBase,taskName,mode),mac(mac) {
        //设置计数器数组大小和初值
        ctrPi.resize(numParties,0);
    }
    //定义Hp拥有的秘钥
    T kAll,mac;
    std::vector<T> ki;
    //Hp和每一个Pi都有一个计数器
    std::vector<T> ctrPi;
    //存储kAll生成的伪随机数
    std::vector<T> kAllPseudoRand;
    //存储ki生成的伪随机数
    std::vector<std::vector<T>> kiPseudoRand;

    //生成Tp类型的kAll
    void genKAll();
    //生成Tp类型数组ki
    void genVecKi();
    //检查kp是否一致
    bool checkKp();
    //生成Tp类型的MAC 并分发
    void genMacKeyAndDis();
    //Hp执行shPdRand
    T hpShPdRand(size_t idPd);
    //Hp执行shPdRand workbench
    std::vector<T> hpShPdRandVec(size_t idPd , size_t len);
    //Hp执行shHpRand
    T hpShHpRand();
    //Hp执行shHpRand workbench
    std::vector<T> hpShHpRandVec(size_t len);
    //Hp执行shHpv
    void hpShHpv(T v);
    //Hp执行shHpv workbench
    void hpShHpvVec(std::vector<T> vec);
    //读取setUp数据
    void getKeyFromSetUpFile(std::string filePath);
    //读取伪随机数
    void getAllPseudoRandFromFile(std::string filePath);
};

    template <std::integral T>
    void Hp<T>::getAllPseudoRandFromFile(std::string filePath){
        kAllPseudoRand = readDataFromFile<T>(filePath + "-kAll.txt");
        kiPseudoRand.resize(numParties);
        for(size_t i = 0 ; i < numParties ; i++){
            kiPseudoRand[i] = readDataFromFile<T>(filePath + "-ki" + std::to_string(i) + ".txt");
        }
    }

    template <std::integral T>
    void Hp<T>::genKAll(){
        kAll = ppml_with_hp::getRand<T>();
    }

    template <std::integral T>
    void Hp<T>::genVecKi(){
        ki.resize(numParties);
        for(size_t i = 0; i < numParties; i++){
            ki[i] = ppml_with_hp::getRand<T>();
        }
    }

    template <std::integral T>
    bool Hp<T>::checkKp(){
        //接收所有的PRF(kp,ctr)
        std::vector<T> PRFkp = ReceiveFromAllExHp<T>();
        for(size_t i = 0;i<PRFkp.size();i++){
            std::cout<<"PRFkp["<<i<<"] = "<<PRFkp[i]<<std::endl;
        }
        bool checkKpRes = ppml_with_hp::areAllElementsEqual(PRFkp);
        if(checkKpRes){
            SendStringExHp("continue");
            return true;
        }
        else{
            SendStringExHp("abort");
            return false;
        }
        return false;
    }

    template<std::integral T>
    void Hp<T>::getKeyFromSetUpFile(std::string filePath){
        std::vector<T> vec = ppml_with_hp::readVectorFromFile<T>(filePath);
        kAll = vec[0];
        ki.resize(numParties);
        for(size_t i = 0 ; i < vec.size(); i++){
            ki[i] = vec[i + 1];
        }
    }

    template <std::integral T>
    void Hp<T>::genMacKeyAndDis(){
        std::vector<T> macShare(numParties,0);
        for(size_t i = 0 ; i < numParties ; i++){
            macShare[i] = ppml_with_hp::getRand<T>();
            #ifdef PPML_HP_DEBUG_PREPROCESS
            std::cout<<"macShare["<<i<<"] = "<<macShare[i]<<std::endl;
            #endif
            Send<T>(i , macShare[i]);
            mac += macShare[i];
            #ifdef PPML_HP_DEBUG_PREPROCESS
            if(i == numParties - 1)
                std::cout<<"mac = "<<mac<<std::endl;
            #endif
        }
    }

    //return rand
    template <std::integral T>
    T Hp<T>::hpShPdRand(size_t idPd){
        //最后一行第一个元素存储v
        // T v = kiPseudoRand[idPd][ctrPi[idPd]++];
        T v = kiPseudoRand[idPd][0];
        ctrPi[idPd]++;
        // T v = getPseudoRand<T>(ki[idPd],ctrPi[idPd]++);
        T viSum = 0,tviSum = 0;
        std::vector<T> res(2,0);
        for(size_t i = 0 ; i < numParties - 1 ; i++){
            // viSum+=kiPseudoRand[i][ctrPi[i]++];
            // tviSum+=kiPseudoRand[i][ctrPi[i]++];
            viSum+=kiPseudoRand[i][0];
            tviSum+=kiPseudoRand[i][0];
            ctrPi[i]+=2;
            // viSum+=getPseudoRand<T>(ki[i],ctrPi[i]++);
            // tviSum+=getPseudoRand<T>(ki[i],ctrPi[i]++);
        }
        res[0] = v - viSum;
        res[1] = v * mac - tviSum;
        SendVec<T>(numParties - 1 , res);
        return v;
    }

    template <std::integral T>
    std::vector<T> Hp<T>::hpShPdRandVec(size_t idPd , size_t len) {
        std::vector<T> vVec(len , 0);
        std::vector<T> res(len * 2 , 0);
        for(size_t i = 0 ; i < len ; i++) {
            // vVec[i] = kiPseudoRand[idPd][ctrPi[idPd]++];
            vVec[i] = kiPseudoRand[idPd][0];
            ctrPi[idPd]++;
            // vVec[i] = getPseudoRand<T>(ki[idPd],ctrPi[idPd]++);
        }
        for(size_t i = 0 ; i < len ; i++) {
            T viSum = 0,tviSum = 0;
            for(size_t j = 0 ; j < numParties - 1 ; j++){
                // viSum+=kiPseudoRand[j][ctrPi[j]++];
                // tviSum+=kiPseudoRand[j][ctrPi[j]++];
                viSum+=kiPseudoRand[j][0];
                tviSum+=kiPseudoRand[j][0];
                ctrPi[j]+=2;
                // viSum+=getPseudoRand<T>(ki[j],ctrPi[j]++);
                // tviSum+=getPseudoRand<T>(ki[j],ctrPi[j]++);
            }
            res[i * 2] = vVec[i] - viSum;
            res[i * 2 + 1] = vVec[i] * mac - tviSum;
        }
        SendVec<T>(numParties - 1 , res);
        return vVec;
    }

    template <std::integral T>
    T Hp<T>::hpShHpRand(){
        T viSum = 0,tviSum = 0;
        for(size_t i = 0 ; i < numParties - 1 ; i++){
            // viSum+=kiPseudoRand[i][ctrPi[i]++];
            // tviSum+=kiPseudoRand[i][ctrPi[i]++];
            viSum+=kiPseudoRand[i][0];
            tviSum+=kiPseudoRand[i][0];
            ctrPi[i]+=2;
            // viSum+=getPseudoRand<T>(ki[i],ctrPi[i]++);
            // tviSum+=getPseudoRand<T>(ki[i],ctrPi[i]++);
        }
        // T v = viSum + kiPseudoRand[numParties - 1][ctrPi[numParties - 1]++];
        T v = viSum + kiPseudoRand[numParties - 1][0];
        ctrPi[numParties - 1]++;
        // T v = viSum + getPseudoRand<T>(ki[numParties - 1],ctrPi[numParties - 1]++);
        T tvn = mac * v - tviSum;
        Send<T>(numParties - 1,tvn);
        return v;
    }

    //return RandVec 
    template <std::integral T>
    std::vector<T> Hp<T>::hpShHpRandVec(size_t len) {
        std::vector<T> vVec(len , 0);
        std::vector<T> tvnVec(len , 0);
        for(size_t i = 0 ; i < len ; i++) {
            T viSum = 0,tviSum = 0;
            for(size_t j = 0 ; j < numParties - 1 ; j++) {
                // viSum += kiPseudoRand[j][ctrPi[j]++];
                // tviSum += kiPseudoRand[j][ctrPi[j]++];
                viSum += kiPseudoRand[j][0];
                tviSum += kiPseudoRand[j][0];
                ctrPi[j]+=2;
                // viSum+=getPseudoRand<T>(ki[j],ctrPi[j]++);
                // tviSum+=getPseudoRand<T>(ki[j],ctrPi[j]++);
            }
            // vVec[i] = viSum + kiPseudoRand[numParties - 1][ctrPi[numParties - 1]++];
            vVec[i] = viSum + kiPseudoRand[numParties - 1][0];
            ctrPi[numParties - 1]++;
            // vVec[i] = viSum + getPseudoRand<T>(ki[numParties - 1],ctrPi[numParties - 1]++);
            tvnVec[i] = mac * vVec[i] - tviSum;
        }
        SendVec<T>(numParties - 1, tvnVec);
        return vVec;
    }

    template <std::integral T>
    void Hp<T>::hpShHpv(T v){
        std::vector<T> vec(2,0);
        T viSum = 0,tviSum = 0;
        for(size_t i = 0 ; i < numParties - 1 ; i++){
            // viSum+=kiPseudoRand[i][ctrPi[i]++];
            // tviSum+=kiPseudoRand[i][ctrPi[i]++];
            viSum+=kiPseudoRand[i][0];
            tviSum+=kiPseudoRand[i][0];
            ctrPi[i]+=2;
            // viSum+=getPseudoRand<T>(ki[i],ctrPi[i]++);
            // tviSum+=getPseudoRand<T>(ki[i],ctrPi[i]++);
        }
        vec[0] = v - viSum;
        vec[1] = v * mac - tviSum;
        SendVec<T>(numParties - 1,vec);
    }

    template <std::integral T>
    void Hp<T>::hpShHpvVec(std::vector<T> vec) {
        std::vector<T> res(2 * vec.size() , 0);
        for(size_t i = 0 ; i < vec.size() ; i++) {
            T viSum = 0,tviSum = 0;
            for( size_t j = 0 ; j < numParties - 1 ; j++) {
                // viSum += kiPseudoRand[j][ctrPi[j]++];
                // tviSum += kiPseudoRand[j][ctrPi[j]++];
                viSum+=kiPseudoRand[j][0];
                tviSum+=kiPseudoRand[j][0];
                ctrPi[j]+=2;
                // viSum+=getPseudoRand<T>(ki[j],ctrPi[j]++);
                // tviSum+=getPseudoRand<T>(ki[j],ctrPi[j]++);
            }
            res[i * 2] = vec[i] - viSum;
            res[i * 2 + 1] = vec[i] * mac - tviSum;
        }
        SendVec<T>(numParties - 1 , res);
    }
}

#endif