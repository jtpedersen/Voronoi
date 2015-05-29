#include "Image.h"

Image::Image() : w(0), h(0) {}

Image::Image(int w, int h) 
    : w(w), h(h), pixels(w*h) { }

void Image::save(string filename) {
    ofstream ofs(filename);
    ofs << "P6\n" << w <<  " " << h << "\n255\n";
    for(int i = 0; i < w*h; i++) {
	const vec3& pixel = pixels[i];
	ofs << static_cast<unsigned char>(pixel.r * 255);
	ofs << static_cast<unsigned char>(pixel.g * 255);
	ofs << static_cast<unsigned char>(pixel.b * 255);
    }
}
void Image::load(string filename) {
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

vec3 Image::convolve(int x, int y, const array<float, 9>& kernel) {
  vec3 res(0);
  for(int j = - 1; j < 2; j++) {
    for(int i = - 1; i < 2; i++) {
      res += kernel[j*3 + i] * pixels[(y+j) * w + (i + x)];
    }
  }  
  return res*res;
}


Image Image::edgy() {
  
  array<float, 9> kernel = { -1, -1, -1, 
			     -1, 8, -1, 
			     -1, -1, -1};
  Image res(w,h);

  for(int j = 1; j < h-1; j++) {
    for(int i = 1; i < w-1; i++) {
      auto idx = j * w + i;
      res.pixels[idx] = convolve(i,j, kernel);
    }
  }

  // edges of image
  for(int x = 0; x < w; x++) {
    res.pixels[x] = vec3(0);
    res.pixels[(h-1) * w + x] = vec3(0);
  }
  for(int y = 0; y < h; y++) {
    res.pixels[y*w] = vec3(0);
    res.pixels[(w-1) +  (w * y)] = vec3(0);
  }

  return res;
  
}
