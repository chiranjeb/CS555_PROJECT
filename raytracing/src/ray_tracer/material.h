#ifndef MATERIALH
#define MATERIALH
#include "hitable.h"
#include "texture.h"
#include "onb.h"

vec3 reflect(const vec3& v, const vec3& n);
bool refract(const vec3& v, const vec3& n, float niOverNt, vec3& refracted);
float schlick(float cosine, float refIdx);
vec3 randomInUnitSphere();
vec3 randomOnUnitSphere();
inline vec3 random_cosine_direction();

enum MaterialType_t
{
   MATERIAL_TYPE_LAMBERTIAN = 0x01,
   MATERIAL_TYPE_METAL,
   MATERIAL_TYPE_DIELECTRIC,
   MATERIAL_TYPE_DIFUSE_LIGHT,
};

class material
{
public:
   virtual bool scatter(const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const { return false;}
   virtual vec3 emitted(float u, float v, const vec3& p) const { return vec3(0, 0, 0);}
   virtual char GetType()=0;
   virtual void Pack(std::ostream& os) = 0;
   virtual void Unpack(std::istream& is) = 0;
};

class lambertian : public material
{
public:
   lambertian() : m_type(MATERIAL_TYPE_LAMBERTIAN) { }
   lambertian(texture *a) : m_type(MATERIAL_TYPE_LAMBERTIAN), albedo(a) { }

   virtual bool scatter(const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const
   {
      vec3 target = rec.p + rec.normal + randomInUnitSphere();
      scattered = ray(rec.p, target - rec.p);
      attenuation = albedo->value(0, 0, rec.p);
      return true;
   }

   /// Return type
   virtual char GetType() { return m_type;}

   /// Custom Message serializer
   void Pack(std::ostream& os);

   /// Custom message deserializer
   void Unpack(std::istream& is);


   char m_type;
   texture *albedo;
};

class metal : public material
{
public:
   metal() : m_type(MATERIAL_TYPE_METAL) { }
   metal(texture *a, float f) : m_type(MATERIAL_TYPE_METAL), albedo(a) { if (f < 1) fuzz = f;
      else fuzz = 1;}
   virtual bool scatter(const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const
   {
      vec3 reflected = reflect(unitVector(rIn.direction()), rec.normal);
      scattered = ray(rec.p, reflected + fuzz * randomInUnitSphere());
      attenuation = albedo->value(0, 0, rec.p);
      return (dot(scattered.direction(), rec.normal) > 0);
   }

   /// Return type
   virtual char GetType() { return m_type;}

   /// Custom Message serializer
   virtual void Pack(std::ostream& os);

   ///  Custom message deserializer
   virtual void Unpack(std::istream& is);

   char m_type;
   texture *albedo;
   float fuzz;
};

class dielectric : public material
{
public:
   dielectric() : m_type(MATERIAL_TYPE_DIELECTRIC) { }
   dielectric(texture *a, float ri) : m_type(MATERIAL_TYPE_DIELECTRIC), albedo(a), refIdx(ri) { }
   virtual bool scatter(const ray& rIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const
   {
      vec3 outwardNormal;
      vec3 reflected = reflect(rIn.direction(), rec.normal);
      float niOverNt;
      attenuation = albedo->value(0, 0, rec.p);
      vec3 refracted;
      float reflectProb;
      float cosine;
      if (dot(rIn.direction(), rec.normal) > 0)
      {
         outwardNormal = -rec.normal;
         niOverNt = refIdx;
         // cosine = refIdx * dot(rIn.direction(), rec.normal) / rIn.direction().length();
         cosine = dot(rIn.direction(), rec.normal) / rIn.direction().length();
         cosine = sqrt(1 - refIdx * refIdx * (1 - cosine * cosine));
      }
      else
      {
         outwardNormal = rec.normal;
         niOverNt = 1.0 / refIdx;
         cosine = -dot(rIn.direction(), rec.normal) / rIn.direction().length();
      }
      if (refract(rIn.direction(), outwardNormal, niOverNt, refracted))
      {
         reflectProb = schlick(cosine, refIdx);
      }
      else
      {
         reflectProb = 1.0;
      }
      if (drand48() < reflectProb)
      {
         scattered = ray(rec.p, reflected);
      }
      else
      {
         scattered = ray(rec.p, refracted);
      }
      return true;
   }

   /// Return type
   virtual char GetType() { return m_type;}

   /// Custom Message serializer
   virtual void Pack(std::ostream& os);

   /// Custom message deserializer
   virtual void Unpack(std::istream& is);

   char m_type;
   texture *albedo;
   float refIdx;
};

class diffuseLight : public material
{
public:
   diffuseLight() : m_type(MATERIAL_TYPE_DIFUSE_LIGHT) { }
   diffuseLight(texture *a) : m_type(MATERIAL_TYPE_DIFUSE_LIGHT), emit(a) { }
   virtual bool scatter(const ray& r_in, const hitRecord& rec, vec3& attenuation, ray& scattered) const { return false;}
   virtual vec3 emitted(float u, float v, const vec3& p) const
   {
      return emit->value(u, v, p);
   }

   /// Return type
   virtual char GetType() { return m_type;}

   /// Custom Message serializer
   virtual void Pack(std::ostream& os);

   ///  Custom message deserializer
   virtual void Unpack(std::istream& is); 

   char m_type;
   texture *emit;
};


vec3 reflect(const vec3& v, const vec3& n)
{
   return v - 2 * dot(v, n) * n;
}

bool refract(const vec3& v, const vec3& n, float niOverNt, vec3& refracted)
{
   vec3 uv = unitVector(v);
   float dt = dot(uv, n);
   float discriminant = 1.0 - niOverNt * niOverNt * (1 - dt * dt);
   if (discriminant > 0)
   {
      refracted = niOverNt * (uv - n * dt) - n * sqrt(discriminant);
      return true;
   }
   else
   {
      return false;
   }
}

float schlick(float cosine, float refIdx)
{
   float r0 = (1 - refIdx) / (1 + refIdx);
   r0 = r0 * r0;
   return r0 + (1 - r0) * pow((1 - cosine), 5);
}

vec3 randomInUnitSphere()
{
   vec3 p;
  do{
      p = 2.0 * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
  }while (p.length2() >= 1.0);
   return p;
}

vec3 randomOnUnitSphere()
{
   vec3 p;
  do{
      p = 2.0 * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
  }while (p.length2() >= 1.0);
   return unitVector(p);
}

inline vec3 random_cosine_direction()
{
   float r1 = drand48();
   float r2 = drand48();
   float z = sqrt(1 - r2);
   float phi = 2 * M_PI * r1;
   float x = cos(phi) * 2 * sqrt(r2);
   float y = sin(phi) * 2 * sqrt(r2);
   return vec3(x, y, z);
}

#endif
