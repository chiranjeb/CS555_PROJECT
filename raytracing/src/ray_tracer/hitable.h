#ifndef HITABLEH
#define HITABLEH

#include "ray.h"
#include "surfaces.h"

class material;

struct hitRecord
{
  float t;
  vec3 p;
  vec3 normal;
  material *matPtr;
};
int box_x_compare(const void *a, const void *b);
int box_y_compare(const void *a, const void *b);
int box_z_compare(const void *a, const void *b);

class aabb {
public:
  aabb() {}
  aabb(const vec3& a, const vec3& b) { _min = a; _max = b;}

  vec3 min() const {return _min;}
  vec3 max() const {return _max;}

  int longestAxis() const
  {
    float longest = -FLT_MAX;
    int axis = 0;

    for(int i = 0; i < 3; i++)
    {
      if (abs(max()[i] - min()[i]) > longest)
      {
        longest = abs(max()[i] - min()[i]);
        axis = i;
      }
    }
    return axis;
  }
  float surfaceArea() const
  {
    return 2*abs(max()[0] - min()[0])*abs(max()[1] - min()[1]) +
           2*abs(max()[0] - min()[0])*abs(max()[2] - min()[2]) +
           2*abs(max()[2] - min()[2])*abs(max()[1] - min()[1]);
  }

  bool hit(const ray& r, float tmin, float tmax) const
  {
    for(int a = 0; a < 3; a++)
    {
      float invD = 1.0f / r.direction()[a];
      float t0 = (min()[a] - r.origin()[a])*invD;
      float t1 = (max()[a] - r.origin()[a])*invD;
      if(invD < 0.0f)
      {
        float temp = t0;
        t0 = t1;
        t1 = temp;
      }
      tmin = t0 > tmin ? t0 : tmin;
      tmax = t1 < tmax ? t1 : tmax;
      if( tmax <= tmin)
        return false;
    }
    return true;
  }

  vec3 _min;
  vec3 _max;
};

aabb surroundingBox(aabb box0, aabb box1)
{
  vec3 small(fmin(box0.min().x(), box1.min().x()),
             fmin(box0.min().y(), box1.min().y()),
             fmin(box0.min().z(), box1.min().z()));

  vec3 big(fmax(box0.max().x(), box1.max().x()),
             fmax(box0.max().y(), box1.max().y()),
             fmax(box0.max().z(), box1.max().z()));

  return aabb(small, big);
}


class hitable
{
public:
  virtual bool hit(const ray& r, float tMin, float tMax, hitRecord& rec) const = 0;
  virtual bool boundingBox(float t0, float t1, aabb& box) const = 0;
};

class bvh : public hitable
{
public:
  bvh(){}
  bvh(hitable **l, int n, float time0, float time1);
  virtual bool hit(const ray& r, float tmin, float tmax, hitRecord& rec) const;
  virtual bool boundingBox(float t0, float t1, aabb& box) const;
  hitable *left;
  hitable *right;
  aabb box;
};
bvh::bvh(hitable **l, int n, float time0, float time1)
{
  aabb *boxes = new aabb[n];
  float *left_area = new float[n];
  float *right_area = new float[n];
  aabb main_box;
  bool dummy = l[0] -> boundingBox(time0, time1, main_box);
  for(int i = 1; i < n; i++)
  {
    aabb new_box;
    bool dummy = l[i] -> boundingBox(time0, time1, new_box);
    main_box = surroundingBox(new_box, main_box);
  }
  int axis = main_box.longestAxis();
  if (axis == 0)
    qsort(l, n, sizeof(hitable*), box_x_compare);
  else if (axis == 1)
    qsort(l, n, sizeof(hitable*), box_y_compare);
  else
    qsort(l, n, sizeof(hitable*), box_z_compare);

  for(int i = 1; i < n; i++)
    bool dummy = l[i] -> boundingBox(time0, time1, boxes[i]);
  left_area[0] = boxes[0].surfaceArea();
  aabb left_box = boxes[0];
  for (int i = 0; i < n - 1; i++)
  {
    left_box = surroundingBox(left_box, boxes[i]);
    left_area[i] = left_box.surfaceArea();
  }
  right_area[n-1] = boxes[n-1].surfaceArea();
  aabb right_box = boxes[n-1];
  for (int i = n -2; i > 0; i--)
  {
    right_box = surroundingBox(right_box, boxes[i]);
    right_area[i] = right_box.surfaceArea();
  }
  float min_SAH = FLT_MAX;
  int min_SAH_idx;
  for(int i = 0; i < n-1; i++)
  {
    float SAH = i*left_area[i] + (n-i-1)*right_area[i+1];
    if (SAH < min_SAH)
    {
      min_SAH_idx = i;
      min_SAH = SAH;
    }
  }
  if(min_SAH_idx == 0)
    left = l[0];
  else
    left = new bvh(l, min_SAH_idx+1, time0, time1);
  if(min_SAH_idx == n-2)
    right = l[min_SAH_idx+1];
  else
    right = new bvh(l + min_SAH_idx +1, n - min_SAH_idx -1, time0, time1);

  box = main_box;
}
bool bvh::boundingBox(float t0, float t1, aabb& b) const
{
  b = box;
  return true;
}
bool bvh::hit(const ray& r, float tmin, float tmax, hitRecord& rec) const
{
  if(box.hit(r, tmin, tmax))
  {
    hitRecord left_rec, right_rec;
    bool hit_left = left -> hit(r, tmin, tmax, left_rec);
    bool hit_right = right -> hit(r, tmin, tmax, right_rec);
    if(hit_left && hit_right)
    {
      if (left_rec.t < right_rec.t)
        rec = left_rec;
      else
        rec = right_rec;
      return true;
    }
    else if (hit_left)
    {
      rec = left_rec;
      return true;
    }
    else if (hit_right)
    {
      rec = right_rec;
      return true;
    }
    else
      return false;
  }
  return false;
}

int box_x_compare(const void *a, const void *b)
{
  aabb box_left, box_right;
  hitable *ah = *(hitable**)a;
  hitable *bh = *(hitable**)b;
  if (box_left.min().x() - box_right.min().x() < 0.0 )
    return -1;

  else
    return 1;
}

int box_y_compare(const void *a, const void *b)
{
  aabb box_left, box_right;
  hitable *ah = *(hitable**)a;
  hitable *bh = *(hitable**)b;
  if (box_left.min().y() - box_right.min().y() < 0.0 )
    return -1;

  else
    return 1;
}

int box_z_compare(const void *a, const void *b)
{
  aabb box_left, box_right;
  hitable *ah = *(hitable**)a;
  hitable *bh = *(hitable**)b;
  if (box_left.min().z() - box_right.min().z() < 0.0 )
    return -1;

  else
    return 1;
}

#endif