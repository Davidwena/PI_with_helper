#include "../utils/rand.h"
#include "../object/Hp.h"
#include "../utils/fixed_point.h"
#include <iostream>
#include "../utils/vectorUtil.h"
#include "../utils/linear_algebra.h"
#include <string>
#include "../protocols/Hp/CircuitForHp.h"
using namespace std;
using namespace ppml_with_hp;

int main() {
    uint64_t k = reverseTrun<uint64_t>(10);
    size_t len = 10;
    auto res = generateRandomArray(len,k);
    //output res
    for(size_t i = 0 ; i < res.size() ; i++) {
        std::cout<<res[i]<<" ";
    }
    std::cout<<std::endl;

    std::cout<<isInRange(res , k);
}