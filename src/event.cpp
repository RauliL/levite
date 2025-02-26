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
#include <peelo/unicode/ctype/isspace.hpp>

#include "./input.hpp"
#include "./screen.hpp"
#include "./termbox2.h"

mode current_mode = mode::normal;
std::u32string input_buffer;
std::u32string::size_type input_cursor;

void run_command(struct sheet& sheet, const std::u32string& command);

static inline bool
is_blank()
{
  return input_buffer.empty() || std::all_of(
    std::begin(input_buffer),
    std::end(input_buffer),
    peelo::unicode::ctype::isspace
  );
}

static void
insert_mode(struct sheet& sheet, const tb_event& event)
{
  switch (event.key)
  {
    case TB_KEY_ESC:
      input_buffer.clear();
      input_cursor = 0;
      current_mode = mode::normal;
      tb_hide_cursor();
      return;

    case TB_KEY_ENTER:
      if (current_mode == mode::insert)
      {
        if (is_blank())
        {
          sheet.erase(cursor);
        } else {
          sheet.set(cursor, input_buffer);
        }
      }
      else if (!is_blank())
      {
        run_command(sheet, input_buffer);
      }
      input_buffer.clear();
      input_cursor = 0;
      current_mode = mode::normal;
      tb_hide_cursor();
      return;

    case TB_KEY_BACKSPACE:
    case TB_KEY_BACKSPACE2:
      if (input_cursor > 0)
      {
        input_buffer.erase(--input_cursor, 1);
      }
      return;

    case TB_KEY_DELETE:
      if (input_cursor < input_buffer.length())
      {
        input_buffer.erase(input_cursor, 1);
      }
      return;

    case TB_KEY_ARROW_LEFT:
    case TB_KEY_CTRL_B:
      if (input_cursor > 0)
      {
        --input_cursor;
      }
      return;

    case TB_KEY_ARROW_RIGHT:
    case TB_KEY_CTRL_F:
      if (input_cursor < input_buffer.length())
      {
        ++input_cursor;
      }
      return;

    case TB_KEY_HOME:
    case TB_KEY_CTRL_A:
      input_cursor = 0;
      return;

    case TB_KEY_END:
    case TB_KEY_CTRL_E:
      input_cursor = input_buffer.length();
      return;

    case TB_KEY_CTRL_U:
      input_buffer.clear();
      input_cursor = 0;
      return;

    case TB_KEY_CTRL_K:
      input_buffer.erase(input_cursor);
      return;

    // TODO: ^W: Deleting by word.
  }

  if (event.ch != 0)
  {
    input_buffer.insert(input_cursor, 1, event.ch);
    ++input_cursor;
  }
}

static void
edit_current_cell(struct sheet& sheet, bool prepend = false)
{
  if (const auto cell = sheet.get(cursor))
  {
    input_buffer = cell->get_source();
    input_cursor = prepend ? 0 : input_buffer.length();
  } else {
    input_buffer.clear();
    input_cursor = 0;
  }
  current_mode = mode::insert;
}

static void
normal_mode(struct sheet& sheet, const tb_event& event)
{
  switch (event.key)
  {
    case TB_KEY_ENTER:
    case TB_KEY_INSERT:
      edit_current_cell(sheet);
      return;

    case TB_KEY_BACKSPACE:
    case TB_KEY_BACKSPACE2:
    case TB_KEY_DELETE:
      sheet.erase(cursor);
      return;

    // Move one row downwards.
    case TB_KEY_ARROW_DOWN:
      move_cursor(direction::down);
      return;

    // Move one cell to the left.
    case TB_KEY_ARROW_LEFT:
      move_cursor(direction::left);
      return;

    // Move one cell to the right.
    case TB_KEY_ARROW_RIGHT:
      move_cursor(direction::right);
      return;

    // Move one row upwards.
    case TB_KEY_ARROW_UP:
      move_cursor(direction::up);
      return;

    // Move one screen towards end of the file.
    case TB_KEY_CTRL_F:
      scroll_down(tb_height() - 3);
      break;

    // Move one screen towards beginning of the file.
    case TB_KEY_CTRL_B:
      scroll_up(tb_height() - 3);
      break;

    // Move 1/2 screen towards end of the file.
    case TB_KEY_CTRL_D:
    case TB_KEY_PGDN:
      scroll_down((tb_height() - 3) / 2);
      break;

    // Move 1/2 screen towards beginning of the file.
    case TB_KEY_CTRL_U:
    case TB_KEY_PGUP:
      scroll_up((tb_height() - 3) / 2);
      break;
  }

  switch (event.ch)
  {
    // Enter into command mode.
    case ':':
      input_buffer.clear();
      input_buffer.append(1, U':');
      input_cursor = 1;
      current_mode = mode::command;
      return;

    case 'i':
      edit_current_cell(sheet);
      break;

    case 'A':
    case 'I':
      edit_current_cell(sheet, true);
      break;

    // Move one cell to the left.
    case 'h':
      move_cursor(direction::left);
      break;

    // Move one row downwards.
    case 'j':
      move_cursor(direction::down);
      break;

    // Move one row upwards.
    case 'k':
      move_cursor(direction::up);
      break;

    // Move one cell to the right.
    case 'l':
      move_cursor(direction::right);
      break;

    // Concatenate current cell with the one above it.
    case 'J':
      if (sheet.join({ cursor.x, cursor.y - 1 }, cursor))
      {
        move_cursor(direction::up);
      }
      break;

    // Edit cell above current one.
    case 'O':
      if (move_cursor(direction::up))
      {
        edit_current_cell(sheet);
      }
      break;
  }
}

void
handle_event(struct sheet& sheet)
{
  tb_event event;

  tb_poll_event(&event);

  if (event.type == TB_EVENT_KEY)
  {
    switch (current_mode)
    {
      case mode::command:
      case mode::insert:
        insert_mode(sheet, event);
        break;

      case mode::normal:
        normal_mode(sheet, event);
        break;
    }
  }

  // TODO: Add support for mouse events.
}
