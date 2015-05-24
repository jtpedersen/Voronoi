#ifndef IMAGE_H_
#define IMAGE_H_

#include "common.h"

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
};

#endif /* !IMAGE_H_ */
