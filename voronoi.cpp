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

class Image {
public:
  int w,h;
  vector<vec3> pixels;

  Image() : w(0), h(0) {};

  Image(int w, int h) 
    : w(w), h(h), pixels(w*h) { };

  void save(string filename) {
    ofstream ofs(filename);
    ofs << "P6\n" << w <<  " " << h << "\n255\n";
    for(int i = 0; i < w*h; i++) {
      const vec3& pixel = pixels[i];
      ofs << static_cast<unsigned char>(pixel.r * 255);
      ofs << static_cast<unsigned char>(pixel.g * 255);
      ofs << static_cast<unsigned char>(pixel.b * 255);
    }
  }
  void load(string filename) {
    cerr << "opening: " << filename << endl;
    std::ifstream ifs;
    ifs.open(filename, std::ios::binary); // need to spec. binary mode for Windows users
    try {
      if (ifs.fail()) { throw("Can't open input file"); }
      std::string line;
      getline(ifs,line);
      if (line != "P6") throw("Can't read input file");
      bool inHeader = true;
      w = h = 0;
      while(inHeader) {
	getline(ifs,line);
	if (line.empty() || line[0] == '#')
	  continue;
	//	cerr << line << endl;
	if (0 == w) {
	  istringstream iss(line);
	  int b;
	  iss >> w >> h >> b;
	} else {
	  inHeader = false;
	}
      }

      cerr << "read header, now for the data " << endl;
      unsigned char pix[3];
      // read each pixel one by one and convert bytes to floats
      for (int i = 0; i < w * h; ++i) {
	ifs.read(reinterpret_cast<char *>(pix), 3);
	pixels.emplace_back( pix[0], pix[1], pix[2]);
	pixels.back() /= 255.0f;
	//	cerr << to_string(pixels.back()) << endl;
      }
      ifs.close();
    }
    catch (const char *err) {
      fprintf(stderr, "%s\n", err);
      ifs.close();
    }
    cerr << "Dimensons: " << w << " x " << h << endl;
  }
};

glm::vec3 randomVec3() {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> box_dist(0, 1.0);
  return glm::vec3(box_dist(gen), box_dist(gen), box_dist(gen));
}

struct VoronoiPoint {
  ivec2 p;
  vec3 col;
  VoronoiPoint(int w, int h)
    : col(randomVec3()) {
    auto rnd = randomVec3();
    p.x = w * rnd.x;
    p.y = h * rnd.y;
  }
  int dist(const ivec2& o) const{
    return (p.x-o.x)*(p.x-o.x) + (p.y-o.y)*(p.y-o.y);
  }
};

struct Voronoi {
  vector<VoronoiPoint> vps;
  int w, h;
  Voronoi(int w, int h) : w(w), h(h) {};
  VoronoiPoint nearest(const ivec2& p) const {
    return vps[nearestIdx(p)];
  }

  int nearestIdx(const ivec2& p) const {
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
	ivec2 p(i,j);
	auto vp = nearest(p);
	res.pixels[i + j * w] = vp.col;
      }
    }
    return res;
  }

  static Voronoi random(int w, int h, int cnt = 42) {
    Voronoi voronoi(w,h);
    for(int i = 0; i < cnt ; i++) {
      voronoi.vps.emplace_back(VoronoiPoint(w,h));
      // cout << to_string(vps.back().p) << " -> " << to_string(vps.back().col) << endl;
    }
    return voronoi;
  }
  static Voronoi randomSampleImage(const Image& img, int cnt = 42) {
    Voronoi v = random(img.w, img.h, cnt);
    vector<int> hits(cnt);
    for(int j = 0; j < v.h; j++) {
      for(int i = 0; i < v.w; i++) {
	ivec2 p(i,j);
	auto idx = v.nearestIdx(p);
	hits[idx]++;
	v.vps[idx].col += img.pixels[i + v.w * j];
      }
    }
    for(int i = 0; i < cnt; i++) {
      v.vps[i].col /= float(hits[i]);
    }
    return v;
  }
};

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("usage %s input.ppm cnt [out.ppm]\n", argv[0]);
    printf("But you get a random voronoi Image at \"test.ppm\"\n");
    auto v = Voronoi::random(256, 256);
    auto img = v.render();
    img.save("test.ppm");
    return 0;
  }

  Image img;
  img.load(argv[1]);
  auto cnt = atoi(argv[2]);
  auto outfile = "hest.ppm";
  if (argc > 3)
    outfile = argv[3];

  auto v = Voronoi::randomSampleImage(img, 400);
  auto i = v.render();
  i.save(outfile);
  return 0;
}

