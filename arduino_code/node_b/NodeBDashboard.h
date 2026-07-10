/*
 * GridMind Node B - Dashboard Asset Interface
 *
 * Declares the immutable browser dashboard stored in flash.
 * HTTP routing and JSON serialization remain in NodeBWebApi.
 */

#ifndef GRIDMIND_NODE_B_DASHBOARD_H
#define GRIDMIND_NODE_B_DASHBOARD_H

#include <Arduino.h>

namespace gridmind {

extern const char NODE_B_DASHBOARD_HTML[] PROGMEM;

}  // namespace gridmind

#endif
