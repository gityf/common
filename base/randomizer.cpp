#include <stdlib.h>
#include <time.h>
#include "math/randomizer.h"

namespace common {

Randomizer::Randomizer(unsigned int s)
{
    SetSeed(s);
}

unsigned int Randomizer::GetSeed()
{
    return mSeed;
}

void Randomizer::SetSeed(unsigned int s)
{
    mSeed = s == 0 ? ::time(NULL) : s;
}

unsigned int Randomizer::Rand(unsigned int max)
{
    if (max > RAND_MAX) 
        max = RAND_MAX;
    return ::rand_r(&mSeed) % max;
}

} // namespace common
