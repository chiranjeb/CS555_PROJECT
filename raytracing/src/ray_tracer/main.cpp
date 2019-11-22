#include <iostream>
#include "surfaces.h"
#include "hitablelist.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "scene.h"
#include "scene_descriptor.hpp"


using namespace std;
vec3 color(const ray& r, hitable *world, int depth, vec3& curAlbedo);
vec3 randomInUnitSphere();

#if 0
int main()
{}
#endif

int ray_tracer_produce_scene()
{
   int nx, ny, ns;
   camera cam;
   vec3 curAlbedo;

   hitable *world = myScene(cam, nx, ny, ns);
   cout << "P3\n" << nx << " " << ny << "\n255\n";
   for (int j = ny - 1; j >= 0; j--)
   {
      for (int i = 0; i < nx; i++)
      {
         vec3 col(0, 0, 0);
#pragma omp parallel for schedule(static, 10)
         for (int s = 0; s < ns; s++)
         {
            float u = float(i + drand48()) / float(nx);
            float v = float(j + drand48()) / float(ny);
            ray r = cam.getRay(u, v);
            vec3 curAlbedo(1.0, 1.0, 1.0);
            col += color(r, world, 0, curAlbedo);
         }
         col /= float(ns);
         col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
         if (col[0] > 1 || col[2] > 1 || col[3] > 1) cout << "there is a problem here: " << i << endl;
         int ir = int(255.99*col[0]);
         int ig = int(255.99*col[1]);
         int ib = int(255.99*col[2]);
         cout << ir << " " << ig << " " << ib << "\n";
      }
   }
   return 0;
}

vec3 color(const ray& r, hitable *world, int depth, vec3& curAlbedo)
{
   hitRecord rec;

   if (world->hit(r, 0.001, FLT_MAX, rec))
   {
      ray scattered;
      vec3 attenuation;
      vec3 emitted = rec.matPtr->emitted(0, 0, rec.p);
      if (depth < 50 && rec.matPtr->scatter(r, rec, attenuation, scattered))
      {
         curAlbedo *= attenuation;
         float p = curAlbedo.max();
         if (drand48() > p)
         {
            return vec3(0, 0, 0);
         }
         curAlbedo *= 1 / p;
         return emitted + color(scattered, world, depth + 1, curAlbedo);
      }
      else
      {
         return emitted * curAlbedo;
      }

   }
   else
   {
      //return vec3(0, 0, 0);
      vec3 unitDirection = unitVector(r.direction());
      float t = 0.5 * (unitDirection.y() + 1.0);
      return curAlbedo * ((1.0 - t) * vec3(1.0, 1.0, 1.0) + t * vec3(0.5, 0.7, 1.0));
   }
}



void ProducePixels(uint32_t NY_end, uint32_t NY_start, uint32_t NX_end, uint32_t NX_start,
                   uint32_t nx, uint32_t ny, uint32_t ns, 
                   camera *p_camera, hitable* world, std::ostream &os)
{
   DEBUG_TRACE("Worker::OnPixelProduceRequestMsg: ");
   vec3 curAlbedo;
   for (int j = NY_end; j >= NY_start; j--)
   {
      for (int i = 0; i < NX_end; ++i)
      {
         vec3 col(0, 0, 0);
         for (int s = 0; s < 20; s++)
         {
            float u = float(i + drand48()) / float(nx);
            float v = float(j + drand48()) / float(ny);
            ray r = p_camera->getRay(u, v);
            vec3 curAlbedo(1.0, 1.0, 1.0);
            col += color(r, world, 0, curAlbedo);
         }
         col /= float(ns);
         col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
         if (col[0] > 1 || col[2] > 1 || col[3] > 1) cout << "there is a problem here: " << i << endl;
         int ir = int(255.99*col[0]);
         int ig = int(255.99*col[1]);
         int ib = int(255.99*col[2]);
         os << ir << " " << ig << " " << ib << "\n";
      }
   }
}



/// Custom Message serializer
std::ostream& operator << (std::ostream &os, aabb &ab)
{
   os << ab._min << " " << ab._max << " ";
   return os;
}

///  Custom message deserializer
std::istream& operator >> (std::istream &is, aabb &ab)
{
   is >> ab._min >> ab._max;
   return is;
}

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

