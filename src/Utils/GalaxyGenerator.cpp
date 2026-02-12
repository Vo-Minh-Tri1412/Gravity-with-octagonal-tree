#include "../Core/Particle.hpp"
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace Core
{

    struct GalaxyConfig
    {
        int totalParticles = 10000;
        float galaxyRadius = 100.0f;        // Bán kính thiên hà
        float coreRadius = 20.0f;           // Bán kính vùng trung tâm
        float coreMassFraction = 0.3f;      // coreMassFraction để tính tỉ lệ khối lượng ở vùng trung tâm
        float minMass = 0.1f;
        float maxMass = 10.0f;
        float maxVelocity = 5.0f;
        unsigned int seed = std::random_device{}(); // như seed minecraft, mỗi seed sẽ khác nhau nhưng đều thỏa điều kiện
    };

    float densityProfile(float radius, float coreRadius, float galaxyRadius){ 
        // Hàm tính mật độ tương đối(mật độ so với mật độ tại trung tâm)sẽ cho ra giảm dần theo bán kính
        float x = radius / coreRadius;
        return 1.0f / (1.0f + x * x);
    }
    
    glm::vec3 generatePosition(float radius, std::mt19937& rng){ // Hàm sinh vị trí ngẫu nhiên // mt19937& rng: Là bộ sinh số ngẫu nhiên
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f); // dùng để sinh số ngẫu nhiên để tính góc trong tọa độ cầu ở ngay phía dưới
        //uniform_real_distribut -> dùng để phân bối đều
        // Sinh góc ngẫu nhiên trong không gian cầu
        float theta = 2.0f * M_PI * dist01(rng);        
        float phi = std::acos(2.0f * dist01(rng) - 1.0f); 
        
        // Chuyển từ tọa độ cầu sang Descartes
        float x = radius * std::sin(phi) * std::cos(theta);
        float y = radius * std::sin(phi) * std::sin(theta);
        float z = radius * std::cos(phi);
        
        z *= 0.1f;  // Vì thiên hà có dạng đĩa nên ban đầu từ một quả cầu chúng ta sẽ ép để buộc chúng thành một dạng đĩa phẳng
        
        return glm::vec3(x, y, z);
    }

    // Sinh khối lượng dựa trên vị trí - nặng hơn ở trung tâm do ở gần trung tâm sẽ chịu áp suất lớn và các lượng vật chất lớn nên
    // có khả năng sinh ra các vì sao lớn
    float generateMass(float radius, const GalaxyConfig& config, std::mt19937& rng)
    {
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
        
        float densityFactor = densityProfile(radius, config.coreRadius, config.galaxyRadius); // như trên

        float avgMass = config.minMass + (config.maxMass - config.minMass) * densityFactor; //khi càng gần tâm thì khối lượng càng lớn

        float variation = 0.7f + 0.6f * dist01(rng); // variation để sản sinh ra khối lượng biến thiên ngẫu nhiên từ 0.7f - 1.3f
        float mass = avgMass * variation;   // Khối lượng hành tinh
        
        return std::max(config.minMass, std::min(config.maxMass, mass)); // Để đảm bảo rằng khối lượng luôn nằm trong khoảng cho phép
    }
    // Hàm tính tâm khối
    glm::vec3 calculateCenterOfMass(const std::vector<Particle>& particles){
        glm::vec3 weightedPositionSum(0.0f, 0.0f, 0.0f);
        float totalMass = 0.0f;
        
        for (const Particle& p : particles){
            weightedPositionSum += p.position * p.mass;
            totalMass += p.mass;
        }
        
        glm::vec3 centerOfMass = weightedPositionSum / totalMass;
        
        return centerOfMass;
    }

    // Sinh vận tốc quỹ đạo dựa trên vị trí
    glm::vec3 generateVelocity(const glm::vec3& position, float centralMass, const GalaxyConfig& config, std::mt19937& rng)
    {
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
        std::normal_distribution<float> normalDist(0.0f, 1.0f); //normal_distribution - Phân phối chuẩn để tạo độ nhiễu cho vận tốc ở phía dưới
        
        float radius = glm::length(position); // hàm để tính khoảng cách từ hạt tới gốc tọa độ
        
        if (radius < 0.001f)
        {
            // Ở tâm thì vận tốc nhỏ
            return glm::vec3(
                normalDist(rng) * 0.1f,
                normalDist(rng) * 0.1f,
                normalDist(rng) * 0.1f
            );
        }
        
        float orbitalSpeed = std::sqrt(centralMass / radius);  // Vận tốc quỹ đạo theo công thức v = sqrt(GM/r)
        
        orbitalSpeed = std::min(orbitalSpeed, config.maxVelocity); // Giới hạn vận tốc
        
        glm::vec3 zAxis(0.0f, 0.0f, 1.0f); // vector đơn vị trục z
        glm::vec3 tangent = glm::cross(zAxis, position); // vector tiếp tuyến quỹ đạo, dùng để xác định hướng vận tốc
        
        if (glm::length(tangent) < 0.001f) // lúc này ta thấy khi nhân hữu hướng zAxis và position, việc <0.001f xảy ra khi mà
        // position của chúng ta có giá trị 'vector vuông góc với trục z' rất bé hay nói cách khác là posistion của hạt gần như là thẳng đứng
        //  và song song với z, việc này sẽ gây khó khăn cho việc tính toán vận tốc tiếp tuyến của hạt 
        {
            tangent = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), position); // chúng ta sẽ thay đổi vector trục tọa độ để có thể ra được phương vận tốc
        }
        
        tangent = glm::normalize(tangent);
        
        // Vận tốc quỹ đạo chính
        glm::vec3 velocity = tangent * orbitalSpeed;
        
        // Thêm nhiễu ngẫu nhiên nhỏ (velocity dispersion)
        float dispersion = 0.1f * config.maxVelocity;
        velocity.x += normalDist(rng) * dispersion;
        velocity.y += normalDist(rng) * dispersion;
        velocity.z += normalDist(rng) * dispersion * 0.3f;  // Ít nhiễu hơn theo trục z
        
        return velocity;
    }

    // Hàm chính để sinh thiên hà
    std::vector<Particle> generateGalaxy(const GalaxyConfig& config = GalaxyConfig())
    {
        std::vector<Particle> particles;
        particles.reserve(config.totalParticles); // hàm cấp phát bộ nhớ trước để chứa particles nhằm tăng hiệu suất

        std::mt19937 rng(config.seed);
        std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
        
        // Tính tổng khối lượng
        float totalMass = config.totalParticles * (config.minMass + config.maxMass) / 2.0f;
        float coreMass = totalMass * config.coreMassFraction;
        
        int coreParticles = 0; // biến đếm hạt 
        float accumulatedCoreMass = 0.0f; // tính tổng khối lượng vùng trung tâm
        
        for (int i = 0; i < config.totalParticles; i++)
        {
            float u = dist01(rng);// Sinh bán kính theo phân bố mật độ
            float radius;
            

            if (u < config.coreMassFraction)
            {
                // Particles trong vùng core (30%)
                radius = config.coreRadius * std::sqrt(u / config.coreMassFraction);
                coreParticles++;
            }
            else
            {
                // Particles ngoài core (70%) - giảm dần theo bán kính
                float t = (u - config.coreMassFraction) / (1.0f - config.coreMassFraction);
                radius = config.coreRadius + (config.galaxyRadius - config.coreRadius) * std::pow(t, 1.5f);
                //Xác định bán kính cho sao ở đĩa ngoài, làm cho chúng tập trung gần tâm hơn và thưa dần khi đi ra ngoài (mô phỏng mật độ giảm dần).

            }
            
            // Sinh vị trí trong không gian 3D
            glm::vec3 position = generatePosition(radius, rng);
            
            // Sinh khối lượng dựa trên vị trí
            float mass = generateMass(radius, config, rng);
            
            if (radius <= config.coreRadius)
            {
                accumulatedCoreMass += mass;
            }
            
            // Tạo particle
            Particle p(position, mass);
            
            // Sinh vận tốc quỹ đạo
            p.velocity = generateVelocity(position, coreMass, config, rng);
            
            particles.push_back(p);
        }
        return particles;
    }
}