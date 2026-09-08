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
#include <optional>
#include <string>
#include <unordered_map>

#include <libterm/libterm.h>

#include "./color.hpp"

static const std::unordered_map<std::u32string, int> mapping =
{
  { U"default", LT_DEFAULT },
  { U"black", LT_BLACK },
  { U"red", LT_RED },
  { U"green", LT_GREEN },
  { U"yellow", LT_YELLOW },
  { U"blue", LT_BLUE },
  { U"magenta", LT_MAGENTA },
  { U"cyan", LT_CYAN },
  { U"white", LT_WHITE },

  { U"bright", LT_DEFAULT | color::BRIGHT },
  { U"bright-black", LT_BLACK | color::BRIGHT },
  { U"bright-red", LT_RED | color::BRIGHT },
  { U"bright-green", LT_GREEN | color::BRIGHT },
  { U"bright-yellow", LT_YELLOW | color::BRIGHT },
  { U"bright-blue", LT_BLUE | color::BRIGHT },
  { U"bright-magenta", LT_MAGENTA | color::BRIGHT },
  { U"bright-cyan", LT_CYAN | color::BRIGHT },
  { U"bright-white", LT_WHITE | color::BRIGHT },
};

namespace color
{
  std::optional<int>
  find_by_name(const std::u32string& name)
  {
    const auto it = mapping.find(name);

    if (it != std::end(mapping))
    {
      return it->second;
    }

    return std::nullopt;
  }

  std::u32string
  get_name(int color)
  {
    for (const auto& entry : mapping)
    {
      if (entry.second == color)
      {
        return entry.first;
      }
    }

    return U"unknown";
  }

  lt_attr
  to_lt_attr(int color)
  {
    lt_attr attr = static_cast<lt_attr>(color & ~BRIGHT);

    if (color & BRIGHT)
    {
      attr |= LT_BOLD;
    }

    return attr;
  }
}
