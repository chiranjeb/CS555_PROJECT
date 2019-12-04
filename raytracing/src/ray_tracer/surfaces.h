#ifndef SURFACESH
#define SURFACESH

#include "hitable.h"
#include "objParser.h"
#include "hitablelist.h"
#include <iostream>
#include <cmath>

using namespace std;

float det(const vec3 col0, const vec3 col1, const vec3 col2);

class sphere: public hitable
{
public:
  sphere():m_type(HITABLE_TYPE_SPHERE) {}
  sphere(vec3 cen, float r, material *ptr): m_type(HITABLE_TYPE_SPHERE), center(cen), radius(r), matPtr(ptr) {};
  virtual bool hit(const ray& r, float tMin, float tMax, hitRecord& rec) const;
  bool boundingBox(float t0, float t1, aabb& box) const;
  vec3 getPointOn()const
  {
    return center;// + randomOnUnitSphere()*radius;
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);
  /// Custom message deserializer
  virtual void Unpack  (std::istream &is);

  int m_type;
  vec3 center;
  float radius;
  material * matPtr;
};

bool sphere::hit(const ray& r, float tMin, float tMax, hitRecord& rec) const
{
  vec3 oc = r.origin() - center;
  float a = dot(r.direction(), r.direction());
  float b = dot(oc, r.direction());
  float c = dot(oc, oc) - radius*radius;
  float discriminant = b*b - a*c;
  if (discriminant > 0)
  {
    float temp = (-b - sqrt(b*b - a*c))/a;
    if (temp < tMax && temp > tMin)
    {
      rec.t = temp;
      rec.p = r.pointAtParameter(rec.t);
      rec.normal = (rec.p - center)/radius;
      rec.matPtr = matPtr;
      return true;
    }
    temp = (-b + sqrt(b*b - a*c))/a;
    if (temp < tMax && temp > tMin)
    {
      rec.t = temp;
      rec.p = r.pointAtParameter(rec.t);
      rec.normal = (rec.p - center)/radius;
      rec.matPtr = matPtr;
      return true;
    }
  }
  return false;
}

bool sphere::boundingBox(float t0, float t1, aabb& box)const
{
  box = aabb(center - vec3(radius, radius,radius), center + vec3(radius, radius, radius));
  return true;
}

class xy_rect: public hitable
{
public:
  xy_rect():m_type(HITABLE_TYPE_XY_RECTANGLE) {}
  xy_rect(float _x0, float _x1, float _y0, float _y1, float _k, material *mat):
          x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), matPtr(mat), m_type(HITABLE_TYPE_XY_RECTANGLE) {};


  virtual bool hit(const ray& r, float t0, float t1, hitRecord& rec) const
  {
    float t = (k - r.origin().z()) / r.direction().z();
    if (t < t0 || t > t1)
      return false;

    float x = r.origin().x() + t*r.direction().x();
    float y = r.origin().y() + t*r.direction().y();
    if (x < x0 || x > x1 || y < y0 || y > y1)
      return false;
      rec.t = t;
      rec.matPtr = matPtr;
      rec.p = r.pointAtParameter(t);
      rec.normal = vec3(0, 0, 1);
    return true;
  }

  virtual bool boundingBox(float t0, float t1, aabb& box) const
  {
    box = aabb(vec3(x0, y0, k-0.0001), vec3(x1, y1, k + 0.0001));
    return true;
  }
  vec3 getPointOn()const
  {
    return vec3(x0 + drand48()*(x1-x0), y0 + drand48()*(y1-y0), k);
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  material *matPtr;
  float x0, x1, y0, y1, k;
  int m_type;
};


