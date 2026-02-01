#pragma once
#include "..Core/Particle.hpp"
#include <vector>
using namespace std;
using namespace glm;

namespace GalaxyGenerator {

struct GalaxyConfig {
    int numParticles = 10000;
    float radius = 500.0f;
    float coreRadius = 50.0f; // Bán kính tâm
    float armTwist = 3.0f; // Được tính toán trong phương trình góc quay của của hạt, awmtwist càng lớn thì hạt quay càng nhiều khi ra tới rìa
    int numArms = 4; // Số lượng xoắn ốc
    float armSpread = 0.5f; // Độ lan tỏa của các hạt
    float diskThickness = 10.0f; // Độ dày của vùng không gian đang xét tới
    float coreMass = 100000.0f; // Khối lượng trung tâm hố đen
    float G = 1.0f; // Hằng số hấp dẫn = 1 dể đơn giản hóa phép tính
    float scaleLength = 500.0f / 3.0f; // để xác định vùng ranh giới giữa nơi có mật độ hạt cao và thấp
    float concentrationPower = 0.5f; // Hệ số để phân phối khối lượng ở gần tâm
    float bulgeFraction = 0.2f; // tỉ lệ hạt trong phần trung tâm thiên hà so với tổng
    float velocityDispersion = 0.1f; // sử dụng để thêm một vận tốc ngẫu nhiên vào vận tốc cơ bản của hạt
};

vector<Particle> Generate(const GalaxyConfig& config);

float TotalMass(const vector<Particle>& particles);
vec3 CenterOfMass(const vector<Particle>& particles);
vec3 TotalMomentum(const vector<Particle>& particles);
}