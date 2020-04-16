#include "Config.hpp"

uint8_t c2IpEdit[4] = { 192, 168, 214, 148};
uint8_t gatewayipEdit[4] = {192, 168, 235, 1};
bool useGateway = false;
int requestActionInterval = 120000;
uint16_t srcPort = 53;
uint16_t dstPort = 53;


uint32_t c2Ip = *reinterpret_cast<uint32_t *>(c2IpEdit);
uint32_t gatewayIp = *reinterpret_cast<uint32_t *>(gatewayipEdit);


