// Scenarios_b.h
// Contains all fictional starting data in one short, editable file.
// Values are teaching examples, not real facility or contract data.
#ifndef GRIDMIND_NODE_B_SCENARIOS_H
#define GRIDMIND_NODE_B_SCENARIOS_H

#include "Game_b.h"

// Placeholder used until the first valid Node A response arrives.
static const Facility INITIAL_FACILITY = {true, true, 22, 28};

// Job order defines the queue order at startup.
static const Job JOBS[] = {
    {"Northstar Research - AI training", 5, 3000000, 800000, true},
    {"BluePeak Media - Video rendering", 2, 1800000, 400000, true},
    {"Helios Weather - Regional forecast", 3, 2500000, 1000000, true},
    {"Atlas Storage - Data backup", 1, 900000, 150000, true},
    {"MedCore Health - Medical analytics", 2, 3200000, 1500000, false},
    {"Civic Systems - Batch reporting", 1, 600000, 100000, true},
    {"Sentinel Security - Threat analysis", 4, 2800000, 1200000, false},
    {"ArchiveWorks - Data compression", 2, 1000000, 200000, true},
    {"Aurora Labs - AI training", 6, 3600000, 1100000, true},
    {"FrameForge Studio - Video rendering", 3, 2000000, 500000, true}};

static const uint8_t JOB_COUNT = sizeof(JOBS) / sizeof(JOBS[0]);

#endif
