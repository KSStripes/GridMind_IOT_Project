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
