#include "surfaces.h"
#include "hitablelist.h"
#include "float.h"
#include "camera.h"
#include "material.h"
#include "transformations.h"

hitable *cornellBox()
{
  hitable **list = new hitable*[20];
  hitable **lights = new hitable*[20];
  hitable **nonLights = new hitable*[20];
  char file[] = "lowpolycat/cat.obj";
  vec3 trans = vec3(0.0, 0.0, 0.0);
  int i = 0, j = 0, k = 0;
  material *red = new lambertian(new constantTexture(vec3(0.65, 0.05, 0.05)));
  material *white = new lambertian(new constantTexture(vec3(0.73, 0.73, 0.73)));
  material *whiteMetal = new metal(new constantTexture(vec3(0.73, 0.73, 0.73)), 0);
  material *green = new lambertian(new constantTexture(vec3(0.12, 0.45, 0.15)));
  material *light = new diffuseLight(new constantTexture(vec3(15, 15, 15)));
  material *cam = new lambertian(new camo(vec3(7,7, 7), 1));
  material *die = new dielectric(new constantTexture(vec3(1.0, 1.0, 1.0)), 1.3);

//  trianglemesh *a = new trianglemesh(file, white, 1.0, trans);

  list[i++] = new flipNormals(new yz_rect(0, 555, -1000, 555, 555, green));
  list[i++] = new yz_rect(0, 555, -1000, 555, 0, red);
  list[i++] = new flipNormals(new xz_rect(213, 343, 227, 332, 554, light));
  list[i++] = new xz_rect(0, 555, -1000, 555, 0, cam);
  list[i++] = new flipNormals(new xz_rect(0, 555, -1000, 555, 555, cam));
  list[i++] = new flipNormals(new xy_rect(0, 555, 0, 555, 555, cam));
  list[i++] = new xy_rect(0, 555, 0, 555, -1000, cam);
  //list[i++] = new triangle(vec3(400, 555, 555), vec3(555, 400,555), vec3(555, 555, 400), white);
  list[i++] = new sphere(vec3(182.5, 240, 147.5), 75, new dielectric(new constantTexture(vec3(1.0, 1.0, 1.0)), 1.3));
  list[i++] = new sphere(vec3(367.5, 405, 407.5), 75, cam);
  list[i++] = new sphere(vec3(460, 75, 75), 75, new lambertian(new marble(vec3(0.4, 0.1, 0.7),10)));
  list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 165, 165), white), -18), vec3(130, 0, 65));
  list[i++] = new translate(new rotate_y(new box(vec3(0, 0, 0), vec3(165, 330, 165), whiteMetal), 15), vec3(265, 0, 295));
  return new bvh(list, i, 0, 1);
}

