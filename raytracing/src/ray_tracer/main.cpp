#include <iostream>
#include "surfaces.h"
#include "hitablelist.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "scene.h"

using namespace std;
vec3 color(const ray& r, hitable* world, int depth, vec3& curAlbedo);
vec3 randomInUnitSphere();

#if 0
int main()
{

}
#endif

int ray_tracer_produce_scene()
{
  int nx, ny, ns;
  camera cam;
  vec3 curAlbedo;

  hitable* world = myScene(cam, nx, ny, ns);
  cout << "P3\n" << nx << " " << ny << "\n255\n";
  for (int j = ny-1; j >= 0; j--)
  {
    for (int i = 0; i < nx; i++)
    {
      vec3 col(0,0,0);
      #pragma omp parallel for schedule(static, 10)
      for (int s = 0; s < ns; s++)
      {
        float u = float(i + drand48()) / float(nx);
        float v = float(j + drand48()) / float(ny);
        ray r = cam.getRay(u, v);
        vec3 curAlbedo(1.0, 1.0, 1.0);
        col += color(r, world,0, curAlbedo);
      }
      col /= float(ns);
      col = vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));
      if(col[0] > 1 || col[2] > 1 || col[3] >1)
       cout << "there is a problem here: " << i << endl;
      int ir = int(255.99*col[0]);
      int ig = int(255.99*col[1]);
      int ib = int(255.99*col[2]);
      cout << ir <<" " << ig << " " << ib << "\n";
    }
  }
  return 0;
}

vec3 color(const ray& r, hitable* world, int depth, vec3& curAlbedo)
{
  hitRecord rec;

  if (world -> hit(r, 0.001, FLT_MAX, rec))
  {
    ray scattered;
    vec3 attenuation;
    vec3 emitted = rec.matPtr -> emitted(0, 0, rec.p);
    if (depth < 50 && rec.matPtr->scatter(r, rec, attenuation, scattered))
    {
      curAlbedo *= attenuation;
      float p = curAlbedo.max();
      if (drand48() > p)
      {
        return vec3(0, 0, 0);
      }
      curAlbedo *= 1/p;
      return emitted + color(scattered, world, depth+1, curAlbedo);
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
    float t = 0.5*(unitDirection.y() + 1.0);
    return curAlbedo*((1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0));
  }
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
