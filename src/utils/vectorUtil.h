#ifndef PPML_HP_UTILS_VECTORUTIL_H
#define PPML_HP_UTILS_VECTORUTIL_H

#include <vector>
#include <algorithm> // 包含 std::all_of
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <execution>
#include "fixed_point.h"

namespace fs = std::filesystem;

namespace ppml_with_hp{
template <typename T>
bool areAllElementsEqual(std::vector<T>& vec) {
    if (vec.empty()) {
        return true; // 空 vector 视为全部相等
    }
    T& first = vec[0]; // 获取第一个元素
    return std::all_of(vec.begin() + 1, vec.end(), [first](T& elem) {
        return elem == first; // 检查每个元素是否与第一个元素相等
    });
}

//将元素存储在指定文件中
template <typename T>
void writeElementToFile(T message,std::string filePath){  
    // 尝试打开文件
    std::ofstream outFile(filePath, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    }
    outFile << message << std::endl;
    // 关闭文件
    outFile.close();
}

// 从指定文件中读取元素
// 从指定文件中读取单个元素
template <typename T>
T readElementFromFile(const std::string& filePath) {
    T element;  // 用于存储读取的元素
    std::ifstream inFile(filePath);  // 尝试打开文件

    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for reading: " << filePath << std::endl;
        throw std::runtime_error("Failed to open file for reading.");
    }

    if (!(inFile >> element)) {  // 尝试读取一个元素
        std::cerr << "Failed to read element from file: " << filePath << std::endl;
        throw std::runtime_error("Failed to read element from file.");
    }

    inFile.close();  // 关闭文件
    return element;  // 返回读取的元素
}

//将数组存储在指定文件中
template <typename T>
void writeVectorToFile(std::vector<T>& vec,size_t size,std::string filePath){
    // 尝试打开文件
    std::ofstream outFile(filePath, std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    }
    for (size_t i = 0; i < size; ++i) {
        outFile << vec[i] << std::endl;
    }
    // 关闭文件
    outFile.close();
}

// 从指定文件中读取数组
template <typename T>
std::vector<T> readVectorFromFile(std::string filePath) {
    std::vector<T> vec;  // 用于存储读取的元素
    std::ifstream inFile(filePath);  // 尝试打开文件

    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for reading: " << filePath << std::endl;
        return vec;  // 返回空的向量
    }

    T element;
    while (inFile >> element) {  // 逐行读取元素
        vec.push_back(element);  // 将读取的元素添加到向量中
    }

    // 关闭文件
    inFile.close();
    return vec;  // 返回包含所有读取元素的向量
}

//判断指定路径文件是否存在
void removeFile(std::string filePathStr){
    fs::path filePath = filePathStr;
    if (fs::exists(filePath)){
        try {
            fs::remove(filePath);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error deleting file: " << e.what() << std::endl;
        }
    }
}

//判断一个二维数组同一列上的元素数值相同
template <typename T>
bool areColumnsEqual(const std::vector<std::vector<T>>& vec) {
    // 检查是否为空
    if (vec.empty() || vec[0].empty()) {
        return true;  
    }
    size_t rows = vec.size();      // 行数
    size_t cols = vec[0].size();   // 列数
    // 遍历每一列
    for (size_t col = 0; col < cols; ++col) {
        // 获取该列的第一个元素
        T firstElement = vec[0][col];

        // 使用并行算法检查该列的所有元素是否等于第一个元素
        bool isColumnEqual = std::all_of(std::execution::par, vec.begin(), vec.end(),
            [col, firstElement](const std::vector<T>& row) {
                return row[col] == firstElement;
            });

        // 如果某一列的元素不全部相同，返回 false
        if (!isColumnEqual) {
            return false;
        }
    }
    // 所有列的元素都相同，返回 true
    return true;
}

