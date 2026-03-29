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
#include <cmath>
#include <cstring>

#include <peelo/unicode/encoding/utf8.hpp>

#include "./input.hpp"
#include "./screen.hpp"
#include "./setting.hpp"
#include "./termbox2.h"

static int xtop;
static int xleft;

std::u32string message;
coordinates cursor;
std::optional<coordinates> visual_anchor;

static inline int
get_page_width()
{
  return std::floor(
    (static_cast<double>(tb_width() - 3)) /
      setting::get(setting::key::cell_width)
  );
}

static inline int
get_page_height()
{
  return tb_height() - 3;
}

bool
move_to(const coordinates& coords)
{
  if (!coords.is_valid())
  {
    return false;
  }
  xtop = std::max(0, coords.y - get_page_height() / 2);
  xleft = std::max(0, coords.x - get_page_width() / 2);
  cursor = coords;

  return true;
}

bool
scroll_up(int count)
{
  const auto height = get_page_height();

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
  const auto max_top = coordinates::MAX_Y - 1;

  if (xtop >= max_top)
  {
    return false;
  }
  xtop = std::min(max_top, xtop + count);
  cursor.y = std::clamp(cursor.y, xtop, coordinates::MAX_Y - 1);

  return true;
}

bool
move_cursor(enum direction direction)
{
  const auto height = get_page_height();

  switch (direction)
  {
    case direction::up:
      if (cursor.y > 0)
      {
        if (--cursor.y < xtop)
        {
          --xtop;
        }

        return true;
      }
      break;

    case direction::down:
      if (cursor.y < coordinates::MAX_Y - 1)
      {
        if (++cursor.y >= xtop + height)
        {
          ++xtop;
        }

        return true;
      }
      break;

    case direction::left:
      if (cursor.x > 0)
      {
        if (--cursor.x < xleft)
        {
          --xleft;
        }

        return true;
      }
      break;

    case direction::right:
      if (cursor.x < coordinates::MAX_X - 1)
      {
        if (++cursor.x >= xleft + get_page_width())
        {
          ++xleft;
        }

        return true;
      }
      break;
  }

  return false;
}

void
click_on(int x, int y)
{
  const auto cell_width = setting::get(setting::key::cell_width);
  const auto page_width = get_page_width();
  const auto page_height = get_page_height();

  if (x < 3 || y < 1 || y > page_height)
  {
    return;
  }

  const int col_view = (x - 3) / cell_width;
  const int row_view = y - 1;

  if (
    col_view < 0 ||
    col_view >= page_width ||
    row_view < 0 ||
    row_view >= page_height
  )
  {
    return;
  }

  const int sx = xleft + col_view;
  const int sy = xtop + row_view;

  if (coordinates::is_valid(sx, sy))
  {
    cursor.x = sx;
    cursor.y = sy;
  }
}

bool
is_in_selection(const coordinates& coords)
{
  if (current_mode != mode::visual || !visual_anchor)
  {
    return false;
  }

  const int min_x = std::min(visual_anchor->x, cursor.x);
  const int max_x = std::max(visual_anchor->x, cursor.x);
  const int min_y = std::min(visual_anchor->y, cursor.y);
  const int max_y = std::max(visual_anchor->y, cursor.y);

  return coords.x >= min_x && coords.x <= max_x
      && coords.y >= min_y && coords.y <= max_y;
}

static void
render_ui()
{
  const auto cell_width = setting::get(setting::key::cell_width);
  const auto foreground = setting::get(setting::key::foreground);
  const auto background = setting::get(setting::key::background);
  const auto width = tb_width();
  const auto height = tb_height();
  const auto display_columns = (width - 3) / cell_width;

  for (int x = 0; x < width; ++x)
  {
    tb_set_cell(x, 0, ' ', foreground, background);
    tb_set_cell(x, height - 1, ' ', foreground, background);
  }
  for (
    int column = 0;
    column < display_columns && column + xleft < coordinates::MAX_X;
    ++column
  )
  {
    tb_set_cell(
      (column * cell_width) + 3 + (cell_width / 2),
      0,
      'A' + xleft + column,
      foreground,
      background
    );
  }
  for (
    int y = 0, row = xtop;
    y < height - 3 && row < coordinates::MAX_Y;
    ++y, ++row
  )
  {
    tb_printf(0, y + 1, foreground, background, "%3d", row + 1);
  }
}

