#pragma once

#include <chrono>
#include <cstdint>

static const char LOG_BASE_NAME[] = "haha";
static const uint32_t LOG_ROLL_SIZE = 10U * 1024 * 1024;
static const uint32_t LOG_BUF_CAP = 4U * 1024 * 1024;
static const uint32_t LOG_BUF_TAIL_CAP = 4000;
static const uint32_t LOG_STREAM_BUF_CAP = 4000;
static const auto LOG_REST_INTERVAL = std::chrono::microseconds(1);

static_assert(LOG_BUF_TAIL_CAP >= LOG_STREAM_BUF_CAP);