class xz_rect: public hitable
{
public:
  xz_rect():m_type(HITABLE_TYPE_XZ_RECTANGLE)  {}
  xz_rect(float _x0, float _x1, float _z0, float _z1, float _k, material *mat):
          x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), matPtr(mat), m_type(HITABLE_TYPE_XZ_RECTANGLE)  {};

  virtual bool hit(const ray& r, float t0, float t1, hitRecord& rec) const
  {
    float t = (k - r.origin().y()) / r.direction().y();
    if (t < t0 || t > t1)
      return false;

    float x = r.origin().x() + t*r.direction().x();
    float z = r.origin().z() + t*r.direction().z();
    if (x < x0 || x > x1 || z < z0 || z > z1)
      return false;
      rec.t = t;
      rec.matPtr = matPtr;
      rec.p = r.pointAtParameter(t);
      rec.normal = vec3(0, 1, 0);
    return true;
  }
  virtual bool boundingBox(float t0, float t1, aabb& box) const
  {
    box = aabb(vec3(x0, k-0.0001, z0), vec3(x1, k + 0.0001, z1));
    return true;
  }
  vec3 getPointOn()const
  {
    return vec3(x0 + drand48()*(x1-x0), k, z0 + drand48()*(z1-z0));
  }
  // virtual float pdf_value(const vec3& o, const vec3& v) const
  // {
  //   hitRecord rec;
  //   if (this -> hit(ray(o, v), 0.001, FLT_MAX, rec))
  //   {
  //     float area = (x1 - x0)*(z1 -z0);
  //     float distance_squared = rec.t * rec.t *v.length2();
  //     float cosine = fabs(dot(v, rec.normal)/v.length());
  //     return distance_squared /(cosine * area);
  //   }
  //   else
  //     return 0;
  // }
  // virtual vec3 random(const vec3& o) const
  // {
  //   vec3 random_point = vec3(x0 + drand48()*(x1-x0), k, z0 + drand48()*(z1-z0));
  //   return random_point - o;
  // }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  material *matPtr;
  float x0, x1, z0, z1, k;
  int m_type;
};




