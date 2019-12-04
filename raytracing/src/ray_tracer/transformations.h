#include "hitable.h"

class translate : public hitable
{
public:
  translate(): m_type(HITABLE_TYPE_TRANSLATE){}
  translate(hitable *p, const vec3& displacement): ptr(p), offset(displacement), m_type(HITABLE_TYPE_TRANSLATE){}
  virtual bool hit(const ray& r, float tmin, float tmax, hitRecord& rec) const;
  virtual bool boundingBox(float t0, float t1, aabb& box) const ;

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  hitable *ptr;
  vec3 offset;
  int m_type;
};

bool translate::hit(const ray&r, float tmin, float tmax, hitRecord& rec) const
{
  ray moved_r(r.origin() - offset, r.direction());
  if (ptr->hit(moved_r, tmin, tmax, rec))
  {
    rec.p += offset;
    return true;
  }
  return false;
}

bool translate::boundingBox(float t0, float t1, aabb& box) const
{
  if (ptr -> boundingBox(t0, t1, box))
  {
    box = aabb(box.min() + offset, box.max() + offset);
    return true;
  }
  return false;
}

class rotate_y : public hitable
{
public:
  rotate_y(): m_type(HITABLE_TYPE_ROTATE_Y){}
  rotate_y(hitable *p, float angle);
  virtual bool hit(const ray& r, float tmin, float tmax, hitRecord& rec) const;
  virtual bool boundingBox(float t0, float t1, aabb& box) const
  {
    box = bbox; return hasbox;
  }

  /// Return type
  virtual int GetType() { return m_type;}

  /// Custom Message serializer
  virtual void Pack (std::ostream &os);

  /// Custom message deserializer
  virtual void Unpack (std::istream &is);

  hitable *ptr;
  float sinTheta;
  float cosTheta;
  bool hasbox;
  aabb bbox;
  int m_type;
};

rotate_y::rotate_y(hitable *p, float angle) : ptr(p), m_type(HITABLE_TYPE_ROTATE_Y)
{
  float radians = (M_PI /180.0)*angle;
  sinTheta = sin(radians);
  cosTheta = cos(radians);
  hasbox = ptr -> boundingBox(0, 1, bbox);
  vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
  vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      for (int k = 0; k < 2; k++)
      {
        float x = i*bbox.max().x() + (1-i)*bbox.min().x();
        float y = j*bbox.max().y() + (1-j)*bbox.min().y();
        float z = k*bbox.max().z() + (1-k)*bbox.min().z();
        float newx = cosTheta*x + sinTheta*z;
        float newz = -sinTheta*x + cosTheta*z;
        vec3 tester(newx, y, newz);
        for (int c = 0; c < 3; c++)
        {
          if (tester[c] > max[c])
            max[c] = tester[c];
          if (tester[c] < min[c])
            min[c] = tester[c];
        }
      }
    }
  }
  bbox = aabb(min, max);
}

bool rotate_y::hit(const ray& r, float tmin, float tmax, hitRecord& rec) const
{
  vec3 origin = r.origin();
  vec3 direction = r.direction();
  origin[0] = cosTheta*r.origin()[0] - sinTheta*r.origin()[2];
  origin[2] = sinTheta*r.origin()[0] + cosTheta*r.origin()[2];
  direction[0] = cosTheta*r.direction()[0] - sinTheta*r.direction()[2];
  direction[2] = sinTheta*r.direction()[0] + cosTheta*r.direction()[2];
  ray rotated_r(origin, direction);
  if(ptr -> hit(rotated_r, tmin, tmax, rec))
  {
    vec3 p = rec.p;
    vec3 normal = rec.normal;
    p[0] = cosTheta*rec.p[0] + sinTheta*rec.p[2];
    p[2] = -sinTheta*rec.p[0] + cosTheta*rec.p[2];
    normal[0] = cosTheta*rec.normal[0] + sinTheta*rec.normal[2];
    normal[2] = -sinTheta*rec.normal[0] + cosTheta*rec.normal[2];
    rec.p = p;
    rec.normal = normal;
    return true;
  }
  return false;
}

class rotate_x : public hitable
{
public:
    rotate_x(): m_type(HITABLE_TYPE_ROTATE_X){}
    rotate_x(hitable *p, float angle);
    virtual bool hit(const ray& r, float tmin, float tmax, hitRecord& rec) const;
    virtual bool boundingBox(float t0, float t1, aabb& box) const
    {
        box = bbox; return hasbox;
    }

    /// Return type
    virtual int GetType() { return m_type;}

    /// Custom Message serializer
    virtual void Pack (std::ostream &os);

    /// Custom message deserializer
    virtual void Unpack (std::istream &is);

    hitable *ptr;
    float sinTheta;
    float cosTheta;
    bool hasbox;
    aabb bbox;
    int m_type;
};

