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
#include <laskin/chrono.hpp>
#include <laskin/error.hpp>
#include <peelo/unicode/encoding/utf8.hpp>
#include <rapidcsv.h>

#include "./range.hpp"
#include "./sheet.hpp"

sheet::sheet()
  : modified(false)
  , separator(',')
  , context(
    [this](const std::u32string& name) -> std::optional<laskin::value>
    {
      using peelo::unicode::encoding::utf8::encode;

      if (const auto range = range::parse(name))
      {
        if (const auto values = range->extract(*this))
        {
          return *values;
        }
      }
      else if (const auto coords = coordinates::parse(name))
      {
        const auto cell = this->grid.find(*coords);

        if (cell != std::end(this->grid) && cell->second)
        {
          return cell->second->evaluate(this->context);;
        }
      }

      return std::nullopt;
    },
    false
  ) {}

void
sheet::set(const coordinates& coords, const std::u32string& input)
{
  if (peelo::number::is_valid(input))
  {
    set(coords, laskin::value::parse_number(input));
  }
  else if (laskin::is_date(input))
  {
    set(coords, laskin::parse_date(input));
  }
  else if (laskin::is_time(input))
  {
    set(coords, laskin::parse_time(input));
  }
  else if (laskin::is_month(input))
  {
    set(coords, laskin::parse_month(input));
  }
  else if (laskin::is_weekday(input))
  {
    set(coords, laskin::parse_weekday(input));
  }
  else if (!input.compare(U"true"))
  {
    set(coords, true);
  }
  else if (!input.compare(U"false"))
  {
    set(coords, false);
  } else {
    set(coords, input);
  }
}

void
sheet::erase(const coordinates& coords)
{
  const auto it = grid.find(coords);

  if (it != std::end(grid))
  {
    grid.erase(it);
  }
}

bool
sheet::join(const coordinates& c1, const coordinates& c2)
{
  if (c1.is_valid() && c2.is_valid())
  {
    const auto cell1 = get(c1);
    const auto cell2 = get(c2);

    if (cell1 && cell2)
    {
      const auto value1 = cell1->evaluate(context);
      const auto value2 = cell2->evaluate(context);
      laskin::value result;

      try
      {
        result = value1 + value2;
      }
      catch (const laskin::error& e)
      {
        return false;
      }
      set(c1, result);
      erase(c2);

      return true;
    }
  }

  return false;
}

std::optional<std::u32string>
sheet::load(const std::filesystem::path& path, char separator)
{
  using peelo::unicode::encoding::utf8::decode;

  if (!std::filesystem::exists(path))
  {
    return U"File does not exist.";
  }

  rapidcsv::Document doc(
    path.string(),
    rapidcsv::LabelParams(-1, -1),
    rapidcsv::SeparatorParams(separator)
  );
  const auto size = doc.GetRowCount();

  if (size > coordinates::MAX_Y)
  {
    return U"Spreadsheet too long.";
  }
  grid.clear();
  for (std::size_t i = 0; i < size; ++i)
  {
    const auto row = doc.GetRow<std::string>(i);

    if (row.size() > coordinates::MAX_X)
    {
      return U"Spreadsheet too wide.";
    }
    for (std::size_t j = 0; j < row.size(); ++j)
    {
      set(
        coordinates{ static_cast<int>(j), static_cast<int>(i) },
        decode(row[j])
      );
    }
  }
  modified = false;

  return std::nullopt;
}

static std::string
escape(const std::string& input)
{
  const auto length = input.length();
  std::string result(1, '"');

  result.reserve(length + 2);
  for (std::size_t i = 0; i < length; ++i)
  {
    const auto c = input[i];

    if (c == '"')
    {
      result.append(2, '"');
    } else {
      result.append(1, c);
    }
  }

  return result.append(1, '"');
}

bool
sheet::save(const std::filesystem::path& path, char separator)
{
  using peelo::unicode::encoding::utf8::encode;

  std::ofstream out(path);
  int max_row = 0;
  int max_col = 0;

  if (!out.is_open())
  {
    return false;
  }

  // Find dimensions of used grid.
  for (const auto& pair : grid)
  {
    if (pair.second)
    {
      const auto& cell = *pair.second;

      max_col = std::max(max_col, cell.coordinates.x + 1);
      max_row = std::max(max_row, cell.coordinates.y + 1);
    }
  }

  // Write CSV data.
  for (int y = 0; y < max_row; ++y)
  {
    for (int x = 0; x < max_col; ++x)
    {
      if (x > 0)
      {
        out << separator;
      }
      if (const auto cell = get({ x, y }))
      {
        const auto source = encode(cell->get_source());

        if (
          source.find(separator) != std::string::npos ||
          source.find('"') != std::string::npos ||
          source.find('\n') != std::string::npos ||
          source.find('\r') != std::string::npos
        )
        {
          out << escape(source);
        } else {
          out << source;
        }
      }
    }
    out << '\n';
  }

  modified = false;

  return true;
}
