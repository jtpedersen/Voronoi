#ifndef IMAGE_H_
#define IMAGE_H_

#include "common.h"
#include <array>

class Image {
public:
    int w,h;
    vector<vec3> pixels;

    Image();
    Image(int w, int h);
    ///save a P6 file
    void save(string filename);
    ///load a P6 file
    void load(string filename);
    /// return a edge detected version
    Image edgy();
    vec3 convolve(int x, int y, const array<float, 9>& kernel);
};

#endif /* !IMAGE_H_ */
