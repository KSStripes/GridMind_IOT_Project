// Game.h
// Defines the small queue model shared by the sketch, dashboard and tests.
// Game contains no Arduino pin or Wi-Fi code, so its rules are easy to test.
#ifndef GRIDMIND_GAME_H
#define GRIDMIND_GAME_H

#include <stdint.h>

enum class Action : uint8_t {
  RUN,
  WAIT,
  CANCEL
};

enum class Reason : uint8_t {
  COMPLETED,
  QUEUED,
  CANCELLED,
  NO_CAPACITY,
  NO_POWER,
  TOO_HOT,
  ALREADY_WAITED,
  QUEUE_EMPTY,
  INVALID_STATE
};

struct Facility {
  bool capacityAvailable;
  bool powerAvailable;
  int16_t tempC;
  int16_t tempLimitC;
};

struct Job {
  const char* name;
  int16_t tempRiseC;
  int32_t valueCents;
  int32_t penaltyCents;
  bool canWait;
};

// A Result describes the last attempted action, including the job acted on.
struct Result {
  Action action;
  const char* jobName;
  bool accepted;
  Reason reason;
  int32_t deltaCents;
  int32_t totalCents;
};

class Game {
 public:
  Game();

  bool begin(const Facility& facility, const Job jobs[], uint8_t jobCount);
  bool setFacility(const Facility& facility);
  Result apply(Action action);

  bool isReady() const;
  const Facility& facility() const;
  const Job* currentJob() const;
  uint8_t queueSize() const;
  int32_t totalCents() const;
  bool hasResult() const;
  const Result& lastResult() const;

 private:
  // A fixed array avoids dynamic memory on the ESP8266.
  static const uint8_t MAX_JOBS = 5;

  Facility facility_;
  Job jobs_[MAX_JOBS];
  uint8_t jobCount_;
  int32_t totalCents_;
  bool ready_;
  bool hasResult_;
  Result lastResult_;

  static bool validFacility(const Facility& facility);
  static bool validJob(const Job& job);
  Result finish(Action action, const char* jobName, bool accepted,
                Reason reason, int32_t deltaCents);
  void removeFront();
};

const char* actionName(Action action);
const char* reasonName(Reason reason);

#endif
