#pragma once
#include "wire_msg.hpp"

class WireMsgFactory
{
public:
    static WireMsgPtr ConstructMsg(char *buffer, int dataLength);
};
