#include "common.h"

namespace util {

    float randf() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<> dist(0, 1.0);
	return dist(gen);
    }

    glm::vec3 randomVec3() {
	return glm::vec3(randf(), randf(), randf());
    }
    glm::vec2 randomVec2() {
	return glm::vec2(randf(), randf());
    }
}

