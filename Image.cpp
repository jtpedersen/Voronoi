#include "Image.h"

Image::Image() : w(0), h(0) {};

Image::Image(int w, int h) 
    : w(w), h(h), pixels(w*h) { };

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
