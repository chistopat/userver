#pragma once

#include <userver/cache/impl/lru.hpp>

USERVER_NAMESPACE_BEGIN

namespace cache::impl {
  template <typename T, typename U, typename Hash, typename Equal>
  class LruBase<T, U, Hash, Equal, CachePolicy::kARC> final{
    public:
      explicit LruBase(size_t max_size, const Hash& hash, const Equal& equal);

      LruBase(LruBase&& other) noexcept = default;
      LruBase& operator=(LruBase&& other) noexcept = default;
      LruBase(const LruBase& lru) = delete;
      LruBase& operator=(const LruBase& lru) = delete;

      bool Put(const T& key, U value);

      template <typename... Args>
      U* Emplace(const T& key, Args&&... args);

      void Erase(const T& key);

      U* Get(const T& key);

      const T* GetLeastUsedKey();

      U* GetLeastUsedValue();

      void Replace(bool flag);

      void SetMaxSize(std::size_t new_max_size);

      void Clear() noexcept;

      template <typename Function>
      void VisitAll(Function&& func) const;

      template <typename Function>
      void VisitT1(Function&& func);
      template <typename Function>
      void VisitT2(Function&& func);
      template <typename Function>
      void VisitB1(Function&& func);
      template <typename Function>
      void VisitB2(Function&& func);

      template <typename Function>
      void VisitAll(Function&& func);

      // size_t GetMainSize() const { return main_.GetSize(); }

      size_t GetSize() const { return t1_.GetSize() + t2_.GetSize() ; }

    private:
      LruBase<T, U> t1_, t2_, b1_, b2_;
      size_t max_size_;
      size_t size_;
      size_t p_;
  };



  template <typename T, typename U, typename Hash, typename Equal>
  LruBase<T, U, Hash, Equal, CachePolicy::kARC>::LruBase(
      size_t max_size, const Hash& hash, const Equal& equal)
      : t1_(static_cast<size_t>(max_size/4 + (max_size % 4 >= 1 ? 1 : 0)), hash, equal),
        t2_(static_cast<size_t>(max_size/4 + (max_size % 4 >= 2 ? 1 : 0)), hash, equal),
        b1_(static_cast<size_t>(max_size/4 + (max_size % 4 >= 3 ? 1 : 0)), hash, equal),
        b2_(static_cast<size_t>(max_size/4), hash, equal),
        max_size_(max_size),
        size_((max_size/4) * 2 + (max_size % 4 >= 1 ? 1 : 0) + (max_size % 4 >= 2 ? 1 : 0)),
        p_(0) {}


  template <typename T, typename U, typename Hash, typename Eq>
  bool LruBase<T, U, Hash, Eq, CachePolicy::kARC>::Put(const T& key, U value) {
    if (t1_.Get(key) != NULL) {
        t1_.Erase(key);
        t2_.Put(key, value);
        return true;
    }

    if (t2_.Get(key) != NULL) {
        t2_.Put(key, value);
        return true;
    }

    size_t delta = 1;

    std::size_t b1Size = b1_.GetSize();
    std::size_t b2Size = b2_.GetSize();

    std::size_t size_ghost = max_size_ - size_;


    if (b1_.Get(key) != NULL) {
      if (b2Size > b1Size) {
          delta = b2Size / b1Size;
      }

      //std::cout << p_ << std::endl;
      p_ = std::min(size_ghost, p_ + delta);

      if (t1_.GetSize() + t2_.GetSize() >= size_) {
          Replace(false);
      }

      b1_.Erase(key);

      t2_.Put(key, value);

      return true;
    } else if (b2_.Get(key) != NULL) {
        if (b1Size > b2Size) {
            delta = b1Size / b2Size;
        }

        //std::cout << p_ << std::endl;
        p_ = std::max((size_t)0, p_ - delta);

        if (t1_.GetSize() + t2_.GetSize() >= size_) {
            Replace(true);
        }

        b2_.Erase(key);
        t2_.Put(key, value);
        return true;
    }


    if(t1_.GetSize() + t2_.GetSize() >= size_){
      Replace(false);
    }


    if (b1_.GetSize() > size_ghost - p_) {
        b1_.Erase(*b1_.GetLeastUsedKey());
    }

    if (b2_.GetSize() > p_) {
        b2_.Erase(*b2_.GetLeastUsedKey());
    }

    t1_.Put(key, value);

    return true;
  }