//使用杨辉三角计算二项式系数并填充为矩阵
template <typename T>
std::vector<T> calBinomialCoef(size_t k , size_t len) {
    std::vector<T> res((k + 1) * len , 0);
    std::vector<T> coefficients(k + 1);
    // 基本情况：C(k,0) = 1
    coefficients[0] = 1;
    // C(n,k) = C(n-1,k-1) + C(n-1,k)
    for (int i = 1; i <= k; i++) {
        // 从后向前计算，避免覆盖需要使用的值
        for (int j = i; j > 0; j--) {
            coefficients[j] += coefficients[j - 1];
        }
    }
    // 使用指针或迭代器直接填充，避免重复计算索引
    T* dest = res.data();
    for (size_t i = 0; i < len; i++) {
        // 使用std::copy直接复制整个coefficients数组到当前行
        std::copy(coefficients.begin(), coefficients.end(), dest);
        dest += (k + 1); // 移动指针到下一行的开始位置
    }
    return res;
}

//使用杨辉三角计算二项式系数要求计算0-k的所有系数，并填充为矩阵
template <typename T>
std::vector<T> calAllBinomialCoef(size_t k , size_t len) {
    size_t rowSize = (k + 1) * (k + 2) / 2;
    std::vector<T> result(len * rowSize, 0);
    std::vector<T> coefficients(rowSize, 0);
    size_t index = 0;
    // 计算从0到k阶的所有二项式系数
    for (size_t n = 0; n <= k; ++n) {
        // 计算第n阶的二项式系数 C(n,r)，其中r从0到n
        std::vector<T> row(n + 1, 0);
        row[0] = 1;  // C(n,0) = 1
        for (size_t r = 1; r <= n; ++r) {
            // 使用杨辉三角性质：C(n,r) = C(n-1,r-1) + C(n-1,r)
            if (r == n) {
                row[r] = 1;  // C(n,n) = 1
            } else {
                // 从上一行计算当前值
                size_t prev_n = n - 1;
                size_t prev_start = prev_n * (prev_n + 1) / 2;
                row[r] = coefficients[prev_start + r - 1] + coefficients[prev_start + r];
            }
        }
        // 将当前行的系数存入结果
        size_t start = n * (n + 1) / 2;
        for (size_t r = 0; r <= n; ++r) {
            coefficients[start + r] = row[r];
        }
    }
    // 将计算好的系数复制到每一行
    for (size_t i = 0; i < len; ++i) {
        std::copy(coefficients.begin(), coefficients.end(), result.begin() + i * rowSize);
    }
    return result;
}

//对数组的每k个值求和并赋值到新数组
template <typename T>
std::vector<T> sumRowMatrix(std::vector<T>& source , size_t k , size_t len) {
    std::vector<T> res(len);
    // 确保输入数组长度正确
    if (source.size() != len * k) {
        throw std::invalid_argument("Source array size must be len*k");
    }
    const T* src = source.data();
    for (size_t i = 0; i < len; i++) {
        T sum = 0;
        // 计算当前k个元素的和
        for (size_t j = 0; j < k; j++) {
            sum += *src++;
        }
        res[i] = sum;
    }
    return res;
}

//依次对数组的每1-k个值求和并赋值到新数组，直到数组遍历结束
template <typename T>
std::vector<T> sumAllkRowMatrix(std::vector<T>& source , size_t k , size_t len) {
    std::vector<T> res(k * len);
    // 确保输入数组长度正确
    if (source.size() != len * (k + 1) * k / 2) {
        throw std::invalid_argument("Source array size must be len * (k + 1) * (k + 2) / 2");
    }
    const T* src = source.data();
    for (size_t i = 0; i < len; i++) {
        for(size_t index = 1 ; index <= k ; index++){
            //计算index个元素的和
            T sum = 0;
            for (size_t j = 0; j < index; j++) {
                sum += *src++;
            }
            res[i * k + index - 1] = sum;
        }
    }
    return res;
}

