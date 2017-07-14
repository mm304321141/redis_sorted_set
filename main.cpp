
#include "sorted_set.h"
#include <string>

void crash_assert(bool success)
{
  if (!success)
  {
    *static_cast<int *>(0) = 0;
  }
}

int main()
{
  redis_sorted_set ss;
  crash_assert(ss.zadd("123", 123) == 123);
  crash_assert(ss.zadd("456", 456) == 456);
  crash_assert(ss.zadd("789", 789) == 789);
  crash_assert(ss.zcount(0, 1000) == 3);
  crash_assert(ss.zcount(0, 500) == 2);
  crash_assert(ss.zrem("789") == 1);
  crash_assert(ss.zcard() == 2);
  crash_assert(ss.zincrby("123", 321 - 123) == 321);
  crash_assert(ss.zadd("456", 654) == 654);
  crash_assert(ss.zincrby("789", 987) == 987);
  ss.zrange(0, -1, [](std::string const &key, double score)
  {
  });
}

