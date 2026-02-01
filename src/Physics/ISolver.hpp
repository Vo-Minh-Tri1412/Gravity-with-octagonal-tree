#pragma once
#include <vector>
#include "../Core/Particle.hpp"
// Có thể hiểu cái này là một cái để đại diện cho các thuật toán (Brute-force, Barnes-Hut và có thể là các thuật toán khác trong tương lai)
namespace Physics
{
    class ISolver
    {
    public:
        virtual ~ISolver() = default;
        virtual void solve(std::vector<Core::Particle> &particles, float dt) = 0;
    };
}