//将a数组中的所有值的0次幂到k次幂的数值填充到b数组中
template <typename T>
void fillPowMatrix(std::vector<T>& a , std::vector<T>& b , size_t k , size_t len) {
    // 确保输入数组长度正确
    if (a.size() != len) {
        throw std::invalid_argument("Source array size must be len");
    }
    if (b.size() != len * (k + 1)) {
        throw std::invalid_argument("Target array size must be len*(k+1)");
    }
    // 遍历原数组中的每个元素
    for (size_t i = 0; i < a.size(); ++i) {
        // 计算当前元素在结果数组中的起始位置
        size_t baseIndex = i * (k + 1);
        // 0次幂总是1
        b[baseIndex] = T(1);
        // 使用一个变量保存当前的幂值
        T currentPower = a[i];
        // 计算1到k次幂
        for (size_t j = 1; j <= k; ++j) {
            b[baseIndex + j] = currentPower;
            currentPower *= a[i]; // 为下一次迭代准备
        }
    }
}
//将a数组中的所有值的从0次幂开始到i次幂不断添加到b数组中,i每次+1,直到k
template <typename T>
void fillAllPowMatrix(std::vector<T>& a , std::vector<T>& b , size_t k , size_t len){
    if (a.size() != len) {
        throw std::invalid_argument("Source array size must be len");
    }
    if (b.size() != len * (k + 1) * (k + 2) / 2) {
        throw std::invalid_argument("Target array size must be len*(k+1)*(k+2)/2");
    }
    // 遍历原数组中的每个元素
    for (size_t i = 0; i < a.size(); ++i) {
        // 计算当前元素在结果数组中的起始位置
        size_t baseIndex = i * (k + 1) * (k + 2) / 2;
        // 0次幂总是1
        b[baseIndex++] = T(1);
        // 使用一个数组保存当前已经计算过的幂值
        std::vector<T> currentPowers(k + 1, T(1));
        // 计算1到k次幂
        for (size_t j = 1; j <= k; ++j) {
            //将currentPowers[0] - 将currentPowers[j - 1]的值复制到b数组的baseIndex位置
            std::copy(currentPowers.begin(), currentPowers.begin() + j, b.begin() + baseIndex);
            baseIndex = baseIndex + j;
            currentPowers[j] = currentPowers[j - 1] * a[i]; // 计算当前幂值
            b[baseIndex++] = currentPowers[j];
        }
    }
}

//将a数组中的所有值的从0次幂开始到i次幂不断添加到b数组中，考虑截断的情况,i每次+1,直到k
template <typename T>
void fillAllPowMatrixTrun(std::vector<T>& a , std::vector<T>& b , size_t k , size_t len) {
    if (a.size() != len) {
        throw std::invalid_argument("Source array size must be len");
    }
    if (b.size() != len * (k + 1) * (k + 2) / 2) {
        throw std::invalid_argument("Target array size must be len*(k+1)*(k+2)/2");
    }
    // 遍历原数组中的每个元素


    for (size_t i = 0; i < a.size(); ++i) {
        // 计算当前元素在结果数组中的起始位置
        size_t baseIndex = i * (k + 1) * (k + 2) / 2;
        // 0次幂总是1
        b[baseIndex++] = T(reverseTrun(1));
        // 使用一个数组保存当前已经计算过的幂值
        std::vector<T> currentPowers(k + 1, T(1));
        currentPowers[0] = T(reverseTrun(1));
        // 计算1到k次幂
        for (size_t j = 1; j <= k; ++j) {
            //将currentPowers[0] - 将currentPowers[j - 1]的值复制到b数组的baseIndex位置
            std::copy(currentPowers.begin(), currentPowers.begin() + j, b.begin() + baseIndex);
            baseIndex = baseIndex + j;
            currentPowers[j] = currentPowers[j - 1] * a[i]; // 计算当前幂值
            //截断
            currentPowers[j] = truncateClear(currentPowers[j]);
            b[baseIndex++] = currentPowers[j];
        }
    }
}

