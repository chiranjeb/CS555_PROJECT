#pragma once
#include "camera.h"
#include "hitable.h"
#include <memory>


typedef std::shared_ptr<camera> CameraPtr;
typedef std::shared_ptr<hitable> HitablePtr;

hitable* ConstructHitable(std::istream& is, char type);

class SceneFactory;
class SceneDescriptor
{
public:
   SceneDescriptor()
   {
      m_Camera = std::make_shared<camera>();
   }

   void Pack(std::ostream& os)
   {
      os << m_nX << " ";
       os << m_nY << " ";
       os << m_RPP << " ";
      DEBUG_TRACE_VERBOSE("m_nx:" << m_nX << "m_nY:" << m_nY << "m_RPP:" << m_RPP << std::endl);

      os << (*m_Camera.get()) << " "; 
      DEBUG_TRACE_VERBOSE("Camera:" << (*m_Camera.get()) << std::endl);

      os << m_HitableList->GetType() << " ";
      DEBUG_TRACE_VERBOSE("Type:" << std::hex << m_HitableList->GetType() << " " << std::endl);

      m_HitableList->Pack(os);
   }

   void Unpack(std::istream& is)
   {
      int hitableSize;
      is >> m_nX >> m_nY >> m_RPP ;
      DEBUG_TRACE_VERBOSE("m_nx:" << m_nX << "m_nY:" << m_nY << "m_RPP:" << m_RPP << std::endl);

      is >> (*m_Camera.get());
      DEBUG_TRACE_VERBOSE("Camera:" << (*m_Camera.get()));

      char type;
      is >> type;
      DEBUG_TRACE_VERBOSE("Type:" << std::hex << type << " " << std::endl);

      hitable *ptr = ConstructHitable(is, type);
      m_HitableList= HitablePtr(ptr);
   }

   int GetNX() { return m_nX; }
    int GetNY() { return m_nY; }
    int GetRPP() { return m_RPP; }

   CameraPtr GetCameraPtr() { return m_Camera; }
   HitablePtr GetWorld() { return m_HitableList;}

   friend class SceneFactory;

protected:
   int m_nX;                            // Image width
   int m_nY;                            // Image height
   int m_RPP;                           // Rays Per Pixel
   CameraPtr m_Camera;                  // Camera
   HitablePtr m_HitableList;            // hitableList pointer
};

typedef std::shared_ptr<SceneDescriptor> SceneDescriptorPtr;

void ProducePixels(uint32_t NY_end, uint32_t NY_start, uint32_t NX_end, uint32_t NX_start,
                   uint32_t nx, uint32_t ny, uint32_t ns, 
                   camera *p_camera, hitable* world, std::ostream &os);

inline void ProducePixels(uint32_t NY_end, uint32_t NY_start, uint32_t NX_end, uint32_t NX_start, SceneDescriptorPtr sceneDescriptorPtr, std::ostream &os)
{
   ProducePixels(NY_end, NY_start, NX_end, NX_start, sceneDescriptorPtr->GetNX(), sceneDescriptorPtr->GetNY(), sceneDescriptorPtr->GetRPP(),
                 sceneDescriptorPtr->GetCameraPtr().get(), sceneDescriptorPtr->GetWorld().get(), os);
}

class SceneFactory
{
public:
   static SceneDescriptorPtr GetScene(std::string scene_name, int nx, int ny, int ns)
   {
      /// Currently we have only one scene. When we have lot of scenes, this function
      /// can chose scenes randomly.

      /// For now, let's create the scene descriptor
      SceneDescriptorPtr sceneDescriptorPtr = std::make_shared<SceneDescriptor>();
      SceneDescriptor *pDescriptor = sceneDescriptorPtr.get();
      camera& cam = *pDescriptor->m_Camera.get();

      /// Lets create get our scene.
      extern hitable* myScene(camera& cam, int& nx, int& ny, std::string scene_name);
      hitable *world = myScene(cam, pDescriptor->m_nX, pDescriptor->m_nY, scene_name);

      /// save the rays per pixel
      pDescriptor->m_RPP = ns;

      /// Now save the hitable list
      pDescriptor->m_HitableList = HitablePtr(world);

      /// Finally return the scene descriptor pointer.
      return sceneDescriptorPtr;
   }
};


