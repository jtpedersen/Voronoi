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

struct Voronoi {
  vector<int> hits;
  vector<vec3> colors;
  KDTree kdtree;
  int w, h, cnt;
  float current_error;
  Image *edges;
  Image *reference;
  Voronoi(int w, int h, int cnt = 42) : w(w), h(h) , cnt(cnt){
    for(int i = 0; i < cnt ; i++) {
      vec2 p = util::randomVec2();
      p.x *= w;
      p.y *= h;
      kdtree.insert(KDNode(p,i));
      colors.emplace_back(util::randomVec3());
      hits.emplace_back(0);
    }
    kdtree.build();
    //	kdtree.dumpNodes(cerr);
  }

  Voronoi(const Voronoi& o, const KDTree& tree) 
    : kdtree(tree), w(o.w), h(o.h), cnt(o.cnt), 
      current_error(-1), edges(o.edges), reference(o.reference)
  {
    for(int i = 0; i < cnt ; i++) {
      hits.emplace_back(0);
      colors.emplace_back(vec3(0));
    }
  }

  Image render() const {
    Image res(w,h);
    for(int j = 0; j < h; j++) {
      for(int i = 0; i < w; i++) {
	vec2 p(i,j);
	auto idx = kdtree.findNearest(p).idx;
	res.pixels[i + j * w] = colors[idx];
      }
    }
    return res;
  }

  void setColorFromImageAverage(const Image& img) {
    for(auto& h: hits) h = 0;
    for(auto& c: colors) { c.x = c.y = c.z = 0;}
    for(int j = 0; j < h; j++) {
      for(int i = 0; i < w; i++) {
	vec2 p(i,j);
	auto idx = kdtree.findNearest(p).idx;
	hits[idx]++;
	colors[idx] += img.pixels[i + w * j];
      }
    }
    for(int i = 0; i < cnt; i++) {
      if (hits[i] > 0) {
	colors[i] /= float(hits[i]);
      }
    }
  }

  float measureError(const KDTree& kdtree) {
#if 0
    float sum = 0;
    for(int j = 0; j < h; j++) {
      for(int i = 0; i < w; i++) {
	vec2 p(i,j);
	auto kdnode = kdtree.findNearest(p);
	auto dist = distance2(p, kdnode.p);
	assert(std::isfinite(dist));
	auto err =  (length2(edges->pixels[j*w + i])) / (.1f + dist);
	assert(std::isfinite(length2(edges->pixels[j*w + i])));
	assert(std::isfinite(err));
	sum += err;
      }
    }
    assert(std::isfinite(sum));
    //cerr << "total error: " << sum << endl;
    return sum;
#else
    setColorFromImageAverage(*reference);
    auto rmse = render().rmse(*reference);
    return length(rmse);
#endif
  }
  glm::vec3 permutation() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> box_dist(-.5, .5);
    return glm::vec3(box_dist(gen), box_dist(gen), box_dist(gen));
  }
  
  void permuteBasedOnError(float temperature) {
    if (current_error == 0)
      current_error = measureError(kdtree);

    auto nextTree = KDTree(kdtree);
    int move = std::max(1.0f, temperature);
    for(int i = 0; i < move ; i++) {
      int idx = static_cast<int>(cnt*util::randf());
      auto disturbance = permutation();
      auto& p = nextTree[idx].p;
      p.x += move * disturbance.x;
      p.y += move * disturbance.y;
      p.x = std::max(std::min(p.x, float(w-1) ), 0.0f);
      p.y = std::max(std::min(p.y, float(h-1) ), 0.0f);
  
      assert(p.x >= 0);
      assert(p.y >= 0);
      assert(p.x < w);
      assert(p.y < h);
    }
    cout << "T: " << temperature << "|";
    nextTree.rebuild();
    auto tmp = Voronoi(*this, nextTree);
    float newError = tmp.measureError(nextTree);
    auto diff =  current_error - newError;
    auto norm_diff =  diff / float(w*h*3);
    float prob =  ( temperature - 100.0 * norm_diff) / 100.0; 

    if (diff > 0  ||  util::randf() < prob) {
      kdtree.clear();
      kdtree = nextTree;
      current_error = newError;
      cout << "taken:";
    } else {
      cout << "      ";
    }
    cout << " |" << current_error << " diff: " << diff << "(" << (norm_diff) <<"), prob: " << prob << endl;
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
    tex_pixels[i] = (f2b(p.x) << 16) | (f2b(p.y) << 8 ) | f2b(p.z);
  }
  // draw
  SDL_UpdateTexture(tex, NULL, tex_pixels, img.w * sizeof(uint32_t));
  SDL_Rect rect = {0, 0, img.w, img.h};
  SDL_RenderCopy(renderer, tex, NULL, &rect);
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
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
  auto edges = img.edgy();
    
  startDisplay(img.w, img.h);
  sendToDisplay(img);
  auto v = Voronoi(img.w, img.h, cnt);
  v.edges = &edges;
  v.reference = &img;

  SDL_Event e;
  float fps = 10.0;
  auto start_time = SDL_GetTicks();

  double T0 = 100;
  double scale = .01;
  double T = T0;
  double k = 1.0;
  int iteration = 0;
  int mode = 0;
  while(1) {
    iteration++;
    // check input
    while (SDL_PollEvent(&e)){
      if (e.type == SDL_QUIT || SDLK_q == e.key.keysym.sym) exit(0);
      if (SDLK_m == e.key.keysym.sym) mode++;
    }
    // cool down
    T = T * exp(-scale*k);
    v.permuteBasedOnError( T );
    // display
#if 1
    v.setColorFromImageAverage(img);
    if (0 == (mode%2)) {
      sendToDisplay(v.render());
    } else {
      sendToDisplay(edges);
    }
#endif
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

