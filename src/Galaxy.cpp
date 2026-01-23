#include "Galaxy.hpp"
#include <random>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>

glm::vec4 hslToRgb(float h, float s, float l, float alpha)
{
    float r, g, b;
    if (s == 0.0f)
    {
        r = g = b = l;
    }
    else
    {
        auto hue2rgb = [](float p, float q, float t)
        {
            if (t < 0.0f)
                t += 1.0f;
            if (t > 1.0f)
                t -= 1.0f;
            if (t < 1.0f / 6.0f)
                return p + (q - p) * 6.0f * t;
            if (t < 1.0f / 2.0f)
                return q;
            if (t < 2.0f / 3.0f)
                return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
            return p;
        };
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        r = hue2rgb(p, q, h + 1.0f / 3.0f);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1.0f / 3.0f);
    }
    return glm::vec4(r, g, b, alpha);
}

void Galaxy::generate(std::vector<Body> &bodies, int count)
{
    bodies.clear();
    int dustCount = count / 3;
    bodies.reserve(count + dustCount);

    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    const int armCount = 2;
    const float G = 1.0f;
    const float centerMass = 10000.0f;

    // --- 1. STARS ---
    for (int i = 0; i < count; ++i)
    {
        Body b;
        b.mass = 1.0f + dis(gen) * 0.5f;

        int armIndex = i % armCount;
        float armAngle = (2.0f * 3.14159f * armIndex) / armCount;

        float r_random = dis(gen);
        float radius = 1.5f + std::pow(r_random, 1.8f) * 80.0f;

        float spiralTwist = radius * 0.35f;
        float spread = (dis(gen) - 0.5f) * std::max(0.5f, radius * 0.45f);
        float angle = armAngle + spiralTwist + spread;

        float y = 0.0f;
        if (radius < 8.0f)
        {
            y = (dis(gen) - 0.5f) * 7.0f * (1.0f - radius / 8.0f);
        }
        else
        {
            float flare = 1.0f + (radius / 80.0f) * 5.0f;
            y = (dis(gen) - 0.5f) * 2.0f * flare;
        }

        b.position = glm::vec3(std::cos(angle) * radius, y, std::sin(angle) * radius);

        glm::vec3 direction = glm::normalize(glm::vec3(-b.position.z, 0.0f, b.position.x));
        float velocityMag = std::sqrt((G * centerMass) / (radius + 0.1f));
        velocityMag *= (0.9f + dis(gen) * 0.2f);
        b.velocity = direction * velocityMag;
        b.acceleration = glm::vec3(0.0f);
        b.isStatic = false;

        // Visuals: Brighter/Lighter
        float sizeRandom = dis(gen);
        float size = 0.5f + dis(gen) * 0.5f;

        if (radius < 8.0f)
        {
            // Core: Extremely bright white
            b.originalColor = hslToRgb(0.6f + dis(gen) * 0.1f, 0.2f, 0.95f, 1.0f);
            size *= 1.5f;
        }
        else
        {
            if (sizeRandom > 0.98f)
            {
                // Blue Giants
                size = 5.0f;
                b.originalColor = hslToRgb(0.6f, 1.0f, 0.9f, 0.95f);
            }
            else if (sizeRandom > 0.95f)
            {
                // Orange Giants
                size = 3.5f;
                b.originalColor = hslToRgb(0.08f, 1.0f, 0.8f, 0.95f);
            }
            else
            {
                // Main Sequence (Cyan/Purple) - Increased Lightness (L)
                float hue = 0.5f + (radius / 120.0f) * 0.3f;
                b.originalColor = hslToRgb(hue, 0.8f, 0.85f, 0.9f); // L=0.85 very bright
                size *= 0.9f;
            }
        }
        b.size = size;
        bodies.push_back(b);
    }

    // --- 2. DUST (NEBULA CLOUDS) ---
    // Fix: Reduce alpha drastically to avoid "thick mud" look
    for (int i = 0; i < dustCount; ++i)
    {
        Body b;
        b.mass = 0.05f;

        int armIndex = i % armCount;
        float armAngle = (2.0f * 3.14159f * armIndex) / armCount;

        float radius = 5.0f + dis(gen) * 75.0f;
        float spiralTwist = radius * 0.35f;
        float spread = (dis(gen) - 0.5f) * (radius * 1.6f);
        float angle = armAngle + spiralTwist + spread;

        float flare = 1.0f + (radius / 75.0f) * 4.0f;
        float y = (dis(gen) - 0.5f) * 4.0f * flare;

        b.position = glm::vec3(std::cos(angle) * radius, y, std::sin(angle) * radius);

        glm::vec3 direction = glm::normalize(glm::vec3(-b.position.z, 0.0f, b.position.x));
        float velocityMag = std::sqrt((G * centerMass) / radius);
        b.velocity = direction * velocityMag;
        b.acceleration = glm::vec3(0.0f);
        b.isStatic = false;

        // Colors: Cyberpunk mix
        if (dis(gen) > 0.5f)
        {
            // Magenta/Pink - Low Alpha (0.05)
            b.originalColor = hslToRgb(0.85f + dis(gen) * 0.1f, 0.8f, 0.4f, 0.05f);
        }
        else
        {
            // Deep Purple/Blue - Low Alpha (0.05)
            b.originalColor = hslToRgb(0.65f + dis(gen) * 0.1f, 0.8f, 0.3f, 0.05f);
        }

        b.size = 20.0f + dis(gen) * 20.0f;
        bodies.push_back(b);
    }
}