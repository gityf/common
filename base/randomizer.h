#ifndef COMMON_MATH_RANDOMIZER_H
#define COMMON_MATH_RANDOMIZER_H

#include <climits>
#include <stdlib.h>

namespace common {

class Randomizer
{
public:
    explicit Randomizer(unsigned int s = 0);

    unsigned int GetSeed();
    void SetSeed(unsigned int s);
    unsigned int Rand(unsigned int max = RAND_MAX);

private:
    unsigned int mSeed;
};

} // namespace common

#endif // COMMON_MATH_RANDOMIZER_H
