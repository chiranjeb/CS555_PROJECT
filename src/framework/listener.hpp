#pragma once
#include "message.hpp"

class Listener
{
public:
   virtual void Notify(std::shared_ptr<Msg> msg) = 0;
};
