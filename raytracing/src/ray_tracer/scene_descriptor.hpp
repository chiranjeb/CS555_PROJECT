#pragma once
#include "camera.h"
#include "hitable.h"
#include <memory>


typedef std::shared_ptr<camera> CameraPtr;
typedef std::shared_ptr<hitable> HitablePtr;

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
      os << (*m_Camera.get()) << " ";
      //os << (*m_HitableList.get()) << " ";
   }

   void UnPack(std::istream& is)
   {
      int hitableSize;
      is >> m_nX >> m_nY >> (*m_Camera.get());
      //m_HitableList = std::make_shared<hitableList>();
      //istrm >> (*m_HitableList.get());
   }

   friend class SceneFactory;

protected:
   int m_nX;                            // Image width
   int m_nY;                            // Image height
   CameraPtr m_Camera;                  // Camera
   HitablePtr m_HitableList;            // hitableList pointer
};

typedef std::shared_ptr<SceneDescriptor> SceneDescriptorPtr;



class SceneFactory
{
public:
   static SceneDescriptorPtr GenerateRandomScene()
   {
      /// Currently we have only one scene. When we have lot of scenes, this function
      /// can chose scenes randomly.

      /// For now, let's create the scene descriptor
      SceneDescriptorPtr sceneDescriptorPtr = std::make_shared<SceneDescriptor>();
      SceneDescriptor *pDescriptor = sceneDescriptorPtr.get();
      camera& cam = *pDescriptor->m_Camera.get();

      int nS;

      /// Lets create get our scene.
      extern hitable* myScene(camera& cam, int& nx, int& ny, int& ns);
      hitable *world = myScene(cam, pDescriptor->m_nX, pDescriptor->m_nY, nS);

      /// Now save the hitable list
      pDescriptor->m_HitableList = HitablePtr(world);

      /// Finally return the scene descriptor pointer.
      return sceneDescriptorPtr;
   }
};