  template <typename T, typename U, typename Hash, typename Eq>
  U* LruBase<T, U, Hash, Eq, CachePolicy::kARC>::Get(const T& key) {
      auto value = t1_.Get(key);
      if (value != NULL) {
          t1_.Erase(key);
          t2_.Add(key, value);
          return value;
      }

      value = t2_.Get(key);
      if (value != NULL) {
          return value;
      }

      return NULL;
  }


  template <typename T, typename U, typename Hash, typename Eq>
  void LruBase<T, U, Hash, Eq, CachePolicy::kARC>::Erase(const T& key) {
      t1_.Erase(key);
      b1_.Erase(key);
      t2_.Erase(key);
      b2_.Erase(key);
  }


  template <typename T, typename U, typename Hash, typename Eq>
  void LruBase<T, U, Hash, Eq, CachePolicy::kARC>::Replace(bool flag){
      size_t t1Size = t1_.GetSize();

      if (t1Size > 0 && (t1Size > p_ || (t1Size == p_ && flag))) {
          b1_.Put(*t1_.GetLeastUsedKey(), NULL);
          t1_.Erase(*t1_.GetLeastUsedKey());
      } else {
          b2_.Put(*t2_.GetLeastUsedKey(), NULL);
          t2_.Erase(*t2_.GetLeastUsedKey());
      }
  }

  template <typename T, typename U, typename Hash, typename Eq>
  template <typename Function>
  void LruBase<T, U, Hash, Eq, CachePolicy::kARC>::VisitAll(
      Function&& func) const {
    t1_.template VisitAll(std::forward<Function>(func));
    t2_.template VisitAll(std::forward<Function>(func));
    b1_.template VisitAll(std::forward<Function>(func));
    b2_.template VisitAll(std::forward<Function>(func));
  }

  template <typename T, typename U, typename Hash, typename Eq>
  template <typename Function>
  void LruBase<T, U, Hash, Eq, CachePolicy::kARC>::VisitAll(Function&& func){
    t1_.template VisitAll(std::forward<Function>(func));
    t2_.template VisitAll(std::forward<Function>(func));
    b1_.template VisitAll(std::forward<Function>(func));
    b2_.template VisitAll(std::forward<Function>(func));
  }

  template <typename T, typename U, typename Hash, typename Equal>
  template <typename Function>
  void LruBase<T, U, Hash, Equal, CachePolicy::kARC>::VisitT1(
      Function&& func) {
    t1_.template VisitAll(func);
  }

  template <typename T, typename U, typename Hash, typename Equal>
  template <typename Function>
  void LruBase<T, U, Hash, Equal, CachePolicy::kARC>::VisitT2(
      Function&& func) {
    t2_.template VisitAll(func);
  }

  template <typename T, typename U, typename Hash, typename Equal>
  template <typename Function>
  void LruBase<T, U, Hash, Equal, CachePolicy::kARC>::VisitB1(
      Function&& func) {
    b1_.template VisitAll(func);
  }

  template <typename T, typename U, typename Hash, typename Equal>
  template <typename Function>
  void LruBase<T, U, Hash, Equal, CachePolicy::kARC>::VisitB2(
      Function&& func) {
    b2_.template VisitAll(func);
  }



  template <typename T, typename U, typename Hash, typename Eq>
  void LruBase<T, U, Hash, Eq, CachePolicy::kARC>::Clear() noexcept {
    t1_.Clear();
    t2_.Clear();
    b1_.Clear();
    t2_.Clear();
  }

  template <typename T, typename U, typename Hash, typename Eq>
  void LruBase<T, U, Hash, Eq, CachePolicy::kARC>::SetMaxSize(
      size_t new_max_size) {
    t1_.SetMaxSize(new_max_size/4 + (new_max_size % 4 >= 1 ? 1 : 0));
    t2_.SetMaxSize(new_max_size/4 + (new_max_size % 4 >= 2 ? 1 : 0));
    b1_.SetMaxSize(new_max_size/4 + (new_max_size % 4 >= 3 ? 1 : 0));
    b2_.SetMaxSize(new_max_size/4);
  }

}// namespace cache::impl

USERVER_NAMESPACE_END