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
#include <unordered_map>

#include "./color.hpp"
#include "./setting.hpp"
#include "./termbox2.h"

namespace setting
{
  static std::unordered_map<key, int> mapping =
  {
    { key::foreground, TB_BLACK },
    { key::background, TB_GREEN },
    { key::cell_foreground, TB_GREEN },
    { key::cell_background, TB_DEFAULT },
    { key::status_foreground, TB_DEFAULT },
    { key::status_background, TB_DEFAULT },
    { key::cursor_foreground, TB_BLACK },
    { key::cursor_background, TB_GREEN | TB_BRIGHT },
  };

  static const std::unordered_map<std::u32string, key> name_mapping =
  {
    { U"foreground", key::foreground },
    { U"background", key::background },
    { U"cell-foreground", key::cell_foreground },
    { U"cell-background", key::cell_background },
    { U"status-foreground", key::status_foreground },
    { U"status-background", key::status_background },
    { U"cursor-foreground", key::cursor_foreground },
    { U"cursor-background", key::cursor_background },
  };

  std::optional<key>
  find_by_name(const std::u32string& name)
  {
    const auto it = name_mapping.find(name);

    if (it != std::end(name_mapping))
    {
      return it->second;
    }

    return std::nullopt;
  }

  int
  get(enum key key)
  {
    return mapping[key];
  }

  std::optional<std::u32string>
  set(enum key key, const std::u32string& value)
  {
    if (const auto color = color::find_by_name(value))
    {
      mapping[key] = *color;

      return std::nullopt;
    }

    return U"Invalid value.";
  }
}
