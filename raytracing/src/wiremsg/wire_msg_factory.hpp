#pragma once
#include "wire_msg.hpp"

class WireMsgFactory
{
public:
    static WireMsgPtr ConstructMsg(uint8_t *buffer, int dataLength);
};
