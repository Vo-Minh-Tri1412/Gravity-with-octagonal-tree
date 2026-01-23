#pragma once
#include <vector>
#include "Physics.hpp"

class Galaxy
{
public:
    static void generate(std::vector<Body> &bodies, int count);
};