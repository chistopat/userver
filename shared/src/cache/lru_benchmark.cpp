#include <benchmark/benchmark.h>

#include <userver/cache/lru_map.hpp>
#include <userver/cache/lfu_map.hpp>

USERVER_NAMESPACE_BEGIN

namespace {

using Lru = cache::LruMap<unsigned, unsigned>;
using Lfu = LfuBase<unsigned, unsigned>;
using WTinyLFU = cache::impl::LruBase<int, int, std::hash<int>,
                                      std::equal_to<int>, cache::CachePolicy::kWTinyLFU>;

constexpr unsigned kElementsCount = 1000;

template <typename CachePolicyContainer>
CachePolicyContainer FillLru(unsigned elements_count) {
  CachePolicyContainer lru(kElementsCount);
  for (unsigned i = 0; i < elements_count; ++i) {
    lru.Put(i, i);
  }

  return lru;
}

template<>
WTinyLFU FillLru<WTinyLFU>(unsigned elements_count){
  WTinyLFU cache(elements_count, std::hash<int>{}, std::equal_to<int>{}, 0.03);
  for (unsigned i = 0; i < elements_count; ++i) {
    cache.Put(i, i);
  }
  return cache;
}

}  // namespace

template <typename CachePolicyContainer>
void Put(benchmark::State& state) {
  for (auto _ : state) {
    auto lru = FillLru<CachePolicyContainer>(kElementsCount);
    benchmark::DoNotOptimize(lru);
  }
}
BENCHMARK(Put<Lru>);
BENCHMARK(Put<Lfu>);
//BENCHMARK(Put<WTinyLFU>);

template <typename CachePolicyContainer>
void Has(benchmark::State& state) {
  auto lru = FillLru<CachePolicyContainer>(kElementsCount);
  for (auto _ : state) {
    for (unsigned i = 0; i < kElementsCount; ++i) {
      benchmark::DoNotOptimize(lru.Get(i));
    }
  }
}
BENCHMARK(Has<Lru>);
BENCHMARK(Has<Lfu>);
//BENCHMARK(Has<WTinyLFU>);

template <typename CachePolicyContainer>
void PutOverflow(benchmark::State& state) {
  auto lru = FillLru<CachePolicyContainer>(kElementsCount);
  unsigned i = kElementsCount;
  for (auto _ : state) {
    for (unsigned j = 0; j < kElementsCount; ++j) {
      ++i;
      lru.Put(i, i);
    }
    benchmark::DoNotOptimize(lru);
  }
}
BENCHMARK(PutOverflow<Lru>);
BENCHMARK(PutOverflow<Lfu>);
//BENCHMARK(PutOverflow<WTinyLFU>);

USERVER_NAMESPACE_END
