#ifndef HSL_RGB_HPP
#define HSL_RGB_HPP

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

typedef struct rgb {
  float r, g, b;
} RGB;

typedef struct hsl {
  float h, s, l;
} HSL;

HSL rgb2hsl(float r, float g, float b);
float hue2rgb(float p, float q, float t);
RGB hsl2rgb(float h, float s, float l);
RGB rotateRGBUsingHSL(int r, int g, int b, int angle);

#endif