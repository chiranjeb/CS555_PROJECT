#pragma once
#include "message.hpp"
#include <memory>
class Listener
{
public:
   virtual void Notify(std::shared_ptr<Msg> msg) = 0;
};
