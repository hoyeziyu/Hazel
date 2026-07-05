#include <benchmark/benchmark.h>
#include <algorithm>
#include <cstdint>
#include <forward_list>
#include <vector>

// The number of items we will insert
const int ITEM_COUNT = 10000;

static void BM_VectorInsertFront(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<int> v;

    for (int i = 0; i < ITEM_COUNT; ++i) {
      // O(n) insertion - shifting elements
      v.insert(v.begin(), i);
    }

    // Prevent DCE
    benchmark::DoNotOptimize(v.data());
  }
}
BENCHMARK(BM_VectorInsertFront);

static void BM_ListInsertFront(benchmark::State& state) {
  for (auto _ : state) {
    std::forward_list<int> l;

    for (int i = 0; i < ITEM_COUNT; ++i) {
      // O(1) insertion - pointer update
      l.push_front(i);
    }

    // Prevent DCE - forward_list doesn't have .data()
    // so we use the front element
    benchmark::DoNotOptimize(l.front());
  }
}
BENCHMARK(BM_ListInsertFront);

// forward_list 单向链表：遍历需沿 next 指针跳转，cache 不友好，理论 O(n)
static void BM_ListTraverse(benchmark::State& state) {
  int64_t n = state.range(0);
  state.SetComplexityN(n);

  // 在计时循环外构造链表，只测遍历本身
  std::forward_list<int> l(static_cast<size_t>(n));
  std::fill(l.begin(), l.end(), 1);

  for (auto _ : state) {
    long long sum = 0;
    for (int i : l) {
      sum += i;
    }
    benchmark::DoNotOptimize(sum);
  }
}

// Range: 链表长度从 8 倍增到 8<<24；Complexity() 让 Google Benchmark 自动拟合 Big-O
// 注意：n 很大时耗时长、占内存多，可先 --benchmark_filter=BM_ListTraverse 或缩小 Range
BENCHMARK(BM_ListTraverse)
  ->Range(8, 8 << 24)
  ->Complexity();
