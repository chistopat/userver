#include <gtest/gtest.h>

#include <algorithm>

#include <userver/cache/impl/arc.hpp>

USERVER_NAMESPACE_BEGIN

using Equal = std::equal_to<int>;
using Hash = std::hash<int>;
using ARC = cache::impl::LruBase<int, int, Hash, Equal,
                                  cache::CachePolicy::kARC>;

std::vector<int> GetT1(ARC& arc) {
  std::vector<int> actual;
  arc.VisitT1([&actual] (int key, int) {
    actual.push_back(key);
  });
  std::sort(actual.begin(), actual.end());
  return actual;
}

std::vector<int> GetT2(ARC& arc) {
  std::vector<int> actual;
  arc.VisitT2([&actual] (int key, int) {
    actual.push_back(key);
  });
  std::sort(actual.begin(), actual.end());
  return actual;
}

std::vector<int> GetB1(ARC& arc) {
  std::vector<int> actual;
  arc.VisitB1([&actual] (int key, int) {
    actual.push_back(key);
  });
  std::sort(actual.begin(), actual.end());
  return actual;
}

std::vector<int> GetB2(ARC& arc) {
  std::vector<int> actual;
  arc.VisitB2([&actual] (int key, int) {
    actual.push_back(key);
  });
  std::sort(actual.begin(), actual.end());
  return actual;
}

TEST(ARC, Put) {
  ARC arc(12, std::hash<int>{}, std::equal_to<int>{});

  for (int i = 0; i < 3; i++) EXPECT_TRUE(arc.Put(i, 0));

  EXPECT_EQ(GetT1(arc), (std::vector<int>{0, 1, 2}));
  EXPECT_EQ(arc.GetSize(), 3);

  for (int i = 0; i < 3; i++) EXPECT_TRUE(arc.Put(i, 0));

  EXPECT_EQ(GetT2(arc), (std::vector<int>{0, 1, 2}));
  EXPECT_EQ(arc.GetSize(), 3);
}


  // EXPECT_TRUE(arc.Put(8, 0));
  // EXPECT_EQ(GetT1(arc), (std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8}));
  // EXPECT_EQ(arc.GetSize(), 8);

  // EXPECT_TRUE(arc.Put(6, 1));
  // EXPECT_EQ(GetT1(arc), (std::vector<int> {1, 2, 3, 4, 5, 7, 8}));
  // EXPECT_EQ(GetT2(arc), (std::vector<int> {6}));

  // EXPECT_FALSE(arc.Put(7, 1));
  // EXPECT_EQ(GetT1(arc), (std::vector<int> {1, 2, 3, 4, 5, 8}));
  // EXPECT_EQ(GetProtected(arc), (std::vector<int> {6, 7}));

  // EXPECT_FALSE(arc.Put(7, 2));
  // EXPECT_EQ(GetT1(arc), (std::vector<int> {1, 2, 3, 4, 5, 8}));
  // EXPECT_EQ(GetProtected(arc), (std::vector<int> {6, 7}));

  // EXPECT_TRUE(arc.Put(9, 0));
  // EXPECT_TRUE(arc.Put(10, 0));
  // EXPECT_TRUE(arc.Put(11, 0));
  // EXPECT_EQ(GetT1(arc), (std::vector<int> {2, 3, 4, 5, 8, 9, 10, 11}));
  // EXPECT_EQ(GetProtected(arc), (std::vector<int> {6, 7}));

  // arc.Erase(9);
  // arc.Erase(10);
  // EXPECT_FALSE(arc.Put(8, 1));
  // EXPECT_EQ(GetT1(arc), (std::vector<int> {2, 3, 4, 5, 6, 11}));
  // EXPECT_EQ(GetProtected(arc), (std::vector<int> {7, 8}));

  // EXPECT_FALSE(arc.Put(5, 1));
  // EXPECT_EQ(GetT1(arc), (std::vector<int> {2, 3, 4, 6, 7, 11}));
  // EXPECT_EQ(GetProtected(arc), (std::vector<int> {5, 8}));

  // for (int i = 12; i < 20; i++)
  //   EXPECT_TRUE(arc.Put(i, 0));
  // EXPECT_EQ(GetT1(arc), (std::vector<int> {12, 13, 14, 15, 16, 17, 18, 19}));
  // EXPECT_EQ(GetProtected(arc), (std::vector<int> {5, 8}));