hitable *random_scene() {
    int n = 500;
    hitable **list = new hitable*[n+1];
    list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(new constantTexture(vec3(0.5, 0.5, 0.5))));
    int i = 1;
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = drand48();
            vec3 center(a+0.9*drand48(),0.2,b+0.9*drand48());
            if ((center-vec3(4,0.2,0)).length() > 0.9) {
                if (choose_mat < 0.8) {  // diffuse
                    list[i++] = new sphere(
                        center, 0.2,
                        new lambertian(new constantTexture(vec3(drand48()*drand48(),
                                            drand48()*drand48(),
                                            drand48()*drand48())))
                    );
                }
                else if (choose_mat < 0.95) { // metal
                    list[i++] = new sphere(
                        center, 0.2,
                        new metal(new constantTexture(vec3(0.5*(1 + drand48()),
                                       0.5*(1 + drand48()),
                                       0.5*(1 + drand48()))),
                                  0.5*drand48())
                    );
                }
                else {  // glass
                    list[i++] = new sphere(center, 0.2, new dielectric(new constantTexture(vec3(1.0, 1.0,1.0)), 1.5));
                }
            }
        }
    }

    list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(new constantTexture(vec3(1.0, 1.0,1.0)), 1.5));
    list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(new constantTexture(vec3(0.4, 0.2, 0.1))));
    list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(new constantTexture(vec3(0.7, 0.6, 0.5)), 0.0));

    return new hitableList(list,i);
 //   int n = 500;
 //   material *cam = new lambertian(new camo(vec3(1.0,1.0, 1.0), 2));
 //   material *light = new diffuseLight(new constantTexture(vec3(1, 1, 1.6)));
 //   hitable **list = new hitable*[n+1];
 //   list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(new constantTexture(vec3(0.5, 0.5, 0.5))));
 //   int i = 1;
 //   for (int a = -11; a < 11; a++) {
 //       for (int b = -11; b < 11; b++) {
 //           float choose_mat = drand48();
 //           vec3 center(a+0.9*drand48(),0.2,b+0.9*drand48());
 //           if ((center-vec3(4,0.2,0)).length() > 0.9) {
 //               if (choose_mat < 0.8) {  // diffuse
 //                   list[i++] = new sphere(center, 0.2, new lambertian( new camo(vec3(drand48(), drand48(), drand48()), drand48()*5)));//new constantTexture(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48()))));
 //               }
 //               else if (choose_mat < 0.95) { // metal
 //                   list[i++] = new sphere(center, 0.2,
 //                           new metal(new constantTexture(vec3(0.5*(1 + drand48()), 0.5*(1 + drand48()), 0.5*(1 + drand48()))),  0.5*drand48()));
 //               }
 //               else {  // glass
 //                   list[i++] = new sphere(center, 0.2, new dielectric(new constantTexture(vec3(1.0, 1.0, 1.0)), 1.5));
 //               }
 //           }
 //       }
 //   }

 //   //list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(new constantTexture(vec3(1.0, 1.0, 1.0)), 1.5));
 //   list[i++] = new sphere(vec3(-4, 1, 0), 1.0, cam);//new constantTexture(vec3(0.4, 0.2, 0.1))));
 //   list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(new constantTexture(vec3(0.7, 0.6, 0.5)), 0.0));
 //   list[i++] = new sphere(vec3(0, 1,0), 1.0, light); 
 // //  list[i++] = new sphere(vec3(0, 10,0), 2.0, light); 
 //   return new bvh(list, i, 0, 1);
 //   //return new hitableList(list,i);
}

hitable* myScene(camera& cam, int& nx, int& ny, int& ns)
{
  nx = 512;
  ny = 512;


  //nx = 64;
  //ny = 64;

  ns = 20; 
  int n = 0;

  //hitable **list = new hitable*[4];
  //list[n++] = new sphere(vec3(0,0,-1), 2, new lambertian(new checkerNoise(vec3(1.0, 1.0, 1.0), 100)));
  //list[n++] = new sphere(vec3(0, -1002, -1), 1000, new lambertian(new checkerTexture(new constantTexture(vec3(0.2, 0.3, 0.1)), new constantTexture(vec3(0.9, 0.9, 0.9)))));
  //list[n++] = new sphere(vec3(4,0,-1), 2, new lambertian(new camo(vec3(1.0, 1.0, 1.0), 1)));
  //list[n++] = new sphere(vec3(-4,0,-1), 2, new lambertian(new marble(vec3(1.0, 1.0, 1.0), 10)));
  //list[4] = new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));
  //list[0] = new sphere(vec3(-R,0,-1), R, new lambertian(vec3(0, 0, 1)));
  //list[1] = new sphere(vec3(R,0,-1), R, new lambertian(vec3(1, 0, 0)));
  //for cornell box
  //vec3 lookfrom(278, 278, -800);
  //vec3 lookat(278, 278, 0);
  float distToFocus = 10; //(lookfrom - lookat).length();
  float aperture = 0.1;
  
  vec3 lookfrom(13, 2,3);
  vec3 lookat(0, 0, 0);
  //float distToFocus= (lookfrom - lookat).length();
  //float aperture = 0;
  cam = camera(lookfrom, lookat, vec3(0,1, 0), 20, float(nx)/float(ny), aperture, distToFocus);
  return random_scene();

  //if (n > 1)
  //  return new bvh(list, n, 0, 1);
  //return new hitableList(list, 4);
}