//将a数组的每k+1个值翻转
template <typename T>
void reverseMatrix(std::vector<T>& a , size_t k , size_t len) {
    // 确保输入数组长度正确
    if (a.size() != len * (k + 1)) {
        throw std::invalid_argument("Source array size must be len*(k+1)");
    }
    // 遍历原数组中的每个元素
    for (size_t i = 0; i < len; ++i) {
        // 计算当前元素在结果数组中的起始位置
        size_t baseIndex = i * (k + 1);
        // 翻转当前k+1个元素
        std::reverse(a.begin() + baseIndex, a.begin() + baseIndex + k + 1);
    }
}

//将a数组的每i个值翻转,i每次+1,直到k
template <typename T>
void reverseAllMatrix(std::vector<T>& a , size_t k , size_t len) {
    // 确保输入数组长度正确
    if (a.size() != len * (k + 1) * (k + 2) / 2) {
        throw std::invalid_argument("Source array size must be len*(k+1)*(k+2)/2");
    }
    // 遍历原数组中的每个元素
    for(size_t i = 0 ; i < len ; i++) {
        size_t baseIndex = i * (k + 1) * (k + 2) / 2;
        //分成k段翻转
        for(size_t j = 0 ; j < k + 1 ; j++) {
            std::reverse(a.begin() + baseIndex , a.begin() + baseIndex + j + 1);
            baseIndex = baseIndex + j + 1;
        }
    }
}

//复制数组k次
template <typename T>
void expand(std::vector<T>& coefficient, size_t k) {
    if (k <= 1) return; // 如果k<=1，无需扩展
    
    size_t originalSize = coefficient.size();
    
    // 调整数组大小为原来的k倍
    coefficient.resize(k * originalSize);
    
    // 从后向前复制，避免数据覆盖
    for (size_t i = k - 1; i > 0; --i) {
        std::copy(coefficient.begin(), coefficient.begin() + originalSize, 
                 coefficient.begin() + i * originalSize);
    }
}

//判断数组res中的每一个数的范围是否在[0 , k]，若是返回true，若不是返回false
template <typename T>
bool isInRange(const std::vector<T>& res , T k) {
    return std::all_of(res.begin(), res.end(), [k](T value) {
        return value >= 0 && value <= k;
    });
}

//从(a,b,c,d)转换为(a,b,b,c,c,c,d,d,d,d)
template <typename T>
void transformVector(std::vector<T>& coefficient) {
    if (coefficient.empty()) {
        return;
    }
    
    // 计算转换后的向量大小
    size_t newSize = 0;
    for (size_t i = 0; i < coefficient.size(); ++i) {
        newSize += (i + 1);
    }
    
    // 创建新向量存储结果
    std::vector<T> result;
    result.reserve(newSize);
    
    // 填充新向量
    for (size_t i = 0; i < coefficient.size(); ++i) {
        for (size_t j = 0; j < i + 1; ++j) {
            result.push_back(coefficient[i]);
        }
    }
    
    // 替换原向量
    coefficient = std::move(result);
}
//(a,b,c,d,e,f,g,h)转换为(a,b,b,c,c,c,d,d,d,d,e,f,f,g,g,g,h,h,h,h)
template <typename T>
void transformVectorByColumns(std::vector<T>& data, size_t col) {
    if (data.empty() || col == 0) {
        return;
    }
    
    // 计算行数（可能有不完整的最后一行）
    size_t rows = (data.size() + col - 1) / col;
    
    // 计算转换后的向量大小
    size_t newSize = 0;
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < col && i * col + j < data.size(); ++j) {
            newSize += (j + 1);
        }
    }
    
    // 创建新向量存储结果
    std::vector<T> result;
    result.reserve(newSize);
    
    // 填充新向量
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < col && i * col + j < data.size(); ++j) {
            // 获取当前元素
            T currentElement = data[i * col + j];
            
            // 重复j+1次
            for (size_t k = 0; k < j + 1; ++k) {
                result.push_back(currentElement);
            }
        }
    }
    
    // 替换原向量
    data = std::move(result);
}

