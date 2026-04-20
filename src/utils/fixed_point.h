
#ifndef PPML_WITH_HP_FIXED_POINT_H
#define PPML_WITH_HP_FIXED_POINT_H


#include <type_traits>
#include <vector>
#include <algorithm>
#include <execution>
#include <cstdint>
#include <stdexcept>
#include <cstring>


namespace ppml_with_hp {


template <typename Tp>
Tp truncateClear(Tp x);

template <typename Tp>
Tp reverseTrun(Tp x);

template <typename Tp>
[[nodiscard]]
std::vector<Tp> reverseTrunVec(const std::vector<Tp>& x);

template <typename Tp>
void reverseTrunVecInplace(std::vector<Tp>& x);

template <typename Tp>
[[nodiscard]]
std::vector<Tp> truncateClearVec(const std::vector<Tp>& x);

template <typename Tp>
void truncateClearVecInplace(std::vector<Tp>& x);

template <typename Tp>
double decodeFixWithLBits(Tp raw, std::size_t fracBits, std::size_t lBits);

template <typename ClearType>
[[nodiscard]] ClearType double2fix(double x);

template <typename ClearType>
[[nodiscard]] std::vector<ClearType> double2fixVec(const std::vector<double>& x);

template <typename ClearType>
[[nodiscard]] double fix2double(ClearType x);

template <typename ClearType>
[[nodiscard]] std::vector<double> fix2doubleVec(const std::vector<ClearType>& x);


namespace FixedPoint {
    constexpr int fractionBits = 12;
    constexpr int truncateValue = 1 << fractionBits;
}


template <typename Tp>
Tp truncateClear(Tp x) {
    return x >> FixedPoint::fractionBits;
}

template <typename Tp>
Tp reverseTrun(Tp x) {
    return x << FixedPoint::fractionBits;
}

template <typename Tp>
[[nodiscard]]
std::vector<Tp> reverseTrunVec(const std::vector<Tp>& x) {
    std::vector<Tp> ret(x.size());
#ifdef _LIBCPP_HAS_NO_INCOMPLETE_PSTL
    std::transform(x.begin(), x.end(), ret.begin(), reverseTrun<Tp>);
#else
    std::transform(std::execution::par_unseq, x.begin(), x.end(), ret.begin(), reverseTrun<Tp>);
#endif
    return ret;
}

template <typename Tp>
void reverseTrunVecInplace(std::vector<Tp>& x) {
#ifdef _LIBCPP_HAS_NO_INCOMPLETE_PSTL
    std::transform(x.begin(), x.end(), x.begin(), reverseTrun<Tp>);
#else
    std::transform(std::execution::par_unseq, x.begin(), x.end(), x.begin(), reverseTrun<Tp>);
#endif
}

template <typename Tp>
[[nodiscard]]
std::vector<Tp> truncateClearVec(const std::vector<Tp>& x) {
    std::vector<Tp> ret(x.size());
#ifdef _LIBCPP_HAS_NO_INCOMPLETE_PSTL
    std::transform(x.begin(), x.end(), ret.begin(), truncateClear<Tp>);
#else
    std::transform(std::execution::par_unseq, x.begin(), x.end(), ret.begin(), truncateClear<Tp>);
#endif
    return ret;
}

template <typename Tp>
void truncateClearVecInplace(std::vector<Tp>& x) {
#ifdef _LIBCPP_HAS_NO_INCOMPLETE_PSTL
    std::transform(x.begin(), x.end(), x.begin(), truncateClear<Tp>);
#else
    std::transform(std::execution::par_unseq, x.begin(), x.end(), x.begin(), truncateClear<Tp>);
#endif
}


template <typename ClearType>
ClearType double2fix(double x) {
    return static_cast<ClearType>(x * FixedPoint::truncateValue);
}

template <typename ClearType>
std::vector<ClearType> double2fixVec(const std::vector<double>& x) {
    std::vector<ClearType> res(x.size());
    for (int i = 0; i < x.size(); i++) {
        res[i] = double2fix<ClearType>(x[i]);
    }
    return res;
}


template <typename ClearType>
double fix2double(ClearType x) {
    return static_cast<double>(static_cast<std::make_signed_t<ClearType>>(x)) / FixedPoint::truncateValue;
}

template <typename ClearType>
std::vector<double> fix2doubleVec(const std::vector<ClearType>& x) {
    std::vector<double> res(x.size());
    std::transform(x.begin(), x.end(), res.begin(), fix2double<ClearType>);
    return res;
}

template <typename Tp>
double decodeFixWithLBits(Tp raw, std::size_t fracBits, std::size_t lBits) {
    static_assert(std::is_unsigned_v<Tp>, "Tp must be an unsigned integer type");

    if (lBits == 0 || lBits > sizeof(Tp) * 8) {
        throw std::invalid_argument("lBits is out of valid range");
    }

    using SignedTp = std::make_signed_t<Tp>;

    if (lBits == sizeof(Tp) * 8) {
        SignedTp sv;
        std::memcpy(&sv, &raw, sizeof(Tp));   // 按位解释为有符号数
        return static_cast<double>(sv) / static_cast<double>(Tp(1) << fracBits);
    }

    Tp mask = (Tp(1) << lBits) - 1;
    Tp u = raw & mask;
    Tp sign_bit = Tp(1) << (lBits - 1);

    SignedTp sv;
    if (u & sign_bit) {
        // 负数：u - 2^l
        sv = static_cast<SignedTp>(u) - static_cast<SignedTp>(Tp(1) << lBits);
    } else {
        sv = static_cast<SignedTp>(u);
    }

    return static_cast<double>(sv) / static_cast<double>(Tp(1) << fracBits);
}

} 

#endif 
