/*
 * GridMind Node B - Application Coordinator Interface
 *
 * Composes the game model with button, LED, Wi-Fi, and web adapters. The class
 * coordinates their lifecycle while keeping domain rules inside NodeBGame.
 */

#ifndef GRIDMIND_NODE_B_APPLICATION_H
#define GRIDMIND_NODE_B_APPLICATION_H

#include "ButtonPanel.h"
#include "FeedbackLed.h"
#include "NodeBGame.h"
#include "NodeBWebApi.h"
#include "WiFiConnection.h"

class NodeBApplication {
 public:
  NodeBApplication(
      const char* wifiSsid,
      const char* wifiPassword,
      bool runStartupTests);

  void begin();
  void update();

 private:
  gridmind::ButtonPanel buttonPanel_;
  // Single authoritative model shared by physical and web adapters.
  gridmind::NodeBGame game_;
  gridmind::FeedbackLed feedbackLed_;
  WiFiConnection wifiConnection_;
  NodeBWebApi webApi_;
  bool runStartupTests_;

  void beginInteractiveScenario();
  void beginWiFi();
  void printDecision(const gridmind::DecisionResult& result) const;
};

#endif
