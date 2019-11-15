#include "perlin.h"

class texture
{
public:
  virtual vec3 value(float u, float v, const vec3 &p) const = 0;
};

class constantTexture : public texture
{
public:
  constantTexture() {}
  constantTexture(vec3 c) : color(c) {}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color;
  }
  vec3 color;
};

class checkerTexture : public texture
{
public:
  checkerTexture() {}
  checkerTexture(texture *t0, texture *t1): even(t0), odd(t1){}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    float sines = sin(10*p.x()) * sin(10*p.y()) * sin(10*p.z());
    if (sines < 0)
    {
      return odd -> value(u, v, p);
    }
    else
    {
      return even -> value(u, v, p);
    }
  }
  texture *odd;
  texture *even;
};

class checkerNoise : public texture
{
public:
  checkerNoise() {}
  checkerNoise(vec3 c, float sc = 1.0) : color(c), scale(sc) {}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color * vec3(1, 1, 1) * noise.noise0(p);
  }
  vec3 color;
  float scale;
  perlin noise;
};

class camo : public texture
{
public:
  camo() {}
  camo(vec3 c, float sc = 1.0): color(c), scale(sc){}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color*vec3(1,1,1)*noise.turb(scale *p);
  }
  vec3 color;
  float scale;
  perlin noise;
};

class marble : public texture
{
public:
  marble() {}
  marble(vec3 c, float sc = 1.0): color(c), scale(sc){}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color*vec3(1,1,1)*0.5*(1+sin(scale*p.x() + 10*noise.turb(p)));
  }
  vec3 color;
  float scale;
  perlin noise;
};
