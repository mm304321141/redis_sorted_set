
#include "red_black_tree.h"
#include "size_balanced_tree.h"
#include <string>
#include <cassert>
#include <functional>

class redis_sorted_set
{
  struct node_t
  {
    node_t *rb_parent, *rb_left, *rb_right;
    node_t *sb_parent, *sb_left, *sb_right;
    size_t rb_is_nil : 1;
    size_t sb_is_nil : 1;
    size_t rb_is_red : 1;
    size_t sb_size : sizeof(size_t) * 8 - 3;
  };
  struct value_node_t : public node_t
  {
    template<class T>
    value_node_t(T k, double s) : key(std::forward<T>(k)), score(s)
    {
    }
    std::string key;
    double score;
  };

  struct rb_interface
  {
    typedef std::string key_t;
    typedef node_t node_t;
    typedef value_node_t value_node_t;
    static key_t const &get_key(value_node_t *node)
    {
      return node->key;
    }
    static bool is_nil(node_t *node)
    {
      return node->rb_is_nil;
    }
    static void set_nil(node_t *node, bool nil)
    {
      node->rb_is_nil = nil;
    }
    static node_t *get_parent(node_t *node)
    {
      return node->rb_parent;
    }
    static void set_parent(node_t *node, node_t *parent)
    {
      node->rb_parent = parent;
    }
    static node_t *get_left(node_t *node)
    {
      return node->rb_left;
    }
    static void set_left(node_t *node, node_t *left)
    {
      node->rb_left = left;
    }
    static node_t *get_right(node_t *node)
    {
      return node->rb_right;
    }
    static void set_right(node_t *node, node_t *right)
    {
      node->rb_right = right;
    }
    static bool is_black(node_t *node)
    {
      return !node->rb_is_red;
    }
    static void set_black(node_t *node, bool black)
    {
      node->rb_is_red = !black;
    }
    static bool predicate(key_t const &left, key_t const &right)
    {
      return left < right;
    }
  };

  struct sb_interface
  {
    typedef double key_t;
    typedef node_t node_t;
    typedef value_node_t value_node_t;
    static key_t const &get_key(value_node_t *node)
    {
      return node->score;
    }
    static bool is_nil(node_t *node)
    {
      return node->sb_is_nil;
    }
    static void set_nil(node_t *node, bool nil)
    {
      node->sb_is_nil = nil;
    }
    static node_t *get_parent(node_t *node)
    {
      return node->sb_parent;
    }
    static void set_parent(node_t *node, node_t *parent)
    {
      node->sb_parent = parent;
    }
    static node_t *get_left(node_t *node)
    {
      return node->sb_left;
    }
    static void set_left(node_t *node, node_t *left)
    {
      node->sb_left = left;
    }
    static node_t *get_right(node_t *node)
    {
      return node->sb_right;
    }
    static void set_right(node_t *node, node_t *right)
    {
      node->sb_right = right;
    }
    static size_t get_size(node_t *node)
    {
      return node->sb_size;
    }
    static void set_size(node_t *node, size_t size)
    {
      node->sb_size = size;
    }
    static bool predicate(key_t const &left, key_t const &right)
    {
      return left < right;
    }
  };

  zzz::red_black_tree<rb_interface> root_;
  
  zzz::red_black_tree<rb_interface> &key_tree()
  {
    return root_;
  }
  zzz::size_balanced_tree<sb_interface> &score_tree()
  {
    return *reinterpret_cast<zzz::size_balanced_tree<sb_interface> *>(&root_);
  }

public:
  redis_sorted_set()
  {
    ::new(&root_) zzz::size_balanced_tree<sb_interface>();
  }

  ~redis_sorted_set()
  {
    key_tree().safe_destroy([](node_t *node)
    {
      delete node;
    });
    key_tree().clear();
    score_tree().clear();
    score_tree().~size_balanced_tree<sb_interface>();
  }

public:
  template<class T, class = typename std::enable_if<std::is_constructible<std::string, T>::value>::type>
  double zadd(T &&key, double score)
  {
    std::string key_find(std::forward<T>(key));
    auto find = key_tree().find(key_find);
    if (find == key_tree().end()) {
      value_node_t *node = new value_node_t(std::move(key_find), score);
      key_tree().insert(node);
      score_tree().insert(node);
      return score;
    }
    else {
      score_tree().erase(&*find);
      find->score = score;
      score_tree().insert(&*find);
      return find->score;
    }
  }

  size_t zcard()
  {
    return key_tree().size();
  }

  size_t zcount(double min, double max)
  {
    return score_tree().count(min, max);
  }

  template<class T, class = typename std::enable_if<std::is_constructible<std::string, T>::value>::type>
  double zincrby(T &&key, double score)
  {
    std::string key_find(std::forward<T>(key));
    auto find = key_tree().find(key_find);
    if (find == key_tree().end()) {
      value_node_t *node = new value_node_t(std::move(key_find), score);
      key_tree().insert(node);
      score_tree().insert(node);
      return score;
    }
    else {
      score_tree().erase(&*find);
      find->score += score;
      score_tree().insert(&*find);
      return find->score;
    }
  }

  template<class F, class = typename std::enable_if<std::is_constructible<std::function<void(std::string const &, double)>, F>::value>::type>
  void zrange(ptrdiff_t start, ptrdiff_t stop, F &&callback)
  {
    for (auto range = score_tree().slice(start, stop); range.first != range.second; ++range.first)
    {
      callback(range.first->key, range.first->score);
    }
  }

  ptrdiff_t zrank(std::string const &key)
  {
    auto find = key_tree().find(key);
    if (find == key_tree().end())
    {
      return -1;
    }
    return score_tree().rank(&*find);
  }

  size_t zrem(std::string const &key)
  {
    auto find = key_tree().find(key);
    if (find == key_tree().end())
    {
      return 0;
    }
    key_tree().erase(find);
    score_tree().erase(&*find);
    delete &*find;
    return 1;
  }

  double const *zscore(std::string const &key)
  {
    auto find = key_tree().find(key);
    if (find == key_tree().end())
    {
      return nullptr;
    }
    return &find->score;
  }
};

