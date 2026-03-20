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

#include "./termbox2.h"

static const std::unordered_map<std::u32string, int> mapping =
{
  { U"default", TB_DEFAULT },
  { U"black", TB_BLACK },
  { U"red", TB_RED },
  { U"green", TB_GREEN },
  { U"yellow", TB_YELLOW },
  { U"blue", TB_BLUE },
  { U"magenta", TB_MAGENTA },
  { U"cyan", TB_CYAN },
  { U"white", TB_WHITE },

  { U"bright", TB_DEFAULT | TB_BRIGHT },
  { U"bright-black", TB_BLACK | TB_BRIGHT },
  { U"bright-red", TB_RED | TB_BRIGHT },
  { U"bright-green", TB_GREEN | TB_BRIGHT },
  { U"bright-yellow", TB_YELLOW | TB_BRIGHT },
  { U"bright-blue", TB_BLUE | TB_BRIGHT },
  { U"bright-magenta", TB_MAGENTA | TB_BRIGHT },
  { U"bright-cyan", TB_CYAN | TB_BRIGHT },
  { U"bright-white", TB_WHITE | TB_BRIGHT },
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
}
