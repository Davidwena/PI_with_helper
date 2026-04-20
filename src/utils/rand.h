#ifndef PPML_HP_UTILS_RAND_H
#define PPML_HP_UTILS_RAND_H

#include <random>
#include <algorithm>
#include <concepts>
#include <iostream>      // 标准输入输出流
#include <fstream>       // 文件流操作
#include <string>        // std::string
#include <stdexcept>     // std::runtime_error
#include <iomanip>       // std::setprecision
#include <limits>        // std::numeric_limits
#include <type_traits>   // std::is_floating_point_v


namespace ppml_with_hp {

//生成一个Tp类型的随机数 
/// Generate a random number of type Tp
/// @tparam Tp The type of the random number to be generated, should be an integral type (e.g., uint64_t)
/// @return The generated random number of type Tp
template <std::integral Tp>
inline
Tp getRand() {
    using EngOutput_t = unsigned; // EngOutput_t is the output type of the random number generator
    static_assert(sizeof(Tp) >= sizeof(EngOutput_t));

    static std::random_device rd;
    static std::independent_bits_engine<std::default_random_engine, 8 * sizeof(EngOutput_t), EngOutput_t> rng(rd());

    // Output of std::independent_bits_engine must be EngOutput_t, so we use sizeof to determine buffer length
    EngOutput_t buf[sizeof(Tp) / sizeof(EngOutput_t)];
    std::generate(std::begin(buf), std::end(buf), []() { return rng(); });

    return *reinterpret_cast<Tp*>(buf);
}

//预先生成伪随机数
// template <typename Tp>
// std::vector<Tp> genPseudoRand(Tp seed , size_t len) {
//     std::vector<Tp> vec(65536 * len);
//     for(size_t i = 0 ; i < 65536 * len ; i++) {
//         // 初始化随机数生成器
//         //溢出会导致回绕
//         std::mt19937_64 rng(seed+i); // 使用种子初始化 Mersenne Twister 随机数生成器
//         // 生成一个 uint64_t 类型的伪随机数
//         std::uniform_int_distribution<Tp> dist;
//         vec[i] = dist(rng);
//     }
//     return vec; 
// }

template <typename Tp>
void genPseudoRandToFile(Tp seed, size_t len, const std::string& filePath) {
    // 打开文件用于写入
    std::ofstream outFile(filePath);
    
    if (!outFile.is_open()) {
        throw std::runtime_error("无法打开文件: " + filePath);
    }
    
    // 设置输出精度（如果是浮点类型）
    if constexpr (std::is_floating_point_v<Tp>) {
        outFile << std::setprecision(std::numeric_limits<Tp>::max_digits10);
    }
    
    for (size_t i = 0; i < 65536 * len; i++) {
        // 初始化随机数生成器
        std::mt19937_64 rng(seed + i); // 使用种子初始化 Mersenne Twister 随机数生成器
        
        // 生成一个伪随机数
        std::uniform_int_distribution<Tp> dist;
        Tp randomValue = dist(rng);
        
        // 将随机数写入文件，一个元素一行
        outFile << randomValue << '\n';
    }
    
    outFile.close();
}


//使用seed和ctr生成伪随机数
//造成时间开销的主要原因
template <typename Tp>
Tp getPseudoRand(Tp seed, Tp ctr){
    // 初始化随机数生成器
    //溢出会导致回绕
    std::mt19937_64 rng(seed+ctr); // 使用种子初始化 Mersenne Twister 随机数生成器
    // 生成一个 uint64_t 类型的伪随机数
    std::uniform_int_distribution<Tp> dist;
    Tp random_number = dist(rng);
    return random_number;
}

//生成[0-k]范围的随机数
template<typename T>
T generateRandomInRange(T k) {
    // 静态断言检查T是数值类型
    static_assert(std::is_arithmetic<T>::value, "T must be a numeric type");
    
    // 根据T是整数还是浮点数选择不同的随机数生成方式
    if constexpr (std::is_integral<T>::value) {
        // 整数类型
        std::random_device rd;  // 获取随机种子
        std::mt19937 gen(rd()); // 标准的mersenne_twister_engine
        std::uniform_int_distribution<T> dis(0, k);
        return dis(gen);
    } else {
        // 浮点数类型
        std::random_device rd;  // 获取随机种子
        std::mt19937 gen(rd()); // 标准的mersenne_twister_engine
        std::uniform_real_distribution<T> dis(0, k);
        return dis(gen);
    }
}

// 生成长度为len，范围为[0,k]的随机数数组
template<typename T>
std::vector<T> generateRandomArray(size_t len, T k) {
    // 静态断言检查T是数值类型
    static_assert(std::is_arithmetic<T>::value, "T must be a numeric type");
    
    // 创建一个随机数生成器，用于整个数组
    std::random_device rd;  // 获取随机种子
    std::mt19937 gen(rd()); // 标准的mersenne_twister_engine
    
    // 创建存储随机数的向量
    std::vector<T> randomArray(len);
    
    // 根据T的类型选择合适的分布
    if constexpr (std::is_integral<T>::value) {
        // 整数类型
        std::uniform_int_distribution<T> dis(0, k);
        // 填充数组
        for (size_t i = 0; i < len; ++i) {
            randomArray[i] = dis(gen);
        }
    } else {
        // 浮点数类型
        std::uniform_real_distribution<T> dis(0, k);
        // 填充数组
        for (size_t i = 0; i < len; ++i) {
            randomArray[i] = dis(gen);
        }
    }
    
    return randomArray;
}

// template <typename T>
// std::vector<T> readDataFromFile(const std::string& filePath) {
//     // 打开文件
//     std::ifstream inFile(filePath);
    
//     if (!inFile.is_open()) {
//         throw std::runtime_error("无法打开文件: " + filePath);
//     }
    
//     std::vector<T> data;
//     std::string line;
    
//     // 逐行读取文件
//     while (std::getline(inFile, line)) {
//         // 跳过空行
//         if (line.empty()) {
//             continue;
//         }
        
//         // 将字符串转换为T类型
//         T value;
//         std::istringstream iss(line);
        
//         // 检查转换是否成功
//         if constexpr (std::is_same_v<T, std::string>) {
//             // 如果T是字符串类型，直接使用整行
//             value = line;
//         } else {
//             // 对于其他类型，使用流操作符
//             if (!(iss >> value)) {
//                 throw std::runtime_error("无法将行转换为指定类型: " + line);
//             }
            
//             // 检查是否有额外的非空白字符
//             char extra;
//             if (iss >> extra) {
//                 throw std::runtime_error("行包含额外的数据: " + line);
//             }
//         }
        
//         data.push_back(value);
//     }
    
//     inFile.close();
//     return data;
// }

//仅读取第一行元素，加速IO操作
template <typename T>
std::vector<T> readDataFromFile(const std::string& filePath) {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        throw std::runtime_error("无法打开文件: " + filePath);
    }

    std::vector<T> data;
    std::string line;

    while (std::getline(inFile, line)) {
        if (line.empty()) {
            continue; // 跳过空行
        }

        T value;
        std::istringstream iss(line);

        if constexpr (std::is_same_v<T, std::string>) {
            value = line; // 直接使用整行内容
        } else {
            if (!(iss >> value)) {
                throw std::runtime_error("无法将行转换为指定类型: " + line);
            }
            // 检查是否有额外字符
            char extra;
            if (iss >> extra) {
                throw std::runtime_error("行包含额外的数据: " + line);
            }
        }

        data.push_back(value);
        break; // 处理完第一个非空行后终止循环
    }

    inFile.close();
    return data; // 若无有效行则返回空vector
}

}

#endif