// vec3 color(const ray& r, hitable* world, hitable* lights, hitable* nonLights, int depth, vec3& curAlbedo)
// {
//   hitRecord rec, recShadow;
//   if (world -> hit(r, 0.001, FLT_MAX, rec))
//   {
//     ray scattered;
//     vec3 attenuation;
//     vec3 emitted = rec.matPtr -> emitted(0, 0, rec.p);
//     if (rec.matPtr -> scatter(r, rec, attenuation, scattered))
//     {
//       curAlbedo *= attenuation;
//       vec3 lightSum(0.0, 0.0, 0.0);
//       if (dynamic_cast<const lambertian*>(rec.matPtr) != NULL)
//       {
//         for( int i = 0; i < lights -> listSize; i++)
//         {
//           vec3 L_m = lights -> list[i].getPointOn() - rec.p;
//           float distanceToL = L_m.length();
//           vec3 dirToL = unitVector(L_m);
//           ray shadowRay(rec.p, dirToL);
//           if (nonLights -> hit(r, 0.001, distanceToL, recShadow))
//           {
//             if (recShadow.t + 0.001 < distanceToL)
//               continue;
//           }
//           float normalDotL = dot(dirToL, rec.normal)
//           if (normalDotL > 0)
//           {
//             lightSum += normalDotL*curAlbedo*lights->list[i].matPtr->emitted(0, 0, rec.p);
//           }
//         }
//         lightSum = vec3(min(lightSum[0], 1.0),min(lightSum[1], 1.0),min(lightSum[2], 1.0));
//       }
//       if (depth < 2)
//       {
//         if (dynamic_cast<const lambertian*>(rec.matPtr) != NULL)
//         {
//           if(drand48() < (lightSum[0]+lightSum[1]+lightSum[2])/3)
//             return lightSum;
//           else
//             return color(scattered, world, lights, nonLights, depth+1, curAlbedo);
//         }
//         else
//           return color(scattered, world, lights, nonLights, depth+1, curAlbedo);
//       }
//       return lightSum;
//     }
//     return curAlbedo*emitted;
//   }
//   else
//   {
//     //return vec3(0, 0, 0);
//     vec3 unitDirection = unitVector(r.direction());
//     float t = 0.5*(unitDirection.y() + 1.0);
//     return curAlbedo*((1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0));
//   }
// }
material* ConstructMaterial(std::istream& is, char type)
{
   material *ptr;
   switch (type)
   {
      case MATERIAL_TYPE_LAMBERTIAN:
         {
            ptr =  new lambertian();
            break;
         }
      case MATERIAL_TYPE_METAL:
         {
            ptr =  new metal();
            break;
         }
      case MATERIAL_TYPE_DIELECTRIC:
         {
            ptr =  new dielectric();
            break;
         }
      case MATERIAL_TYPE_DIFUSE_LIGHT:
         {
            ptr =  new diffuseLight();
            break;
         }
      default:
         ptr = nullptr;
         break;
   }
   ptr->Unpack(is);
   return ptr;
}



hitable* ConstructHitable(std::istream& is, char type)
{
   hitable *ptr;
   switch (type)
   {
      case HITABLE_TYPE_LIST:
         {
             ptr = new hitableList();
             break;
         }
      case HITABLE_TYPE_BVH:
         {
            ptr =  new bvh();
            break;
         }

      case HITABLE_TYPE_SPEHERE:
         {
            ptr =  new sphere();
            break;
         }

      case HITABLE_TYPE_XY_RECTANGLE:
         {
            ptr =  new xy_rect();
            break;
         }
      case HITABLE_TYPE_XZ_RECTANGLE:
         {
            ptr =  new xz_rect();
            break;
         }
      case HITABLE_TYPE_YZ_RECTANGLE:
         {
            ptr =  new yz_rect();
            break;
         }
      case HITABLE_TYPE_FLIP_NORMALS:
         {
            ptr =  new flipNormals();
            break;
         }
      case HITABLE_TYPE_BOX:
         {
            ptr =  new box();
            break;
         }
      case HITABLE_TYPE_TRIANGLE:
         {
            ptr =  new triangle();
            break;
         }
      case HITABLE_TYPE_TRIANGLE_MESH:
         {
            ptr =  new trianglemesh();
            break;
         }
      case HITABLE_TYPE_TRANSLATE:
         {
            ptr =  new translate();
            break;
         }
      case HITABLE_TYPE_ROTATE_Y:
         {
            ptr =  new rotate_y();
            break;
         }
      default:
         return nullptr;
   }
   ptr->Unpack(is);
   return ptr;
}

texture* ConstructTexture(std::istream& is, uint16_t type)
{
   texture *ptr;
   switch (type)
   {
      case TEXTURE_TYPE_CONSTANT_TEXTURE:
         ptr = new constantTexture(); 
         break;
      case TEXTURE_TYPE_CHECKER_TEXTURE:
         ptr = new checkerTexture(); 
         break;
      case TEXTURE_TYPE_CHECKER_NOISE:
         ptr = new checkerNoise(); 
         break;
      case TEXTURE_TYPE_CAMO:
         ptr = new camo(); 
         break;
      case TEXTURE_TYPE_MARBLE:
         ptr = new marble(); 
         break;
      default:
         return nullptr;
   }
   ptr->Unpack(is);
   return ptr;
}


