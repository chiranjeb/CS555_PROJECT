#pragma once
#include "wire_msg.hpp"
#include "ray_tracer/scene_descriptor.hpp"

class SceneDescriptionMsg : public WireMsg
{
   public:
   /** 
   * SceneDescriptionMsg message constructor
   *  
   */
   SceneDescriptionMsg(SceneDescriptorPtr sceneDescriptorPtr): 
      WireMsg(MsgIdSceneDistributionMsg), m_SceneDescriptorPtr(sceneDescriptorPtr)
   {
       DEBUG_TRACE("SceneDescriptionMsg: Constructor");
   }

   /// Scene distribution message.
   SceneDescriptionMsg(): WireMsg(MsgIdSceneDistributionMsg)
   {
   }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom Message serializer
   /// @param [ostrm] Output stream where the message is being
   ///            serialized to
   /// 
   ///////////////////////////////////////////////////////////////////////////
   void Pack(std::ostream &ostrm)
   {
      DEBUG_TRACE("SceneDescriptionMsg:Pack");
      WireMsg::Pack(ostrm);
      m_SceneDescriptorPtr.get()->Pack(ostrm);
   }

   ///////////////////////////////////////////////////////////////////////////
   ///
   /// Custom message deserializer
   /// @param [istrm] Input stream from which the message is being
   ///           deserialized.
   /// 
   ///////////////////////////////////////////////////////////////////////////
   void Unpack(std::istream &istrm)
   {
      WireMsg::Unpack(istrm);
      m_SceneDescriptorPtr = std::make_shared<SceneDescriptor>();
      m_SceneDescriptorPtr.get()->Unpack(istrm);
   }

   ~SceneDescriptionMsg()
   {
       DEBUG_TRACE("SceneDescriptionMsg: Destructor");
   }

   // @todo: add chunk size.
   SceneDescriptorPtr  m_SceneDescriptorPtr;
};

typedef std::shared_ptr<SceneDescriptionMsg> SceneDescriptionMsgPtr;
