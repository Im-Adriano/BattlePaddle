/*
 * Config
 *
 * Configurations for the bot.
 * Compiled in to reduce amount of files on the target system.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <string>

extern uint32_t C2IP;
extern uint32_t GatewayIP;
extern bool useGateway;
extern std::string C2IPSTR;
extern int requestActionInterval;

#endif

