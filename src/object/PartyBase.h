#ifndef PPML_HP_PartyBase_H
#define PPML_HP_PartyBase_H

#include <random>
#include <vector>
#include <boost/asio.hpp>
#include <mutex>
#include <iostream>
#include <chrono>
#include <string>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include "../utils/rand.h"
#include "../utils/uint128_io.h"

namespace ppml_with_hp{

    // 网络模式枚举
    enum class NetworkMode {
        LAN,
        WAN
    };

class PartyBase{
public:

    PartyBase(std::size_t n,std::size_t myId,std::size_t portBase,std::string taskName,NetworkMode mode);
    //Hp类不能被复制构造函数以及复制赋值运算符
    PartyBase(const PartyBase&) = delete;
    PartyBase& operator=(const PartyBase&) = delete;
    //numParties表示除了HP之外所有参与方的数量,myId表示自己的ID号，portBase表示端口基点,规定Id为numParties的参与方为Hp
    std::size_t numParties;
    std::size_t myId;
    std::size_t portBase;
    std::string taskName;

    //用来处理异步操作
    boost::asio::io_context ioContext;
    //每个对象代表一个tcp套接字，用于发送数据
    std::vector<boost::asio::ip::tcp::socket> sendSockets;
    //每个对象代表一个tcp套接字，用于接收数据
    std::vector<boost::asio::ip::tcp::socket> receiveSockets;
    //表示发送数据的目标IP和端口
    std::vector<boost::asio::ip::tcp::endpoint> sendEndpoints;
    //用于监听特定端口，并接受客户端的连接请求
    std::vector<boost::asio::ip::tcp::acceptor> acceptors;
    //每个对象代表一个定时器。这些定时器可以用于实现超时机制、定时任务或延迟执行等
    std::vector<boost::asio::steady_timer> timers;
    uint64_t bytesSent = 0;
    std::chrono::steady_clock::time_point startTime, stopTime;

    mutable std::mutex cerrMutex;
    

    // 切换网络模式
    void SetNetworkMode(NetworkMode mode);
    // 获取当前网络模式
    NetworkMode GetNetworkMode() const { return networkMode_; }
    // 获取当前网络参数
    double GetNetworkDelay() const { return networkDelay_; }
    double GetPacketLoss() const { return packetLoss_; }
    int GetBandwidth() const { return bandwidth_; }
    // 应用网络配置
    void ApplyNetworkConfig();
    
    // 清理网络配置
    void ClearNetworkConfig();
    
    // 执行shell命令
    void ExecuteCommand(const std::string& command);

    void SendString(std::size_t toId, const std::string& message);
    void SendStringExHp(const std::string& message);
    std::string ReceiveString(std::size_t fromId);

    void SendInt(std::size_t toId, int message);
    int ReceiveInt(std::size_t fromId);
    
    // //生成Tp类型的kAll
    // void genKAll();
    // //生成Tp类型数组ki
    // void genVecKi();
    //Hp检查targetId是否符合标准
    inline void CheckID(std::size_t targetId) const;

    //Hp向目标ID发送信息
    template <std::integral Tp>
    void Send(std::size_t toId, Tp message);

    //Hp从原始ID接收信息
    template <std::integral Tp>
    Tp Receive(std::size_t fromId);

    //Hp向目标Id发送数组
    template <std::integral Tp>
    void SendVec(std::size_t toId, const std::vector<Tp>& message);

    //Hp从发送Id接收数组
    template <std::integral Tp>
    std::vector<Tp> ReceiveVec(std::size_t fromId, std::size_t numElements);

    // template <std::integral Tp>
    // void SendVecToOther(const std::vector<Tp>& message);
    
    // template <std::integral Tp>
    // std::vector<Tp> ReceiveVecFromOther(std::size_t numElements);

    template <std::integral Tp>
    void Broadcast(Tp message);

    template <std::integral Tp>
    void BroadcastVec(const std::vector<Tp>& message);

    template <std::integral Tp>
    void BroadcastExHp(Tp message);

    template <std::integral Tp>
    void BroadcastVecExHp(const std::vector<Tp>& message);

    template <std::integral Tp>
    std::vector<Tp> ReceiveFromAllExHp();

    template <std::integral Tp>
    std::vector<std::vector<Tp>> ReceiveVecFromAllExHp(size_t numElements);

    uint16_t WhichPort(std::size_t fromId, std::size_t toId) const;
    void TryAccept(std::size_t fromId);
    void TryConnect(std::size_t toId);
    void AcceptHandler(const boost::system::error_code& ec, std::size_t fromId) const;
    void ConnectHandler(const boost::system::error_code& ec, std::size_t toId);
protected:
    // 网络模式和参数
    NetworkMode networkMode_;
    double networkDelay_;    // 毫秒
    double packetLoss_;   // 丢包率百分比
    int bandwidth_;       // Mbps
    bool networkConfigApplied_ = false;
    static inline std::mutex networkConfigMutex;
    // 连接重试时间
    static constexpr int kRetryAfterSeconds = 2;
};

