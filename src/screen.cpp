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
#include "./screen.hpp"
#include "./termbox2.h"

static constexpr int CELL_WIDTH = 10;
static constexpr int UI_FOREGROUND = TB_BLACK;
static constexpr int UI_BACKGROUND = TB_GREEN;
static constexpr int CELL_FOREGROUND = TB_GREEN;
static constexpr int CELL_BACKGROUND = TB_DEFAULT;
static constexpr int STATUS_FOREGROUND = TB_DEFAULT;
static constexpr int STATUS_BACKGROUND = TB_DEFAULT;
static constexpr int CURSOR_FOREGROUND = TB_BLACK;
static constexpr int CURSOR_BACKGROUND = TB_GREEN | TB_BRIGHT;

static int xtop;

std::u32string message;
coordinates cursor;

bool
move_to(const coordinates& coords)
{
  if (!coords.is_valid())
  {
    return false;
  }
  xtop = std::max(0, coords.y - (tb_height() - 3) / 2);
  cursor = coords;

  return true;
}

bool
scroll_up(int count)
{
  const auto height = tb_height() - 3;

  if (xtop == 0)
  {
    return false;
  }
  xtop = std::max(0, xtop - count);
  cursor.y = std::min(cursor.y, xtop + height - 1);

  return true;
}

bool
scroll_down(int count)
{
  if (xtop >= coordinates::MAX_Y)
  {
    return false;
  }
  xtop = std::min(coordinates::MAX_Y, xtop + count);
  cursor.y = std::max(cursor.y, xtop);

  return true;
}

bool
move_cursor(enum direction direction)
{
  const auto height = tb_height() - 3;

  switch (direction)
  {
    case direction::up:
      if (cursor.y > xtop)
      {
        --cursor.y;

        return true;
      }

      return scroll_up(1);

    case direction::down:
      if (cursor.y < xtop + height - 1)
      {
        ++cursor.y;

        return true;
      }

      return scroll_down(1);

    case direction::left:
      if (cursor.x > 0)
      {
        --cursor.x;

        return true;
      }
      break;

    case direction::right:
      if (cursor.x < coordinates::MAX_X - 1)
      {
        ++cursor.x;

        return true;
      }
      break;
  }

  return false;
}

static void
render_ui()
{
  const auto width = tb_width();
  const auto height = tb_height();
  const auto display_columns = (width - 3) / CELL_WIDTH;

  for (int x = 0; x < width; ++x)
  {
    tb_set_cell(x, 0, ' ', UI_FOREGROUND, UI_BACKGROUND);
    tb_set_cell(x, height - 1, ' ', UI_FOREGROUND, UI_BACKGROUND);
  }
  for (
    int column = 0;
    column < display_columns && column < coordinates::MAX_X;
    ++column
  )
  {
    tb_set_cell(
      (column * CELL_WIDTH) + 3 + 4,
      0,
      'A' + column,
      UI_FOREGROUND,
      UI_BACKGROUND
    );
  }
  for (int y = 0, row = xtop; y < height - 3; ++y, ++row)
  {
    tb_printf(0, y + 1, UI_FOREGROUND, UI_BACKGROUND, "%3d", row + 1);
  }
}

static void
render_status(struct sheet& sheet)
{
  using peelo::unicode::encoding::utf8::encode;

  const auto height = tb_height();
  const auto name = encode(cursor.get_name());

  if (current_mode == mode::insert || current_mode == mode::command)
  {
    tb_printf(
      0,
      height - 1,
      CURSOR_FOREGROUND,
      CURSOR_BACKGROUND,
      "%s %s",
      name.c_str(),
      encode(input_buffer).c_str()
    );
    tb_set_cursor(input_cursor + name.length() + 1, height - 1);
  }
  else if (const auto cell = sheet.get(cursor))
  {
    tb_printf(
      0,
      height - 1,
      UI_FOREGROUND,
      UI_BACKGROUND,
      "%s %s",
      name.c_str(),
      encode(cell->get_source()).c_str()
    );
  } else {
    tb_printf(0, height - 1, UI_FOREGROUND, UI_BACKGROUND, "%s", name.c_str());
  }
  tb_printf(
    0,
    height - 2,
    STATUS_FOREGROUND,
    STATUS_BACKGROUND,
    encode(message).c_str()
  );
}

static void
render_cell(
  const struct cell& cell,
  laskin::context& context,
  bool& cursor_rendered
)
{
  using peelo::unicode::encoding::utf8::encode;

  const auto is_selected = cell.coordinates == cursor;
  auto value = cell.evaluate(context);
  std::u32string result;

  if (value.is(laskin::value::type::string))
  {
    result = value.as_string();
    if (result.length() > CELL_WIDTH)
    {
      result = result.substr(0, CELL_WIDTH - 1);
    }
    else if (result.length() < CELL_WIDTH)
    {
      result.append(CELL_WIDTH - result.length(), U' ');
    }
  } else {
    result = value.to_string();
    if (result.length() > CELL_WIDTH)
    {
      result = result.substr(0, CELL_WIDTH - 1);
    }
    else if (result.length() < CELL_WIDTH)
    {
      result.insert(0, CELL_WIDTH - result.length(), U' ');
    }
  }

  tb_print(
    (cell.coordinates.x * CELL_WIDTH) + 3,
    cell.coordinates.y - xtop + 1,
    is_selected ? CURSOR_FOREGROUND : CELL_FOREGROUND,
    is_selected ? CURSOR_BACKGROUND : CELL_BACKGROUND,
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
  const auto height = tb_height() - 3;
  bool cursor_rendered = false;

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < coordinates::MAX_X; ++x)
    {
      if (const auto cell = sheet.get({ x, y + xtop }))
      {
        render_cell(*cell, sheet.context, cursor_rendered);
      }
    }
  }

  if (!cursor_rendered)
  {
    tb_print(
      (CELL_WIDTH * cursor.x) + 3,
      cursor.y - xtop + 1,
      CURSOR_FOREGROUND,
      CURSOR_BACKGROUND,
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
