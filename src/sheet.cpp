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
#include <regex>
#include <sstream>

#include <laskin/chrono.hpp>
#include <laskin/error.hpp>
#include <peelo/unicode/encoding/utf8.hpp>
#include <rapidcsv.h>

#include "./sheet.hpp"

static const std::regex CELL_PATTERN("^([A-Z])([0-9]+)$");

const char*
get_cell_name(int x, int y)
{
  static char buffer[BUFSIZ];

  std::snprintf(buffer, BUFSIZ, "%c%d", 'A' + x, y + 1);

  return buffer;
}

laskin::value
cell::evaluate(laskin::context& context) const
{
  if (is_formula())
  {
    std::stringstream out;

    try
    {
      context.clear();
      laskin::quote::parse(value.as_string().substr(1)).call(context, out);

      return context.pop();
    }
    catch (const laskin::error&)
    {
      return laskin::value::make_string(U"#ERROR");
    }
  }

  return value;
}

sheet::sheet()
  : modified(false)
  , separator(',')
  , context(
    [this](const std::u32string& unencoded_name) -> std::optional<laskin::value>
    {
      using peelo::unicode::encoding::utf8::encode;

      const auto name = encode(unencoded_name);
      std::smatch match;

      if (std::regex_search(
        std::begin(name),
        std::end(name),
        match,
        CELL_PATTERN)
      )
      {
        const int x = match[1].str()[0] - 'A';
        const int y = std::stoi(match[2].str()) - 1;
        const auto name = get_cell_name(x, y);
        const auto cell = this->grid.find(name);

        if (cell != std::end(this->grid) && cell->second)
        {
          return cell->second->evaluate(this->context);
        }
      }

      return std::nullopt;
    }
  )
  , cursor_x(0)
  , cursor_y(0) {}

void
sheet::set(int x, int y, const std::u32string& input)
{
  if (laskin::number::is_valid(input))
  {
    set(x, y, laskin::value::make_number(input));
  }
  else if (laskin::is_date(input))
  {
    set(x, y, laskin::value::make_date(input));
  }
  else if (laskin::is_time(input))
  {
    set(x, y, laskin::value::make_time(input));
  }
  else if (laskin::is_month(input))
  {
    set(x, y, laskin::value::make_month(input));
  }
  else if (laskin::is_weekday(input))
  {
    set(x, y, laskin::value::make_weekday(input));
  }
  else if (!input.compare(U"true"))
  {
    set(x, y, laskin::value::make_boolean(true));
  }
  else if (!input.compare(U"false"))
  {
    set(x, y, laskin::value::make_boolean(false));
  } else {
    set(x, y, laskin::value::make_string(input));
  }
}

void
sheet::erase(int x, int y)
{
  const auto it = grid.find(get_cell_name(x, y));

  if (it != std::end(grid))
  {
    grid.erase(it);
  }
}

void
sheet::load(const std::filesystem::path& path, char separator)
{
  using peelo::unicode::encoding::utf8::decode;

  rapidcsv::Document doc(
    path.string(),
    rapidcsv::LabelParams(-1, -1),
    rapidcsv::SeparatorParams(separator)
  );
  const auto size = doc.GetRowCount();

  for (std::size_t i = 0; i < size; ++i)
  {
    const auto row = doc.GetRow<std::string>(i);

    if (row.size() > MAX_COLUMNS)
    {
      return; // TODO: Throw exception.
    }
    for (std::size_t j = 0; j < row.size(); ++j)
    {
      set(j, i, decode(row[j]));
    }
  }
  modified = false;
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

      max_col = std::max(max_col, cell.x + 1);
      max_row = std::max(max_row, cell.y + 1);
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
      if (const auto cell = get(x, y))
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
