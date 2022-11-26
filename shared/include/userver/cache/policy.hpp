#pragma once

USERVER_NAMESPACE_BEGIN

namespace cache {
enum class CachePolicy { kLRU = 0, kSLRU, kLFU, kTinyLFU, kWTinyLFU, kARC };
enum class FrequencySketchPolicy { Trivial = 0, Bloom, DoorkeeperBloom };
}  // namespace cache

USERVER_NAMESPACE_END