//}

// TEST(ARC, PutFirst) {
//   ARC arc(10, std::hash<int>{}, std::equal_to<int>{}, 0.8);

//   EXPECT_TRUE(arc.Put(0, 0));
//   EXPECT_EQ(arc.GetSize(), 1);
// }

// TEST(ARC, Get) {
//   ARC arc(10, std::hash<int>{}, std::equal_to<int>{}, 0.8);
//   EXPECT_FALSE(arc.Get(0));

//   for (int i = 0; i < 8; i++)
//     arc.Put(i, i);

//   for (int i = 0; i < 8; i++) {
//     auto *res = arc.Get(i);
//     EXPECT_TRUE(res);
//     EXPECT_EQ(*res, i);
//   }

//   EXPECT_EQ(GetProbation(arc), (std::vector<int> {0, 1, 2, 3, 4, 5}));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {6, 7}));
//   arc.Erase(6);
//   arc.Erase(7);
//   arc.Put(6, 6);
//   arc.Put(7, 7);

//   arc.Put(8, 8);
//   EXPECT_EQ(GetProbation(arc), (std::vector<int> {1, 2, 3, 4, 5, 6, 7, 8}));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {}));
//   EXPECT_TRUE(arc.Get(8));
//   EXPECT_EQ(GetProbation(arc), (std::vector<int> {1, 2, 3, 4, 5, 6, 7}));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {8}));

//   auto *res = arc.Get(1);
//   EXPECT_TRUE(res);
//   EXPECT_EQ(*res, 1);
//   EXPECT_EQ(GetProbation(arc), (std::vector<int> {2, 3, 4, 5, 6, 7}));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {1, 8}));

//   EXPECT_TRUE(arc.Get(2));
//   EXPECT_EQ(GetProbation(arc), (std::vector<int> {3, 4, 5, 6, 7, 8}));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {1, 2}));
//   EXPECT_TRUE(arc.Get(1));
//   EXPECT_TRUE(arc.Get(2));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {1, 2}));

//   arc.Put(9, 9);
//   arc.Put(10, 10);
//   for (int i = 3; i < 11; i++) {
//     auto *res = arc.Get(i);
//     EXPECT_TRUE(res);
//     EXPECT_EQ(*res, i);
//   }
//   EXPECT_TRUE(arc.Get(1));
//   EXPECT_TRUE(arc.Get(2));
//   EXPECT_EQ(GetProbation(arc), (std::vector<int> {3, 4, 5, 6, 7, 8, 9, 10}));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {1, 2}));
// }

// TEST(ARC, Erase) {
//   ARC arc(10, std::hash<int>{}, std::equal_to<int>{}, 0.8);
//   for (int i = 0; i < 8; i++)
//     arc.Put(i, i);

//   arc.Get(2);

//   arc.Erase(10);
//   arc.Erase(1);
//   arc.Erase(2);

//   EXPECT_FALSE(arc.Get(1));
//   EXPECT_FALSE(arc.Get(2));
// }

// TEST(ARC, Size1) {
//   arc arc(1, std::hash<int>{}, std::equal_to<int>{}, 0.8);
//   arc.Put(1, 1);
//   arc.Put(2, 2);
//   EXPECT_EQ(GetProbation(arc), (std::vector<int> {2}));
//   EXPECT_EQ(GetProtected(arc), (std::vector<int> {}));
//   EXPECT_TRUE(arc.Get(2));
//   EXPECT_EQ(1, arc.GetSize());
//   EXPECT_FALSE(arc.Get(1));
// }

USERVER_NAMESPACE_END