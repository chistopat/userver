#pragma once

USERVER_NAMESPACE_BEGIN

namespace cache {
enum CachePolicy { kLRU = 0, kSLRU, kLFU, kTinyLFU, kWTinyLFU };
enum FrequencySketchPolicy { Trivial = 0, Bloom, DoorkeeperBloom, CaffeineBloom };
}  // namespace cache

USERVER_NAMESPACE_END
