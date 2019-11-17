#ifndef CAMERAH
#define CAMERAH

#include "ray.h"
vec3 randomInUnitDisk();


class camera
{
public:
  camera(){}
  camera(vec3 lookfrom, vec3 lookat, vec3 vup, float vfov, float aspect, float aperture, float focusDist)
  {
    lensRadius = aperture/2;
    float theta = vfov*M_PI/180;
    float halfHeight = tan(theta/2);
    float halfWidth = aspect*halfHeight;
    origin = lookfrom;
    w = unitVector(lookfrom - lookat);
    u = unitVector(cross(vup, w));
    v = cross(w, u);
    lowerLeftCorner = origin - halfWidth*focusDist*u - halfHeight*focusDist*v - focusDist*w;
    horizontal = 2*halfWidth*focusDist*u;
    vertical = 2*halfHeight*focusDist*v;

  }
  ray getRay(float s, float t)
  {
    vec3 rd = lensRadius*randomInUnitDisk();
    vec3 offset = u*rd.x() + v*rd.y();
    return ray(origin + offset, lowerLeftCorner+s*horizontal + t*vertical - origin - offset);
  }


  friend std::ostream& operator << (std::ostream &os, camera &cam);
  friend std::istream& operator >> (std::istream &is, camera &cam);
  vec3 origin;
  vec3 lowerLeftCorner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
  float lensRadius;
};

/// Custom serializer
std::ostream& operator << (std::ostream &os, camera &cam);

/// Custom deserializer
std::istream& operator >> (std::istream &is, camera &cam);

inline vec3 randomInUnitDisk()
{
  vec3 p;
  do{
    p = 2.0*vec3(drand48(), drand48(), 0) - vec3(1, 1, 0);
  } while (dot(p,p) >= 1.0);
  return p;
}
#endif
