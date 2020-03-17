#include "Config.hpp"

uint8_t c2IpEdit[4] = {192, 168, 176, 1};
uint8_t gatewayipEdit[4] = {192, 168, 235, 1};
bool useGateway = false;
int requestActionInterval = 5000;


uint32_t c2Ip = *reinterpret_cast<uint32_t *>(c2IpEdit);
uint32_t gatewayIp = *reinterpret_cast<uint32_t *>(gatewayipEdit);


