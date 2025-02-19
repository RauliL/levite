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
#include <cstring>

#include <peelo/unicode/encoding/utf8.hpp>

#include "./input.hpp"
#include "./sheet.hpp"
#include "./termbox2.h"

static constexpr int CELL_WIDTH = 9;

inline int
cell_x_to_screen_x(int x)
{
  return (CELL_WIDTH * x) + 3;
}

inline int
cell_y_to_screen_y(int y)
{
  return y + 2;
}

static void
render_ui()
{
  const auto width = tb_width();
  const auto height = tb_height();

  for (int x = 0; x < width; ++x)
  {
    tb_set_cell(x, 0, ' ', TB_BLACK, TB_GREEN);
    tb_set_cell(x, 1, ' ', TB_BLACK, TB_GREEN);
  }
  tb_set_cell(7, 1, 'A', TB_BLACK, TB_GREEN);
  tb_set_cell(16, 1, 'B', TB_BLACK, TB_GREEN);
  tb_set_cell(25, 1, 'C', TB_BLACK, TB_GREEN);
  tb_set_cell(34, 1, 'D', TB_BLACK, TB_GREEN);
  for (int y = 2; y < height; ++y)
  {
    tb_printf(0, y, TB_BLACK, TB_GREEN, "% 3d", y - 1);
  }
}

static void
render_status(const struct sheet& sheet)
{
  using peelo::unicode::encoding::utf8::encode;

  const auto key = get_cell_name(sheet.cursor_x, sheet.cursor_y);

  if (current_mode == mode::insert)
  {
    tb_printf(
      0,
      0,
      TB_BLACK,
      TB_GREEN,
      "%s %s",
      key,
      encode(input_buffer).c_str()
    );
    tb_set_cursor(input_cursor + std::strlen(key) + 1, 0);
  } else {
    const auto cell = sheet.grid.find(key);

    if (cell != std::end(sheet.grid) && cell->second)
    {
      tb_printf(
        0,
        0,
        TB_BLACK,
        TB_GREEN,
        "%s %s",
        key,
        encode(cell->second->get_source()).c_str()
      );
    } else {
      tb_printf(0, 0, TB_BLACK, TB_GREEN, "%s", key);
    }
  }
}

static void
render_cell(
  const struct cell& cell,
  struct sheet& sheet,
  bool& cursor_rendered
)
{
  using peelo::unicode::encoding::utf8::encode;

  const auto is_selected =
    cell.x == sheet.cursor_x &&
    cell.y == sheet.cursor_y;
  const auto value = cell.evaluate(sheet.context);
  std::u32string result;

  if (value.is(laskin::value::type::string))
  {
    result = value.as_string();
    if (result.length() > CELL_WIDTH)
    {
      result = result.substr(0, CELL_WIDTH);
    }
    else if (result.length() < CELL_WIDTH)
    {
      result.append(CELL_WIDTH - result.length(), U' ');
    }
  } else {
    result = value.to_string();
    if (result.length() < CELL_WIDTH)
    {
      result.insert(0, CELL_WIDTH - result.length(), ' ');
    }
  }

  tb_print(
    cell_x_to_screen_x(cell.x),
    cell_y_to_screen_y(cell.y),
    is_selected ? TB_BLACK : TB_GREEN,
    is_selected ? (TB_GREEN | TB_BRIGHT) : TB_DEFAULT,
    encode(result).c_str()
  );

  if (is_selected)
  {
    cursor_rendered = true;
  }
}

static void
render_sheet(struct sheet& sheet)
{
  bool cursor_rendered = false;

  for (const auto& cell : sheet.grid)
  {
    if (cell.second)
    {
      render_cell(*cell.second, sheet, cursor_rendered);
    }
  }
  if (!cursor_rendered)
  {
    tb_print(
      cell_x_to_screen_x(sheet.cursor_x),
      cell_y_to_screen_y(sheet.cursor_y),
      TB_BLACK,
      TB_GREEN | TB_BRIGHT,
      std::string(CELL_WIDTH, ' ').c_str()
    );
  }
}

void
render(struct sheet& sheet)
{
  tb_clear();

  render_ui();
  render_status(sheet);
  render_sheet(sheet);

  tb_present();
}