/// Custom serializer
std::ostream& operator << (std::ostream &os, camera &cam)
{
   os << cam.origin   << " " 
      << cam.lowerLeftCorner << " " 
      << cam.horizontal << " " 
      << cam.vertical << " " 
      << cam.u << " " 
      << cam.v << " " 
      << cam.w << " " 
      << cam.lensRadius << " ";
   return os;
}

/// Custom deserializer
std::istream& operator >> (std::istream &is, camera &cam)
{
   is >> cam.origin >> cam.lowerLeftCorner 
      >> cam.horizontal >> cam.vertical 
      >> cam.u >> cam.v >> cam.w >> cam.lensRadius ;
   return is;
}


void sphere::Pack(std::ostream& os)
{
   DEBUG_TRACE_VERBOSE(", sphere::Pack:" << " ");
   os << center << " " << radius << " ";
   os << matPtr->GetType() << " ";
   DEBUG_TRACE_VERBOSE(center << " " << radius << " type: " << std::hex << int(matPtr->GetType()) << ",");
   matPtr->Pack(os);
}

/// Custom message deserializer
void sphere::Unpack(std::istream& is)
{
   DEBUG_TRACE_VERBOSE(", sphere::Unpack" << " ");
   is >> center >> radius;
   char type;
   is >> type;

   DEBUG_TRACE_VERBOSE(center << " " << radius << " type: " << std::hex << int(type) << ",");
   matPtr = ConstructMaterial(is, type);
}


/// Custom Message serializer
void xy_rect::Pack(std::ostream& os)
{
   os << x0 << " " << x1 << " " << y0 << " " << y1 << " " << k << " ";
   os << matPtr->GetType() << " ";
   matPtr->Pack(os);
}

/// Custom message deserializer
void xy_rect::Unpack(std::istream& is)
{
   is >> x0 >>  x1 >> y0 >> y1 >> k;
   char type;
   is >> type;
   matPtr = ConstructMaterial(is, type);
}


void xz_rect::Pack(std::ostream& os)
{
   os << x0 << " " << x1 << " " << z0 << " " << z1 << " " << k << " ";
   os << matPtr->GetType() << " ";
   matPtr->Pack(os);
}

/// Custom message deserializer
void xz_rect::Unpack(std::istream& is)
{
   is >> x0 >>  x1 >> z0 >> z1 >> k;
   char type;
   is >> type;
   matPtr = ConstructMaterial(is, type);
}

/// Custom Message serializer
void yz_rect::Pack(std::ostream& os)
{
   os << y0 << " " << y1 << " " << z0 << " " << z1 << " " << k << " ";
   os << matPtr->GetType() << " ";
   matPtr->Pack(os);
}

/// Custom message deserializer
void yz_rect::Unpack(std::istream& is)
{
   is >> y0 >>  y1 >> z0 >> z1 >> k;
   char type;
   is >> type;
   matPtr = ConstructMaterial(is, type);
}

void triangle::Pack(std::ostream& os)
{
   os << a << " " << b << " " << c << " ";
   os << matPtr->GetType() << " ";
   matPtr->Pack(os);
}

/// Custom message deserializer
void triangle::Unpack(std::istream& is)
{
   is >> a >>  b >> c;
   char type;
   is >> type;
   matPtr = ConstructMaterial(is, type);
}

void flipNormals::Pack(std::ostream& os)
{
   os << ptr->GetType() << " ";
   ptr->Pack(os);
}

/// Custom message deserializer
void flipNormals::Unpack(std::istream& is)
{
   char type;
   is >> type;
   ptr = ConstructHitable(is, type);
}

/// Custom Message serializer
void box::Pack(std::ostream& os)
{
   os << pmin << " " << pmax << " ";
   os << listPtr->GetType() << " ";
   listPtr->Pack(os);
}

/// Custom message deserializer
void box::Unpack(std::istream& is)
{
   is >> pmin >> pmax;
   char type;
   is >> type;
   listPtr = ConstructHitable(is, type);
}

/// Custom Message serializer
void rotate_y::Pack(std::ostream& os)
{
   os << ptr->GetType() << " ";
   ptr->Pack(os);
   os << sinTheta << " "  << cosTheta << " " << hasbox << " " << bbox << " ";
}

/// Custom message deserializer
void rotate_y::Unpack(std::istream& is)
{
   char type;
   is >> type;
   ptr = ConstructHitable(is, type);
   is >> sinTheta >> cosTheta >> hasbox >> bbox;
}


/// Custom Message serializer
void translate::Pack(std::ostream& os)
{
   os << offset << " ";
   os << ptr->GetType() << " ";
   ptr->Pack(os);
}

/// Custom message deserializer
void translate::Unpack(std::istream& is)
{
   is >> offset;
   char type;
   is >> type;
   ptr = ConstructHitable(is, type);
}