    inline
    void PartyBase::CheckID(std::size_t targetId) const {
        //PartyBase的targetId范围是[0 , numParties]
        if (targetId > numParties || targetId < 0) {
            throw std::invalid_argument("Target ID out of range");
        }
        if (targetId == myId) {
            throw std::invalid_argument("Party cannot send to itself");
        }
    }

    template <std::integral Tp>
    inline void PartyBase::Send(std::size_t toId, Tp message) {
        CheckID(toId);
        bytesSent += boost::asio::write(sendSockets[toId], boost::asio::buffer(&message, sizeof(message)));
    #ifdef PPML_HP_DEBUG_ASIO
        std::lock_guard cerr_lock(cerrMutex);
        std::cerr << "Party " << myId << " sent integer " << message << " to Party " << toId << '\n';
    #endif
    }

    template <std::integral Tp>
    Tp PartyBase::Receive(std::size_t fromId) {
        CheckID(fromId);
        Tp message;
        boost::asio::read(receiveSockets[fromId], boost::asio::buffer(&message, sizeof(message)));

    #ifdef PPML_HP_DEBUG_ASIO
        std::lock_guard cerr_lock(cerrMutex);
        std::cerr << "Party " << myId << " received integer " << message << " from Party " << fromId << '\n';
    #endif
        return message;
    }

    template <std::integral Tp>
    void PartyBase::SendVec(std::size_t toId, const std::vector<Tp>& message) {
        CheckID(toId);
        bytesSent += boost::asio::write(sendSockets[toId], boost::asio::buffer(message));
    #ifdef PPML_HP_DEBUG_ASIO
        std::lock_guard cerr_lock(cerrMutex);
        std::cerr << "Party " << myId << " sent vector to Party " << toId << '\n';
        std::ranges::for_each(message, [](Tp element) {
            std::cerr << element << ' ';
        });
        std::cerr << '\n';
    #endif
    }

    // template <std::integral Tp>
    // void PartyBase::SendVec(std::size_t toId,const std::vector<Tp>& message) { // 按值传递
    // CheckID(toId);
    // auto msg_ptr = std::make_shared<std::vector<Tp>>(std::move(message));
    
    // boost::asio::async_write(
    //     sendSockets[toId],
    //     boost::asio::buffer(*msg_ptr),
    //     [this, toId, msg_ptr](boost::system::error_code ec, size_t sent) {
    //         if (!ec) {
    //             bytesSent += sent;
    //             // 调试输出...
    //         }
    //     });
    // }

    template <std::integral Tp>
    std::vector<Tp> PartyBase::ReceiveVec(std::size_t fromId, std::size_t numElements) {
        CheckID(fromId);
        std::vector<Tp> message(numElements);
        boost::asio::read(receiveSockets[fromId], boost::asio::buffer(message));

    #ifdef PPML_HP_DEBUG_ASIO
        std::lock_guard cerr_lock(cerrMutex);
        std::cerr << "Party " << myId << " received vector from Party " << fromId << '\n';
        std::ranges::for_each(message, [](Tp element) {
            std::cerr << element << ' ';
        });
        std::cerr << '\n';
    #endif
        return message;
    }

    // template <std::integral Tp>
    // void PartyBase::SendVecToOther(const std::vector<Tp>& message) {
    //     SendVec(1 - myId, message);
    // }

    // template <std::integral Tp>
    // std::vector<Tp> PartyBase::ReceiveVecFromOther(std::size_t numElements) {
    // return ReceiveVec<Tp>(1 - myId, numElements);
    // }

    template <std::integral Tp>
    void PartyBase::Broadcast(Tp message) {
        for(size_t i = 0 ; i <= numParties ; i++){
            if(i == myId) continue;
            Send<Tp>(i , message);
        }
    }

    template <std::integral Tp>
    void PartyBase::BroadcastVec(const std::vector<Tp>& message){
        for(size_t i = 0 ; i <= numParties ; i++){
            if(i == myId) continue;
            SendVec<Tp>(i , message);
        }
    };

    template <std::integral Tp>
    void PartyBase::BroadcastExHp(Tp message) {
        for(size_t i = 0 ; i < numParties ; i++){
            if(i == myId) continue;
            Send<Tp>(i , message);
        }
    }

    template <std::integral Tp>
    void PartyBase::BroadcastVecExHp(const std::vector<Tp>& message){
        for(size_t i = 0 ; i < numParties ; i++){
            if(i == myId) continue;
            SendVec<Tp>(i , message);
        }
    };

    template <std::integral Tp>
    std::vector<Tp> PartyBase::ReceiveFromAllExHp(){
        std::vector<Tp> res;
        for(size_t i = 0 ; i < numParties ; i++){
            if(i == myId) continue;
            res.emplace_back(Receive<Tp>(i));
        }
        return res;
    }

    template<std::integral Tp>
    std::vector<std::vector<Tp>> PartyBase::ReceiveVecFromAllExHp(std::size_t numElements){
        std::vector<std::vector<Tp>> res(numParties,std::vector<Tp>(numElements , 0));
        for(size_t i = 0 ; i < numParties ; i++){
            if(i == myId) continue;
            res[i] = ReceiveVec<Tp>(i,numElements);
        }
        return res;
    }
}

#endif