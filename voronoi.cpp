#include "Image.h"
#include "kdtree.h"

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

#include <SDL2/SDL.h>


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
    vector<vec3> edgeAverage;
    KDTree kdtree;
    int w, h, cnt;
    Voronoi(int w, int h, int cnt = 42) : w(w), h(h) , cnt(cnt){
	for(int i = 0; i < cnt ; i++) {
	    vps.emplace_back(VoronoiPoint(w,h));
	    kdtree.insert(KDNode(vps.back().p, i));
	    hits.emplace_back(0);
	    errors.emplace_back(0);
	    edgeAverage.emplace_back(vec3(0));
	}
//	kdtree.dumpNodes(cerr);
    }

    VoronoiPoint nearest(const vec2& p) const {
	return vps[nearestIdx(p)];
    }
//	
    int nearestIdx(const vec2& p) const {
	size_t res = kdtree.findNearest(p).idx;
	// cout << res << endl;
	assert(res >= 0);
	assert(res < vps.size());
	return res;
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

    void setColorFromImageAverage(const Image& img) {
	for(auto& h: hits) h = 0;
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

    }

    void measureError(const Image& edges) {
	for(auto& e: errors) e = 0;
	float sum = 0;
	for(int j = 0; j < h; j++) {
	    for(int i = 0; i < w; i++) {
		vec2 p(i,j);
		auto idx = nearestIdx(p);
		auto dist = distance2(p, vps[idx].p);
		auto err =  (length2(edges.pixels[j*w + i])) / (.1f + dist);
		errors[idx] += err;
		sum += err;
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
	kdtree.clear();
	for(int i = 0; i < cnt; i++) {
	    //assert((errors[i] * temperature) >= 0);
	    auto prob = errors[i] * temperature;
	    if (prob < util::randf()) continue;
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
	    kdtree.insert(KDNode(p, i));
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


SDL_Texture  *tex        = nullptr; 
SDL_Renderer *renderer   = nullptr;
SDL_Window   *win        = nullptr;
uint32_t     *tex_pixels = nullptr;
void startDisplay(int w, int h) { 
    if (0 != SDL_Init(SDL_INIT_EVERYTHING) ) exit(42);
    SDL_CreateWindowAndRenderer(w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL, &win, &renderer);
    if(nullptr == win || nullptr == renderer) exit(42);

    tex_pixels = new uint32_t[w*h];
    for(int i = 0; i < w * h; i++) {
	tex_pixels[i] = 0;
    }
    tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);
    SDL_UpdateTexture(tex, NULL, tex_pixels, w * sizeof(uint32_t));

}

uint8_t f2b(float f)  {
    return static_cast<uint8_t>(f*255);
}

void sendToDisplay(const Image& img) {
    for(int i = 0; i < img.w * img.h; i++) {
	auto p = img.pixels[i];
	tex_pixels[i] = f2b(p.x) << 16 | f2b(p.y) | f2b(p.z);
    }
    // draw
    SDL_UpdateTexture(tex, NULL, tex_pixels, img.w * sizeof(uint32_t));
    SDL_Rect rect = {0, 0, img.w, img.h};
    SDL_RenderCopy(renderer, tex, NULL, &rect);
}



int main(int argc, char *argv[]) {
    if (argc < 4) {
	printf("usage %s input.ppm cnt iterations \n", argv[0]);
	printf("But you get a random voronoi Image at \"test.ppm\"\n");
	auto v = Voronoi(1024, 1024, 256);
	auto img = v.render();
	img.save("test.ppm");
	return 0;
    }

    Image img;
    img.load(argv[1]);
    auto cnt = atoi(argv[2]);
    auto iterations = atoi(argv[3]);
    auto edges = img.edgy();

    startDisplay(img.w, img.h);
    sendToDisplay(img);
    // double T0 = 1000;
    // double scale = .000001;
    auto v = Voronoi(img.w, img.h, cnt);


    SDL_Event e;
    float fps = 10.0;
    auto start_time = SDL_GetTicks();

    int iteration = 0;
    while(1) {
	iteration++;
	v.measureError(edges);
	auto t = (1.0 + iterations ) / iterations;
    	v.permuteBasedOnError( 100 * t );

#if 1
	v.setColorFromImageAverage(img);
	sendToDisplay(v.render());
#endif
    	// sprintf(buf, "iteration-%04d.dat", i);
    	// v.dump(buf);



// check input
	while (SDL_PollEvent(&e)){
	    if (e.type == SDL_QUIT || SDLK_q == e.key.keysym.sym) exit(0);
	}

// display



// calc FPS
	auto end_time = SDL_GetTicks();
	auto dt = end_time - start_time;
	fps = fps * .9 + .1 * (1000.0 / dt);
	start_time = end_time;

	char buf[512];
	sprintf(buf, "%s (%3d ms, %.2f fps)",
		argv[0], dt, fps);

	SDL_SetWindowTitle(win, buf);
	SDL_RenderPresent(renderer);

    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    
    return 0;
}

