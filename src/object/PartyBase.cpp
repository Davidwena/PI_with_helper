#include <iostream>
#include <string>
#include <vector>
#include <cstddef>
#include <mutex>
#include <cstdlib>
#include <sstream>
#include <boost/asio.hpp>
#include "PartyBase.h"

namespace ppml_with_hp {

// 修改后的构造函数，增加网络模式参数
PartyBase::PartyBase(std::size_t n, std::size_t myId, std::size_t portBase, std::string taskName, NetworkMode mode)
    : numParties(n), myId(myId), portBase(portBase), taskName(taskName), networkMode_(mode) {
    // 根据网络模式设置网络参数
    SetNetworkMode(mode);
    
    sendSockets.reserve(numParties + 1);
    sendEndpoints.reserve(numParties + 1);
    receiveSockets.reserve(numParties + 1);
    timers.reserve(numParties + 1);
    acceptors.reserve(numParties + 1);

    for (std::size_t otherId = 0; otherId <= numParties; ++otherId) {
        // We don't skip my_id_ when constructing the vectors, to avoid confusion on the indices
        sendSockets.emplace_back(ioContext);
        receiveSockets.emplace_back(ioContext);
        timers.emplace_back(ioContext);

        sendEndpoints.emplace_back(boost::asio::ip::make_address("127.0.0.1"), WhichPort(myId, otherId));

        // receiveEndpoint is used only once and is not stored
        boost::asio::ip::tcp::endpoint receiveEndpoint(boost::asio::ip::tcp::v4(), WhichPort(otherId, myId));
        acceptors.emplace_back(ioContext, receiveEndpoint);
    }

    // start listening from other parties
    for (std::size_t fromId = 0; fromId <= numParties; ++fromId) {
        if (fromId == myId) {
            continue;
        }
        TryAccept(fromId);
    }

    // connect to other parties
    for (std::size_t toId = 0; toId <= numParties; ++toId) {
        if (toId == myId) {
            continue;
        }
        TryConnect(toId);
    }

    // 应用网络配置
    ApplyNetworkConfig();

    ioContext.run();

    // These can be safely deleted
    sendEndpoints.clear();
    sendEndpoints.shrink_to_fit();
    acceptors.clear();
    acceptors.shrink_to_fit();
    timers.clear();
    timers.shrink_to_fit();
}

void PartyBase::SetNetworkMode(NetworkMode mode) {
    networkMode_ = mode;
    
    if (mode == NetworkMode::WAN) {
        // 配置WAN参数
        networkDelay_ = 50;      // 毫秒 (单程延迟，RTT为100ms)
        packetLoss_ = 0;         // 不设置丢包率
        bandwidth_ = 100;        // Mbps
    } else {
        // 配置LAN参数
        networkDelay_ = 0.05;    // 毫秒 (单程延迟，RTT为0.1ms)
        packetLoss_ = 0.01;         // 不设置丢包率
        bandwidth_ = 10000;      // Mbps (10 Gbps)
    }
    
    // 如果配置已应用，则更新网络配置
    if (networkConfigApplied_) {
        ClearNetworkConfig();
        ApplyNetworkConfig();
    }
}

void PartyBase::ExecuteCommand(const std::string& command) {
    std::cout << "Executing: " << command << std::endl;
    
    // 使用popen捕获命令输出
    std::string fullCommand = command + " 2>&1";
    FILE* pipe = popen(fullCommand.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to execute command: " << command << std::endl;
        return;
    }
    
    // 读取输出
    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    
    // 获取命令退出状态
    int status = pclose(pipe);
    
    if (status != 0) {
        std::cerr << "Command failed with status " << status << ": " << command << std::endl;
        if (!result.empty()) {
            std::cerr << "Output: " << result << std::endl;
        }
    } else if (!result.empty()) {
        std::cout << "Command output: " << result << std::endl;
    }
}

void PartyBase::ApplyNetworkConfig() {
    // 获取互斥锁，确保一次只有一个线程修改网络配置
    std::lock_guard<std::mutex> lock(networkConfigMutex);
    
    if (geteuid() != 0) {
        std::cerr << "Warning: Network simulation requires root privileges. Running without network simulation." << std::endl;
        return;
    }
    
    std::string modeStr = (networkMode_ == NetworkMode::WAN) ? "WAN" : "LAN";
    std::cout << "Applying " << modeStr << " network settings..." << std::endl;
    
    try {
        // 清除现有配置
        ClearNetworkConfig();
        
        // 使用最简单的netem配置
        std::stringstream tcCmd;
        tcCmd << "tc qdisc add dev lo root netem delay " << networkDelay_ << "ms loss "<<packetLoss_<<"% rate " << bandwidth_ << "mbit";
        ExecuteCommand(tcCmd.str());
        
        networkConfigApplied_ = true;
        std::cout << "Network configuration applied: Delay=" << networkDelay_ 
                 << "ms, Loss=" << packetLoss_ << "%, Bandwidth=" << bandwidth_ << "Mbps" << std::endl;
                 
        // 验证配置是否生效
        ExecuteCommand("tc qdisc show dev lo");
    }
    catch (const std::exception& e) {
        std::cerr << "Error applying network configuration: " << e.what() << std::endl;
    }
}

void PartyBase::ClearNetworkConfig() {
    
    if (geteuid() != 0) {
        return;  // 如果没有root权限，直接返回
    }
    
    std::cout << "Clearing network configuration..." << std::endl;
    
    try {
        // 使用更彻底的方式清除TC规则
        ExecuteCommand("tc qdisc del dev lo root 2>/dev/null || true");
        ExecuteCommand("tc qdisc del dev lo ingress 2>/dev/null || true");
        
        // 验证清理结果
        ExecuteCommand("tc qdisc show dev lo");
        
        networkConfigApplied_ = false;
        std::cout << "Network configuration cleared." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error clearing network configuration: " << e.what() << std::endl;
    }
}

uint16_t PartyBase::WhichPort(std::size_t fromId, std::size_t toId) const {
    std::size_t ret = portBase + fromId * (numParties + 1) + toId;
    if (ret > 65535) {
        throw std::invalid_argument("Port number exceeds 65535");
    }
    return static_cast<uint16_t>(ret);
}

void PartyBase::TryAccept(std::size_t fromId) {
    acceptors[fromId].async_accept(
        receiveSockets[fromId],
        [this, fromId](const boost::system::error_code& ec) {
            AcceptHandler(ec, fromId);
        }
    );
}

void PartyBase::TryConnect(std::size_t toId) {
    sendSockets[toId].async_connect(
        sendEndpoints[toId],
        [toId, this](const boost::system::error_code& ec) {
            this->ConnectHandler(ec, toId);
        }
    );
}

void PartyBase::AcceptHandler(const boost::system::error_code& ec, std::size_t fromId) const {
#ifdef PPML_HP_DEBUG_ASIO
    std::lock_guard cerr_lock(cerrMutex);
    if (!ec) {
        std::cerr << "Party " << myId << " accepted Party " << fromId << '\n';
    }
    else {
        std::cerr << "Accept failed: " << ec.message() << '\n';
    }
#endif
}

void PartyBase::ConnectHandler(const boost::system::error_code& ec, std::size_t toId) {
    if (!ec) {
        // Connection successful
        return;
    }

    std::lock_guard cerr_lock(cerrMutex);
    // std::cerr << std::format("Failed to connect to Party {}, retry after {} seconds...\n", to_id, kRetryAfterSeconds);
    std::cerr << "Failed to connect to Party " << toId << ", retry after " << kRetryAfterSeconds << " seconds...\n";

    if (sendSockets[toId].is_open()) {
        sendSockets[toId].close();
    }

    timers[toId].expires_from_now(boost::asio::chrono::seconds(kRetryAfterSeconds));
    timers[toId].async_wait([toId, this](const boost::system::error_code&) {
        this->TryConnect(toId);
    });
}

void PartyBase::SendString(std::size_t toId, const std::string& message) {
    CheckID(toId);

    // We don't handle the exception from boost.asio here, we are happy to let the program terminate anyway
    boost::asio::write(sendSockets[toId], boost::asio::buffer(message));

#ifdef PPML_HP_DEBUG_ASIO
    std::lock_guard lock(cerrMutex);
    std::cerr << "Party " << myId << " sent: " << message << " to Party " << toId << '\n';
#endif
}

void PartyBase::SendStringExHp(const std::string& message) {
    for(size_t i = 0 ; i < numParties ; i++){
        if(i == myId) continue;
        SendString(i , message);
    }
}

std::string PartyBase::ReceiveString(std::size_t fromId) {
    CheckID(fromId);

    std::vector<char> buffer(1024);
    size_t length = receiveSockets[fromId].read_some(boost::asio::buffer(buffer));
    std::string message(buffer.data(), length);

#ifdef PPML_HP_DEBUG_ASIO
    std::lock_guard lock(cerrMutex);
    std::cerr << "Party " << myId << " received: " << message << " from Party " << fromId << '\n';
#endif

    return message;
}

// Send an integer
void PartyBase::SendInt(std::size_t toId, int message) {
    CheckID(toId);
    boost::asio::write(sendSockets[toId], boost::asio::buffer(&message, sizeof(message)));

#ifdef PPML_HP_DEBUG_ASIO
    std::lock_guard lock(cerrMutex);
    std::cerr << "Party" << myId << " sent integer " << message << " to Party " << toId << '\n';
#endif
}

// Receive an integer
int PartyBase::ReceiveInt(std::size_t fromId) {
    CheckID(fromId);

    int message;
    boost::asio::read(receiveSockets[fromId], boost::asio::buffer(&message, sizeof(message)));

#ifdef PPML_HP_DEBUG_ASIO
    std::lock_guard lock(cerrMutex);
    std::cerr << "Party " << myId << " received integer " << message << " from Party " << fromId << '\n';
#endif

    return message;
}

} // namespace ppml_with_hp