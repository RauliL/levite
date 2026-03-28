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

#include <peelo/unicode/encoding/utf8.hpp>

#include "./color.hpp"
#include "./setting.hpp"
#include "./termbox2.h"
#include "./utils.hpp"

namespace setting
{
  enum class type
  {
    color,
    number,
  };

  struct variable
  {
    enum type type;
    int value;
  };

  static std::unordered_map<key, variable> mapping =
  {
    { key::background, { type::color, TB_GREEN } },
    { key::cell_background, { type::color, TB_DEFAULT } },
    { key::cell_foreground, { type::color, TB_GREEN } },
    { key::cell_width, { type::number, 10 } },
    { key::cursor_background, { type::color, TB_GREEN | TB_BRIGHT } },
    { key::cursor_foreground, { type::color, TB_BLACK } },
    { key::foreground, { type::color, TB_BLACK } },
    { key::selection_background, { type::color, TB_GREEN } },
    { key::selection_foreground, { type::color, TB_BLACK } },
    { key::status_background, { type::color, TB_DEFAULT } },
    { key::status_foreground, { type::color, TB_DEFAULT } },
  };

  static const std::unordered_map<std::u32string, key> name_mapping =
  {
    { U"background", key::background },
    { U"cell-background", key::cell_background },
    { U"cell-foreground", key::cell_foreground },
    { U"cell-width", key::cell_width },
    { U"cursor-background", key::cursor_background },
    { U"cursor-foreground", key::cursor_foreground },
    { U"foreground", key::foreground },
    { U"selection-background", key::selection_background },
    { U"selection-foreground", key::selection_foreground },
    { U"status-background", key::status_background },
    { U"status-foreground", key::status_foreground },
  };

  static std::optional<key>
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
    return mapping[key].value;
  }

  std::u32string
  get_for_display(const std::u32string& name)
  {
    using peelo::unicode::encoding::utf8::decode;

    if (const auto key = find_by_name(name))
    {
      const auto& variable = mapping[*key];

      if (variable.type == type::color)
      {
        return color::get_name(variable.value);
      }

      return decode(std::to_string(variable.value));
    }

    return U"Unrecognized variable.";
  }

  std::optional<std::u32string>
  set(const std::u32string& name, const std::u32string& value)
  {
    using peelo::unicode::encoding::utf8::encode;

    if (const auto key = find_by_name(name))
    {
      auto& variable = mapping[*key];

      if (variable.type == type::color)
      {
        if (const auto color = color::find_by_name(value))
        {
          variable.value = *color;

          return std::nullopt;
        }
      } else {
        try
        {
          const auto number = std::stoi(encode(value));

          if (number > 0)
          {
            variable.value = number;

            return std::nullopt;
          }
        }
        catch (const std::exception&) {};
      }

      return U"Invalid value.";
    }

    return U"Unrecognized variable.";
  }
}

std::optional<std::u32string>
complete_setting(const std::u32string& input)
{
  for (const auto& entry : setting::name_mapping)
  {
    if (utils::starts_with(entry.first, input))
    {
      return entry.first;
    }
  }

  return std::nullopt;
}
