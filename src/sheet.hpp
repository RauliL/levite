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

  inline bool
  is_valid() const
  {
    return is_valid(x, y);
  }

  static bool
  is_valid_name(const std::u32string& input);

  static std::optional<coordinates>
  parse(const std::u32string& input);

  std::u32string
  get_name() const;

  inline bool
  operator==(const coordinates& that) const
  {
    return x == that.x && y == that.y;
  }

  inline bool
  operator!=(const coordinates& that) const
  {
    return x != that.x || y != that.y;
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

struct cell
{
  struct coordinates coordinates;
  laskin::value value;

  inline bool
  is_formula() const
  {
    if (value.is(laskin::value::type::string))
    {
      const auto& source = value.as_string();

      return !source.empty() && source[0] == U'=';
    }

    return false;
  }

  inline std::u32string
  get_source() const
  {
    return value.to_string();
  }

  laskin::value
  evaluate(laskin::context& context) const;
};

struct sheet
{
  using container_type = std::unordered_map<coordinates, std::optional<cell>>;

  static constexpr char DEFAULT_SEPARATOR = ',';

  std::optional<std::filesystem::path> filename;
  bool modified;
  char separator;
  container_type grid;
  laskin::context context;

  explicit sheet();

  inline std::optional<cell>
  get(const coordinates& coords) const
  {
    const auto it = grid.find(coords);

    return it != std::end(grid) && it->second ? it->second : std::nullopt;
  }

  inline void
  set(const coordinates& coords, const laskin::value& value)
  {
    grid[coords] = { coords, value };
    modified = true;
  }

  void
  set(const coordinates& coords, const std::u32string& input);

  void
  erase(const coordinates& coords);

  bool
  join(const coordinates& c1, const coordinates& c2);

  bool
  load(const std::filesystem::path& path, char separator = DEFAULT_SEPARATOR);

  bool
  save(const std::filesystem::path& path, char separator = DEFAULT_SEPARATOR);
};