class yz_rect: public hitable
{
public:
  yz_rect():m_type(HITABLE_TYPE_YZ_RECTANGLE) {}
  yz_rect(float _y0, float _y1, float _z0, float _z1, float _k, material *mat):
          y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), matPtr(mat), m_type(HITABLE_TYPE_YZ_RECTANGLE) {};

  virtual bool hit(const ray& r, float t0, float t1, hitRecord& rec) const
  {
    float t = (k - r.origin().x()) / r.direction().x();
    if (t < t0 || t > t1)
      return false;

    float y = r.origin().y() + t*r.direction().y();
    float z = r.origin().z() + t*r.direction().z();
    if (y < y0 || y > y1 || z < z0 || z > z1)
      return false;
      rec.t = t;
      rec.matPtr = matPtr;
      rec.p = r.pointAtParameter(t);
      rec.normal = vec3(1, 0, 0);
    return true;
  }
  virtual bool boundingBox(float t0, float t1, aabb& box) const
  {
    box = aabb(vec3(k-0.0001, y0, z0), vec3(k + 0.0001, y1, z1));
    return true;
  }
  vec3 getPointOn()const
  {
    return vec3(k, y0 + drand48()*(y1-y0), z0 + drand48()*(z1-z0));
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack(std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  material *matPtr;
  float y0, y1, z0, z1, k;
  int m_type;
};

class flipNormals : public hitable
{
public:
  flipNormals(): m_type(HITABLE_TYPE_FLIP_NORMALS){}
  flipNormals(hitable *p) : ptr(p), m_type(HITABLE_TYPE_FLIP_NORMALS){}

  virtual bool hit(const ray& r, float tmin, float tmax, hitRecord &rec) const
  {
    if (ptr -> hit(r, tmin, tmax, rec))
    {
      rec.normal = -rec.normal;
      return true;
    }
    else
      return false;
  }
  virtual bool boundingBox(float t0, float t1, aabb& box) const
  {
    return ptr->boundingBox(t0, t1, box);
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack(std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  hitable *ptr;
  int m_type;
};

class box : public hitable
{
public:
  box():m_type(HITABLE_TYPE_BOX){}
  box(const vec3& p0, const vec3& p1, material *ptr);
  virtual bool hit(const ray& r, float t0, float t1, hitRecord& rec) const;
  virtual bool boundingBox(float t0, float t1, aabb& box) const
  {
    box = aabb(pmin, pmax);
    return true;
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  vec3 pmin, pmax;
  hitable *listPtr;
  int m_type;
};

box::box(const vec3& p0, const vec3& p1, material *ptr): m_type(HITABLE_TYPE_BOX)
{
  pmin = p0;
  pmax = p1;
  hitable **list = new hitable*[6];
  list[0] = new xy_rect(p0.x(), p1.x(), p0.y(), p1.y(), p1.z(), ptr);
  list[1] = new flipNormals(new xy_rect(p0.x(), p1.x(), p0.y(), p1.y(), p0.z(), ptr));
  list[2] = new xz_rect(p0.x(), p1.x(), p0.z(), p1.z(), p1.y(), ptr);
  list[3] = new flipNormals(new xz_rect(p0.x(), p1.x(), p0.z(), p1.z(), p0.y(), ptr));
  list[4] = new yz_rect(p0.y(), p1.y(), p0.z(), p1.z(), p1.x(), ptr);
  list[5] = new flipNormals(new yz_rect(p0.y(), p1.y(), p0.z(), p1.z(), p0.x(), ptr));
  listPtr = new hitableList(list, 6);
}
bool box::hit(const ray& r, float t0, float t1, hitRecord& rec) const
{
  return listPtr -> hit(r, t0, t1, rec);
}


class triangle : public hitable
{
public:
  triangle(): m_type(HITABLE_TYPE_TRIANGLE) {}
  triangle(const vec3& p0, const vec3& p1, const vec3& p2, material* ptr) : a(p0), b(p1), c(p2), matPtr(ptr), m_type(HITABLE_TYPE_TRIANGLE) {};
  virtual bool hit(const ray& r, float t0, float t1, hitRecord& rec) const;
  virtual bool boundingBox(float t0, float t1, aabb& box) const
  {
    vec3 pmin = vec3(fmin(a.x(), fmin(b.x(), c.x())) - 0.0001,
                     fmin(a.y(), fmin(b.y(), c.y())) - 0.0001,
                     fmin(a.z(), fmin(b.z(), c.z())) - 0.0001);

    vec3 pmax = vec3(fmax(a.x(), fmax(b.x(), c.x())) + 0.0001,
                     fmax(a.y(), fmax(b.y(), c.y())) + 0.0001,
                     fmax(a.z(), fmax(b.z(), c.z())) + 0.0001);
    box = aabb(pmin, pmax);
    return false;
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  vec3 a, b, c, normal = vec3(0,0,0);
  material *matPtr;
  int m_type;
};

bool triangle::hit(const ray&r, float t0, float t1, hitRecord &rec) const
{
  vec3 aminusb = a - b;
  vec3 aminusc = a - c;
  vec3 aminuso = a - r.origin();

  float detA = det(aminusb, aminusc, r.direction());

  float t = det(aminusb, aminusc, aminuso)/detA;

  if (t < t0 || t > t1)
    return false;

  float gamma = det(aminusb, aminuso, r.direction())/detA;
  if (gamma < 0 || gamma > 1)
    return false;

  float beta = det(aminuso, aminusc, r.direction())/detA;
  if (beta < 0 or beta > 1-gamma)
    return false;

  rec.t = t;
  rec.p = r.pointAtParameter(t);
  rec.matPtr = matPtr;
  if (normal.x() == 0 and normal.y() == 0 and normal.z() == 0)
    rec.normal = unitVector(cross(b-a, c-a));

  if (dot(rec.normal, r.direction()) > 0)
    rec.normal *= -1.0;

  return true;

}


// This hitable is unimplemented.  
class trianglemesh : public hitable
{
public:
  trianglemesh(): m_type(HITABLE_TYPE_TRIANGLE_MESH)  {}
  trianglemesh(char file[], material *ptr, float scale, const vec3& translate): m_type(HITABLE_TYPE_TRIANGLE_MESH) 
  {
    objl::Loader loader;
    loader.LoadFile(file);
    cout << "\n\n" << loader.LoadedMeshes[0].MeshName << endl << endl;

    for (int i = 0; i < FLT_MAX; i++);
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack(std::ostream &os)
  {
     //This hitable is unimplemented at the moment.
  }

  /// Custom message deserializer
  virtual void Unpack(std::istream &is)
  {
     //This hitable is unimplemented at the moment.
  }

  virtual bool hit(const ray& r, float tmin, float tmax, hitRecord& rec) const {return false;}
  virtual bool boundingBox(float t0, float t1, aabb& box) const {return false;}
  int m_type;
};



float det(const vec3 col0, const vec3 col1, const vec3 col2)
{
  return col0.x()*(col1.y()*col2.z() - col2.y()*col1.z()) -
         col1.x()*(col0.y()*col2.z() - col2.y()*col0.z()) +
         col2.x()*(col0.y()*col1.z() - col1.y()*col0.z());
}
#endif
