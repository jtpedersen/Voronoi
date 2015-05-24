#include "Image.h"

#include <iostream>
#include <fstream>
#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <glm/ext.hpp> // << friends

#include <random>
#include <sstream>

using namespace std;
using namespace glm;


struct VoronoiPoint {
    vec2 p;
    vec3 col;
    VoronoiPoint(int w, int h)
	: col(util::randomVec3()) {
	auto rnd = util::randomVec3();
	p.x = w * rnd.x;
	p.y = h * rnd.y;
    }
    int dist(const vec2& o) const{
	return (p.x-o.x)*(p.x-o.x) + (p.y-o.y)*(p.y-o.y);
    }
};

struct Voronoi {
    vector<VoronoiPoint> vps;
    vector<int> hits;
    vector<float> errors;
    int w, h, cnt;
    Voronoi(int w, int h, int cnt = 42) : w(w), h(h) , cnt(cnt){
	for(int i = 0; i < cnt ; i++) {
	    vps.emplace_back(VoronoiPoint(w,h));
	    hits.emplace_back(0);
	    errors.emplace_back(0);
	}
    }

    VoronoiPoint nearest(const vec2& p) const {
	return vps[nearestIdx(p)];
    }

    int nearestIdx(const vec2& p) const {
	auto best = 0;
	auto dist = vps[best].dist(p);
	for(int i = 1; i < vps.size(); i++) {
	    auto candidate = vps[i].dist(p);
	    if (candidate < dist) {
		dist = candidate;
		best = i;
	    }
	}
	return best;
    }

    Image render() const {
	Image res(w,h);
	for(int j = 0; j < h; j++) {
	    for(int i = 0; i < w; i++) {
		vec2 p(i,j);
		auto vp = nearest(p);
		res.pixels[i + j * w] = vp.col;
	    }
	}
	return res;
    }

    void sampleImageAndMeasureError(const Image& img) {
	for(auto& h: hits) h = 0;
	for(auto& e: errors) e = 0;
	for(auto& vp: vps) { vp.col.x = vp.col.y = vp.col.z = 0;}
	for(int j = 0; j < h; j++) {
	    for(int i = 0; i < w; i++) {
		vec2 p(i,j);
		auto idx = nearestIdx(p);
		hits[idx]++;
		vps[idx].col += img.pixels[i + w * j];
	    }
	}
	for(int i = 0; i < cnt; i++) {
	    if (hits[i] > 1) {
		vps[i].col /= float(hits[i]);
	    }
	}
	for(int j = 0; j < h; j++) {
	    for(int i = 0; i < w; i++) {
		vec2 p(i,j);
		auto idx = nearestIdx(p);
		errors[idx] += glm::distance2(vps[idx].col, img.pixels[i + w * j]);
		assert(isfinite(errors[idx]));
	    }
	}
	float sum = 0;
	for(int i = 0; i < cnt; i++) {
	    if (hits[i] > 1) {
		errors[i] /= float(hits[i]);
		sum += errors[i];
	    }
	}
	cerr << "total error: " << sum << endl;
    }
    glm::vec3 permutation() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<> box_dist(-.5, .5);
	return glm::vec3(box_dist(gen), box_dist(gen), box_dist(gen));
    }
  

    void permuteBasedOnError(float temperature) {
	for(int i = 0; i < cnt; i++) {
	    assert((errors[i] * temperature) >= 0);
	    auto prob = errors[i] * temperature;
	    if (prob > util::randf()) continue;
	    auto disturbance = permutation();
	    auto& p = vps[i].p;
	    p.x += disturbance.x;
	    p.y += disturbance.y;
	    p.x = std::max(std::min(p.x, float(w-1) ), 0.0f);
	    p.y = std::max(std::min(p.y, float(h-1) ), 0.0f);
  
	    assert(p.x >= 0);
	    assert(p.y >= 0);
	    assert(p.x < w);
	    assert(p.y < w);
	}
    }
  
    void dump(std::string filename) {
	ofstream ofs(filename);
	for(int i = 0; i < cnt; i++) {
	    char buf[2048];
	    sprintf(buf, "%04d %2.3f %2.3f %03d %f\n", i, vps[i].p.x, vps[i].p.y, hits[i], errors[i]);
	    ofs << buf;
	}
    }
};






int main(int argc, char *argv[]) {
    if (argc < 3) {
	printf("usage %s input.ppm cnt [out.ppm]\n", argv[0]);
	printf("But you get a random voronoi Image at \"test.ppm\"\n");
	auto v = Voronoi(256, 256);
	auto img = v.render();
	img.save("test.ppm");
	return 0;
    }
    Image img;
    img.load(argv[1]);
    auto cnt = atoi(argv[2]);
    auto outfile = "hest.ppm";
    auto iterations = 100;
    if (argc > 3)
	outfile = argv[3];

    auto v = Voronoi(img.w, img.h, cnt);
    v.sampleImageAndMeasureError(img);
    for(int i =0; i < iterations; i ++) {
	v.permuteBasedOnError(1.0);
	v.sampleImageAndMeasureError(img);
	char buf[128];
	sprintf(buf, "iteration-%04d.ppm", i);
	auto tmp = v.render();
	tmp.save(buf);


	sprintf(buf, "iteration-%04d.dat", i);
	v.dump(buf);

    }
    auto i = v.render();
    i.save(outfile);
    return 0;
}

