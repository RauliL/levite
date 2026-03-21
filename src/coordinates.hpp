/*
 * Copyright (c) 2025-2026, Rauli Laine
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <algorithm>
#include <optional>
#include <string>

struct coordinates
{
  static constexpr int MAX_X = 'Z' - '@';
  static constexpr int MAX_Y = 999;

  int x;
  int y;

  static inline bool
  is_valid(int x, int y)
  {
    return x >= 0 && x < MAX_X && y >= 0 && y < MAX_Y;
  }

  static inline bool
  is_valid(const std::u32string& input)
  {
    return (
      input.length() > 1 &&
      (
        (input[0] >= U'a' && input[0] <= U'z') ||
        (input[0] >= U'A' && input[0] <= U'Z')
      ) &&
      std::all_of(
        std::begin(input) + 1,
        std::end(input),
        [](const char32_t c)
        {
          return c >= U'0' && c <= U'9';
        }
      )
    );
  }

  inline bool
  is_valid() const
  {
    return is_valid(x, y);
  }

  static std::optional<coordinates>
  parse(const std::u32string& input);

  std::u32string
  to_string() const;

  inline bool
  equals(const coordinates& that) const
  {
    return x == that.x && y == that.y;
  }

  inline bool
  operator==(const coordinates& that) const
  {
    return equals(that);
  }

  inline bool
  operator!=(const coordinates& that) const
  {
    return !equals(that);
  }

  inline int
  compare(const coordinates& that) const
  {
    if (x > that.x)
    {
      return 1;
    }
    else if (x < that.x)
    {
      return -1;
    }
    else if (y > that.y)
    {
      return 1;
    }
    else if (y < that.y)
    {
      return -1;
    }

    return 0;
  }

  inline bool
  operator<(const coordinates& that) const
  {
    return compare(that) < 0;
  }

  inline bool
  operator>(const coordinates& that) const
  {
    return compare(that) > 0;
  }

  inline bool
  operator<=(const coordinates& that) const
  {
    return compare(that) <= 0;
  }

  inline bool
  operator>=(const coordinates& that) const
  {
    return compare(that) >= 0;
  }
};

template<>
struct std::equal_to<coordinates>
{
  bool
  operator()(const coordinates& lhs, const coordinates& rhs) const
  {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
};

template<>
struct std::hash<coordinates>
{
  std::size_t
  operator()(const coordinates& c) const
  {
    return hash<int>()(c.x) ^ hash<int>()(c.y);
  }
};
