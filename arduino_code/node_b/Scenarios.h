// Scenarios.h
// Contains all fictional starting data in one short, editable file.
// Values are teaching examples, not real facility or contract data.
#ifndef GRIDMIND_SCENARIOS_H
#define GRIDMIND_SCENARIOS_H

#include "Game.h"

// An accepted Wait advances to the next simple facility condition.
static const Facility FACILITIES[] = {
    {true, true, 26, 28},
    {false, true, 24, 28},
    {true, false, 23, 28},
    {true, true, 22, 28}};

static const uint8_t FACILITY_COUNT =
    sizeof(FACILITIES) / sizeof(FACILITIES[0]);

// Job order defines the queue order at startup.
static const Job JOBS[] = {
    {"Northstar Research - AI training", 5, 3000000, 800000, true},
    {"BluePeak Media - Video rendering", 2, 1800000, 400000, true},
    {"Helios Weather - Regional forecast", 3, 2500000, 1000000, true}};

static const uint8_t JOB_COUNT = sizeof(JOBS) / sizeof(JOBS[0]);

#endif
