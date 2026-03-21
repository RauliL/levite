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
#include "./range.hpp"
#include "./sheet.hpp"

std::optional<range>
range::parse(const std::u32string& input)
{
  const auto index = input.find(U':');

  if (index != std::u32string::npos)
  {
    const auto begin = coordinates::parse(input.substr(0, index));
    const auto end = coordinates::parse(input.substr(index + 1));

    if (begin && end)
    {
      return std::make_optional<range>({ *begin, *end });
    }
  }

  return std::nullopt;
}

static inline void
evaluate(
  struct sheet& sheet,
  const struct coordinates& coordinates,
  std::vector<laskin::value>& values
)
{
  if (const auto cell = sheet.get(coordinates))
  {
    values.push_back(cell->evaluate(sheet.context));
  }
}

std::optional<std::vector<laskin::value>>
range::extract(struct sheet& sheet) const
{
  std::vector<laskin::value> values;
  coordinates current(begin);

  // A1:A1
  if (current == end)
  {
    evaluate(sheet, current, values);
  }
  // A1:A5
  else if (current.x == end.x)
  {
    // A5:A1
    if (current.y > end.y)
    {
      while (current.is_valid() && current.y >= end.y)
      {
        evaluate(sheet, current, values);
        --current.y;
      }
    } else {
      while (current.is_valid() && current.y <= end.y)
      {
        evaluate(sheet, current, values);
        ++current.y;
      }
    }
  }
  // A1:D1
  else if (current.y == end.y)
  {
    // D1:A1
    if (current.x > end.x)
    {
      while (current.is_valid() && current.x >= end.x)
      {
        evaluate(sheet, current, values);
        --current.x;
      }
    } else {
      while (current.is_valid() && current.x <= end.x)
      {
        evaluate(sheet, current, values);
        ++current.x;
      }
    }
  } else {
    return std::nullopt;
  }

  return values;
}
