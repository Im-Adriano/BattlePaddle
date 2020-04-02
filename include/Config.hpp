/*
 * Config
 *
 * Configurations for the bot.
 * Compiled in to reduce amount of files on the target system.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

extern uint32_t c2Ip;
extern uint32_t gatewayIp;
extern bool useGateway;
extern int requestActionInterval;
extern uint16_t srcPort;
extern uint16_t dstPort;

#endif

