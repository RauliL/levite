/*
 * Copyright (c) 2025, Rauli Laine
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

#include <filesystem>

#include "laskin/context.hpp"

const char* get_cell_name(int x, int y);

enum class direction
{
  left,
  right,
  up,
  down,
};

struct cell
{
  int x;
  int y;
  laskin::value value;

  inline bool is_formula() const
  {
    if (value.is(laskin::value::type::string))
    {
      const auto& source = value.as_string();

      return !source.empty() && source[0] == U'=';
    }

    return false;
  }

  inline std::u32string get_source() const
  {
    return value.to_string();
  }

  laskin::value evaluate(laskin::context& context) const;
};

struct sheet
{
  using container_type = std::unordered_map<std::string, std::optional<cell>>;

  static constexpr int MAX_COLUMNS = 4;
  static constexpr int MAX_ROWS = 999;

  std::optional<std::filesystem::path> filename;
  bool modified;
  char separator;
  container_type grid;
  laskin::context context;
  int cursor_x;
  int cursor_y;

  explicit sheet();

  inline std::optional<cell> get(int x, int y) const
  {
    const auto it = grid.find(get_cell_name(x, y));

    return it != std::end(grid) && it->second ? it->second : std::nullopt;
  }

  inline void set(int x, int y, const laskin::value& value)
  {
    grid[get_cell_name(x, y)] = { x, y, value };
    modified = true;
  }

  void set(int x, int y, const std::u32string& input);

  void erase(int x, int y);

  bool join(int x1, int y1, int x2, int y2);

  bool move_cursor(enum direction direction);

  bool load(const std::filesystem::path& path, char separator = ',');

  bool save(const std::filesystem::path& path, char separator = ',');
};