static void
render_status(struct sheet& sheet)
{
  using peelo::unicode::encoding::utf8::encode;

  const auto height = tb_height();
  const auto name = encode(cursor.to_string());
  const auto cell = sheet.get(cursor);

  if (current_mode == mode::insert || current_mode == mode::command)
  {
    tb_printf(
      0,
      height - 1,
      setting::get(setting::key::cursor_foreground),
      setting::get(setting::key::cursor_background),
      "%s %s",
      name.c_str(),
      encode(input_buffer).c_str()
    );
    tb_set_cursor(input_cursor + name.length() + 1, height - 1);
  }
  else if (current_mode == mode::visual)
  {
    tb_printf(
      0,
      height - 1,
      setting::get(setting::key::cursor_foreground),
      setting::get(setting::key::cursor_background),
      "%s -- VISUAL --",
      name.c_str()
    );
  }
  else if (cell)
  {
    tb_printf(
      0,
      height - 1,
      setting::get(setting::key::foreground),
      setting::get(setting::key::background),
      "%s %s",
      name.c_str(),
      encode(cell->get_source()).c_str()
    );
  } else {
    tb_printf(
      0,
      height - 1,
      setting::get(setting::key::foreground),
      setting::get(setting::key::background),
      "%s",
      name.c_str()
    );
  }
  tb_printf(
    0,
    height - 2,
    setting::get(setting::key::status_foreground),
    setting::get(setting::key::status_background),
    (cell && cell->error ? *cell->error : encode(message)).c_str()
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

  const auto cell_width = setting::get(setting::key::cell_width);
  const auto is_cursor = cell.coordinates == cursor;
  const auto is_selected = is_in_selection(cell.coordinates);
  auto value = cell.evaluate(context);
  std::u32string result;

  if (value.is(laskin::value::type::string))
  {
    result = value.as_string();
    if (result.length() > static_cast<unsigned int>(cell_width))
    {
      result = result.substr(0, cell_width - 1);
    }
    else if (result.length() < static_cast<unsigned int>(cell_width))
    {
      result.append(cell_width - result.length(), U' ');
    }
  } else {
    result = value.to_string();
    if (result.length() > static_cast<unsigned int>(cell_width))
    {
      result = result.substr(0, cell_width - 1);
    }
    else if (result.length() < static_cast<unsigned int>(cell_width))
    {
      result.insert(0, cell_width - result.length(), U' ');
    }
  }

  tb_print(
    (cell_width * (cell.coordinates.x - xleft)) + 3,
    cell.coordinates.y - xtop + 1,
    setting::get(
      is_cursor   ? setting::key::cursor_foreground :
      is_selected ? setting::key::selection_foreground :
                    setting::key::cell_foreground
    ),
    setting::get(
      is_cursor   ? setting::key::cursor_background :
      is_selected ? setting::key::selection_background :
                    setting::key::cell_background
    ),
    encode(result).c_str()
  );

  if (is_cursor)
  {
    cursor_rendered = true;
  }
}

static void
render_sheet(struct sheet& sheet)
{
  const auto cell_width = setting::get(setting::key::cell_width);
  const auto cell_foreground = setting::get(setting::key::cell_foreground);
  const auto cell_background = setting::get(setting::key::cell_background);
  const auto height = get_page_height();
  const auto width = get_page_width();
  bool cursor_rendered = false;

  sheet.reset_errors();

  for (int y = 0; y < height && y < coordinates::MAX_Y; ++y)
  {
    for (int x = 0; x < width && x < coordinates::MAX_X; ++x)
    {
      const coordinates coords = { x + xleft, y + xtop };

      if (const auto cell = sheet.get(coords))
      {
        render_cell(*cell, sheet.context, cursor_rendered);
      } else {
        const auto selected = is_in_selection(coords);

        tb_print(
          (x * cell_width) + 3,
          y + 1,
          selected ? setting::get(setting::key::selection_foreground)
                   : cell_foreground,
          selected ? setting::get(setting::key::selection_background)
                   : cell_background,
          std::string(cell_width, ' ').c_str()
        );
      }
    }
  }

  if (!cursor_rendered)
  {
    tb_print(
      (cell_width * (cursor.x - xleft)) + 3,
      cursor.y - xtop + 1,
      setting::get(setting::key::cursor_foreground),
      setting::get(setting::key::cursor_background),
      std::string(cell_width, ' ').c_str()
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

void
display_messages(const std::vector<std::u32string>& messages)
{
  using peelo::unicode::encoding::utf8::encode;

  const auto fg = setting::get(setting::key::foreground);
  const auto bg = setting::get(setting::key::background);
  const auto height = tb_height();
  const auto width = tb_width();

  tb_clear();

  int y = 0;
  for (std::size_t i = 0; i < messages.size() && y < height - 1; ++i, ++y)
  {
    tb_print(0, y, fg, bg, encode(messages[i]).c_str());
  }

  tb_print(
    0,
    height - 1,
    setting::get(setting::key::status_foreground),
    setting::get(setting::key::status_background),
    std::string(width, ' ').c_str()
  );
  tb_print(
    0,
    height - 1,
    setting::get(setting::key::status_foreground),
    setting::get(setting::key::status_background),
    "Press ENTER or type command to continue"
  );

  tb_present();

  tb_event event;

  for (;;)
  {
    tb_poll_event(&event);
    if (event.type == TB_EVENT_KEY &&
        (event.key == TB_KEY_ENTER || event.key == TB_KEY_ESC ||
         event.ch == ':'))
    {
      break;
    }
  }
}