void hitableList::Pack(std::ostream& os)
{
   os << listSize;
   DEBUG_TRACE_VERBOSE( "hitableList::Pack, listSize: " << listSize << std::endl);
   for (int index = 0; index < listSize; ++index)
   {
      DEBUG_TRACE_VERBOSE(std::endl << "hitableList::Pack, listSize: " << listSize << ", index:" << index);
      os << list[index]->GetType() << " ";
      list[index]->Pack(os);
   }
   DEBUG_TRACE_VERBOSE(std::endl << "hitableList::Pack, done");
}

void hitableList::Unpack(std::istream& is)
{
   is >> listSize;
   DEBUG_TRACE_VERBOSE("hitableList::Unpack, listSize: " << listSize << std::endl);
   list = new hitable *[listSize + 1];
   for (int index = 0; index < listSize; ++index)
   {
      DEBUG_TRACE_VERBOSE(std::endl << "hitableList::Unpack, listSize: " << listSize << ", index:" << index);
      char type;
      is >> type;
      list[index] = ConstructHitable(is, type);
   }
   DEBUG_TRACE_VERBOSE(std::endl << "hitableList::Unpack, done: ");
}

/// Custom Message serializer
void bvh::Pack(std::ostream& os)
{
   os << left->GetType() << " ";
   left->Pack(os);
   os << right->GetType() << " ";
   right->Pack(os);
   os << box << " ";
}

///  Custom message deserializer
void bvh::Unpack(std::istream& is)
{
   char type1, type2;
   is >> type1;
   left = ConstructHitable(is, type1);

   is >> type2;
   right = ConstructHitable(is, type2);
   is >> box;
}


/// Custom Message serializer
void lambertian::Pack(std::ostream& os)
{
   os << albedo->GetType() << " ";
   albedo->Pack(os);
}

/// Custom message deserializer
void lambertian::Unpack(std::istream& is)
{
   char type;
   is >> type;
   albedo = ConstructTexture(is, type);
}

/// Custom Message serializer
void metal::Pack(std::ostream& os)
{
   os << albedo->GetType() << " ";
   albedo->Pack(os);
   os << fuzz << " ";
}

///  Custom message deserializer
void metal::Unpack(std::istream& is)
{
   char type;
   is >> type;
   albedo = ConstructTexture(is, type);
   is >> fuzz;
}

/// Custom Message serializer
void dielectric::Pack(std::ostream& os)
{
   os << albedo->GetType() << " ";
   albedo->Pack(os);
   os << refIdx << " ";
   DEBUG_TRACE_VERBOSE(refIdx << " ");
}

/// Custom message deserializer
void dielectric::Unpack(std::istream& is)
{
   char type;
   is >> type;
   albedo = ConstructTexture(is, type);
   is  >> refIdx;
   DEBUG_TRACE_VERBOSE(refIdx << " ");
}

/// Custom Message serializer
void diffuseLight::Pack(std::ostream& os)
{
   os << emit->GetType() << " ";
   emit->Pack(os);
}

///  Custom message deserializer
void diffuseLight::Unpack(std::istream& is)
{
   char type;
   is >> type;
   emit = ConstructTexture(is, type);
}


/// Custom Message serializer
void constantTexture::Pack(std::ostream& os)
{
   os << color << " ";
   DEBUG_TRACE_VERBOSE(color << " ");
}

/// Custom message deserializer
void constantTexture::Unpack(std::istream& is)
{
   is >> color;
   DEBUG_TRACE_VERBOSE(color << " ");
}


/// Custom Message serializer
void checkerTexture::Pack(std::ostream& os)
{
   os << odd->GetType() << " ";
   odd->Pack(os);

   os << even->GetType() << " ";
   even->Pack(os);
}

/// Custom message deserializer
void checkerTexture::Unpack(std::istream& is)
{
   char oddType, evenType;

   is >> oddType;
   odd = ConstructTexture(is, oddType);

   is >> evenType;
   even = ConstructTexture(is, evenType);
}


/// Custom Message serializer
void checkerNoise::Pack(std::ostream& os)
{
   os << color << " "; 
   os << scale  << " ";
}

/// Custom message deserializer
void checkerNoise::Unpack(std::istream& is)
{
   is >> color >> scale;
}


/// Custom Message serializer
void camo::Pack(std::ostream& os)
{
   os << color << " ";
   os << scale  << " ";
}

/// Custom message deserializer
void camo::Unpack(std::istream& is)
{
   is >> color >> scale;
}

/// Custom Message serializer
void marble::Pack(std::ostream& os)
{
   os << color << " ";
   os << scale  << " ";
}

/// Custom message deserializer
void marble::Unpack(std::istream& is)
{
   is >> color >> scale;
}

