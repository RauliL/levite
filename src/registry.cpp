/*
 * Copyright (c) 2026, Rauli Laine
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
#include "./registry.hpp"
#include "./sheet.hpp"

namespace registry
{
  using entry = std::vector<std::pair<coordinates, laskin::value>>;

  static std::unordered_map<char32_t, entry> registers;

  void
  yank_cell(
    struct sheet& sheet,
    char32_t reg,
    const coordinates& pos
  )
  {
    entry e;

    if (const auto cell = sheet.get(pos))
    {
      e.emplace_back(coordinates{ 0, 0 }, cell->value);
    }
    registers[reg] = std::move(e);
    registers[UNNAMED] = registers[reg];
  }

  void
  yank_range(
    struct sheet& sheet,
    char32_t reg,
    const coordinates& from,
    const coordinates& to
  )
  {
    const int min_x = std::min(from.x, to.x);
    const int max_x = std::max(from.x, to.x);
    const int min_y = std::min(from.y, to.y);
    const int max_y = std::max(from.y, to.y);
    entry e;

    for (int y = min_y; y <= max_y; ++y)
    {
      for (int x = min_x; x <= max_x; ++x)
      {
        if (const auto cell = sheet.get({ x, y }))
        {
          e.emplace_back(
            coordinates{ x - min_x, y - min_y },
            cell->value
          );
        }
      }
    }
    registers[reg] = std::move(e);
    if (reg != UNNAMED)
    {
      registers[UNNAMED] = registers[reg];
    }
  }

  bool
  paste(
    struct sheet& sheet,
    char32_t reg,
    const coordinates& at
  )
  {
    const auto it = registers.find(reg);

    if (it == registers.end() || it->second.empty())
    {
      return false;
    }

    for (const auto& [offset, value] : it->second)
    {
      const coordinates target = { at.x + offset.x, at.y + offset.y };

      if (target.is_valid())
      {
        sheet.set(target, value);
      }
    }

    return true;
  }
}