template <typename T>
std::vector<T> expandToDescendingPowers(const std::vector<T>& input, size_t k) {
    if (input.empty() || k < 0) {
        return {};
    }
    
    // 计算结果向量的大小: 每个元素会产生(k+1)个幂次结果
    size_t resultSize = input.size() * (k + 1);
    std::vector<T> result;
    result.reserve(resultSize);
    
    // 对每个输入元素计算k到0次幂（降序）
    for (const T& element : input) {
        for (int power = k; power >= 0; --power) {
            // 计算element的power次幂
            result.push_back(std::pow(element, power));
        }
    }
    
    return result;
}

//(a,b,c,d)变换成(a,a,b,a,b,c,a,b,c,d)
template <typename T>
std::vector<T> transformToCumulativePattern(const std::vector<T>& input) {
    if (input.empty()) {
        return {};
    }
    
    // 计算结果向量的大小: 1 + 2 + 3 + ... + n = n*(n+1)/2
    size_t n = input.size();
    size_t resultSize = n * (n + 1) / 2;
    std::vector<T> result;
    result.reserve(resultSize);
    
    // 对于每个位置i，添加input[0]到input[i]的所有元素
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j <= i; ++j) {
            result.push_back(input[j]);
        }
    }
    
    return result;
}

//输入(a,b,c,d，e,f,g,h)和k 变换成(a,a,b,a,b,c,a,b,c,d , e,e,f,e,f,g,e,f,g,h)
template <typename T>
void transformToGroupedCumulativePatternInPlace(std::vector<T>& input, size_t k) {
    if (input.empty() || k <= 0) {
        return;
    }
    
    // 保存原始数组的副本
    std::vector<T> original = input;
    
    size_t n = original.size();
    size_t numGroups = (n + k - 1) / k;
    
    // 计算结果大小
    size_t resultSize = 0;
    for (size_t group = 0; group < numGroups; ++group) {
        size_t startIdx = group * k;
        size_t endIdx = std::min(startIdx + k, n);
        size_t groupSize = endIdx - startIdx;
        resultSize += groupSize * (groupSize + 1) / 2;
    }
    
    // 调整原数组大小
    input.resize(resultSize);
    
    // 填充数据
    size_t index = 0;
    for (size_t group = 0; group < numGroups; ++group) {
        size_t startIdx = group * k;
        size_t endIdx = std::min(startIdx + k, n);
        
        for (size_t i = startIdx; i < endIdx; ++i) {
            for (size_t j = startIdx; j <= i; ++j) {
                input[index++] = original[j];
            }
        }
    }
}
//输入(a,b,c,d，e,f,g,h)和k 变换成(a,b,a,c,b,a,d,c,b,a,e,f,e,g,f,e,h,g,f,e)
template <typename T>
void transformToGroupedReversePatternInPlace(std::vector<T>& input, int k) {
    if (input.empty() || k <= 0) {
        return;
    }
    
    // 保存原始数组的副本
    std::vector<T> original = input;
    
    size_t n = original.size();
    size_t numGroups = (n + k - 1) / k;
    
    // 计算结果大小
    size_t resultSize = 0;
    for (size_t group = 0; group < numGroups; ++group) {
        size_t startIdx = group * k;
        size_t endIdx = std::min(startIdx + k, n);
        for (size_t i = startIdx; i < endIdx; ++i) {
            resultSize += (i - startIdx + 1); // 当前元素 + 之前所有元素
        }
    }
    
    // 调整原数组大小
    input.resize(resultSize);
    
    // 填充数据
    size_t index = 0;
    for (size_t group = 0; group < numGroups; ++group) {
        size_t startIdx = group * k;
        size_t endIdx = std::min(startIdx + k, n);
        
        for (size_t i = startIdx; i < endIdx; ++i) {
            input[index++] = original[i];
            
            for (size_t j = i; j > startIdx; --j) {
                input[index++] = original[j-1];
            }
        }
    }
}
}
#endif