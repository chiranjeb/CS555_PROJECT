#include "perlin.h"

enum TextureType_t
{
   TEXTURE_TYPE_CONSTANT_TEXTURE = 0x01,
   TEXTURE_TYPE_CHECKER_TEXTURE,
   TEXTURE_TYPE_CHECKER_NOISE,
   TEXTURE_TYPE_CAMO,
   TEXTURE_TYPE_MARBLE,
};

class texture
{
public:
  virtual vec3 value(float u, float v, const vec3 &p) const = 0;
  virtual char GetType() = 0;
  virtual void Pack (std::ostream &os)=0;
  virtual void Unpack (std::istream &is)=0;
};

class constantTexture : public texture
{
public:
  constantTexture() : m_type(TEXTURE_TYPE_CONSTANT_TEXTURE) {}
  constantTexture(vec3 c) : color(c), m_type(TEXTURE_TYPE_CONSTANT_TEXTURE) {}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color;
  }

  /// Return type
  virtual char GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  vec3 color;
  char m_type;
};

class checkerTexture : public texture
{
public:
  checkerTexture() : m_type(TEXTURE_TYPE_CHECKER_TEXTURE) {}
  checkerTexture(texture *t0, texture *t1): even(t0), odd(t1),  m_type(TEXTURE_TYPE_CHECKER_TEXTURE){}
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

  /// Return type
  virtual char GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  texture *odd;
  texture *even;
  char m_type;
};

class checkerNoise : public texture
{
public:
  checkerNoise(): m_type(TEXTURE_TYPE_CHECKER_NOISE){}
  checkerNoise(vec3 c, float sc = 1.0) : color(c), scale(sc), m_type(TEXTURE_TYPE_CHECKER_NOISE) {}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color * vec3(1, 1, 1) * noise.noise0(p);
  }

  /// Return type
  virtual char GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  vec3 color;
  float scale;
  perlin noise;

  char m_type;
};

class camo : public texture
{
public:
  camo():m_type(TEXTURE_TYPE_CAMO) {}
  camo(vec3 c, float sc = 1.0): color(c), scale(sc), m_type(TEXTURE_TYPE_CAMO){}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color*vec3(1,1,1)*noise.turb(scale *p);
  }

  /// Return type
  virtual char GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  vec3 color;
  float scale;
  perlin noise;

  char m_type;
};

class marble : public texture
{
public:
  marble():m_type(TEXTURE_TYPE_MARBLE) {}
  marble(vec3 c, float sc = 1.0): color(c), scale(sc), m_type(TEXTURE_TYPE_MARBLE){}
  virtual vec3 value(float u, float v, const vec3& p) const
  {
    return color*vec3(1,1,1)*0.5*(1+sin(scale*p.x() + 5*noise.turb(scale*p)));
  }

  /// Return type
  virtual char GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  vec3 color;
  float scale;
  perlin noise;

  char m_type;
};