rotate_x::rotate_x(hitable *p, float angle) : ptr(p), m_type(HITABLE_TYPE_ROTATE_Y)
{
    float radians = (M_PI /180.0)*angle;
    sinTheta = sin(radians);
    cosTheta = cos(radians);
    hasbox = ptr -> boundingBox(0, 1, bbox);
    vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                float x = i*bbox.max().x() + (1-i)*bbox.min().x();
                float y = j*bbox.max().y() + (1-j)*bbox.min().y();
                float z = k*bbox.max().z() + (1-k)*bbox.min().z();
                float newy = cosTheta*y + sinTheta*z;
                float newz = -sinTheta*y + cosTheta*z;
                vec3 tester(x, newy, newz);
                for (int c = 0; c < 3; c++)
                {
                    if (tester[c] > max[c])
                        max[c] = tester[c];
                    if (tester[c] < min[c])
                        min[c] = tester[c];
                }
            }
        }
    }
    bbox = aabb(min, max);
}

bool rotate_x::hit(const ray& r, float tmin, float tmax, hitRecord& rec) const
{
    vec3 origin = r.origin();
    vec3 direction = r.direction();
    origin[1] = cosTheta*r.origin()[1] - sinTheta*r.origin()[2];
    origin[2] = sinTheta*r.origin()[1] + cosTheta*r.origin()[2];
    direction[1] = cosTheta*r.direction()[1] - sinTheta*r.direction()[2];
    direction[2] = sinTheta*r.direction()[1] + cosTheta*r.direction()[2];
    ray rotated_r(origin, direction);
    if(ptr -> hit(rotated_r, tmin, tmax, rec))
    {
        vec3 p = rec.p;
        vec3 normal = rec.normal;
        p[1] = cosTheta*rec.p[1] + sinTheta*rec.p[2];
        p[2] = -sinTheta*rec.p[1] + cosTheta*rec.p[2];
        normal[1] = cosTheta*rec.normal[1] + sinTheta*rec.normal[2];
        normal[2] = -sinTheta*rec.normal[1] + cosTheta*rec.normal[2];
        rec.p = p;
        rec.normal = normal;
        return true;
    }
    return false;
}

class rotate_z : public hitable
{
public:
    rotate_z(): m_type(HITABLE_TYPE_ROTATE_Z){}
    rotate_z(hitable *p, float angle);
    virtual bool hit(const ray& r, float tmin, float tmax, hitRecord& rec) const;
    virtual bool boundingBox(float t0, float t1, aabb& box) const
    {
        box = bbox; return hasbox;
    }

    /// Return type
    virtual int GetType() { return m_type;}

    /// Custom Message serializer
    virtual void Pack (std::ostream &os);

    /// Custom message deserializer
    virtual void Unpack (std::istream &is);

    hitable *ptr;
    float sinTheta;
    float cosTheta;
    bool hasbox;
    aabb bbox;
    int m_type;
};

rotate_z::rotate_z(hitable *p, float angle) : ptr(p), m_type(HITABLE_TYPE_ROTATE_Y)
{
    float radians = (M_PI /180.0)*angle;
    sinTheta = sin(radians);
    cosTheta = cos(radians);
    hasbox = ptr -> boundingBox(0, 1, bbox);
    vec3 min(FLT_MAX, FLT_MAX, FLT_MAX);
    vec3 max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            for (int k = 0; k < 2; k++)
            {
                float x = i*bbox.max().x() + (1-i)*bbox.min().x();
                float y = j*bbox.max().y() + (1-j)*bbox.min().y();
                float z = k*bbox.max().z() + (1-k)*bbox.min().z();
                float newx = cosTheta*x + sinTheta*y;
                float newy = -sinTheta*x + cosTheta*y;
                vec3 tester(newx, newy, z);
                for (int c = 0; c < 3; c++)
                {
                    if (tester[c] > max[c])
                        max[c] = tester[c];
                    if (tester[c] < min[c])
                        min[c] = tester[c];
                }
            }
        }
    }
    bbox = aabb(min, max);
}

bool rotate_z::hit(const ray& r, float tmin, float tmax, hitRecord& rec) const
{
    vec3 origin = r.origin();
    vec3 direction = r.direction();
    origin[0] = cosTheta*r.origin()[0] - sinTheta*r.origin()[1];
    origin[1] = sinTheta*r.origin()[0] + cosTheta*r.origin()[1];
    direction[0] = cosTheta*r.direction()[0] - sinTheta*r.direction()[1];
    direction[1] = sinTheta*r.direction()[0] + cosTheta*r.direction()[1];
    ray rotated_r(origin, direction);
    if(ptr -> hit(rotated_r, tmin, tmax, rec))
    {
        vec3 p = rec.p;
        vec3 normal = rec.normal;
        p[0] = cosTheta*rec.p[0] + sinTheta*rec.p[1];
        p[1] = -sinTheta*rec.p[0] + cosTheta*rec.p[1];
        normal[0] = cosTheta*rec.normal[0] + sinTheta*rec.normal[1];
        normal[1] = -sinTheta*rec.normal[0] + cosTheta*rec.normal[1];
        rec.p = p;
        rec.normal = normal;
        return true;
    }
    return false;
}
