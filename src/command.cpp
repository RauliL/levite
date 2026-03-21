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
#include <peelo/unicode/encoding/utf8.hpp>

#include "./color.hpp"
#include "./screen.hpp"
#include "./setting.hpp"
#include "./sheet.hpp"
#include "./termbox2.h"
#include "./utils.hpp"

using command_callback = void(*)(
  sheet*,
  const std::u32string&,
  const std::optional<std::u32string>&
);

static void
cmd_echo(
  struct sheet*,
  const std::u32string&,
  const std::optional<std::u32string>& arg
)
{
  message = arg.value_or(U"");
}

static void
cmd_edit(
  struct sheet* sheet,
  const std::u32string&,
  const std::optional<std::u32string>& arg
)
{
  if (arg)
  {
    sheet->filename = *arg;
  }
  else if (!sheet->filename)
  {
    message = U"No filename.";
    return;
  }
  if (const auto error = sheet->load(*sheet->filename, sheet->separator))
  {
    message = *error;
  } else {
    message = U"File loaded.";
  }
}

static void
cmd_quit(
  struct sheet* sheet,
  const std::u32string& command,
  const std::optional<std::u32string>&
)
{
  if (sheet->modified && command.back() != '!')
  {
    message = U"File modified.";
    return;
  }
  tb_shutdown();
  std::exit(EXIT_SUCCESS);
}

static void
cmd_set(
  sheet*,
  const std::u32string&,
  const std::optional<std::u32string>& arg
)
{
  if (arg)
  {
    const auto index = arg->find(U'=');
    const auto name = index ==
      std::u32string::npos
        ? *arg
        : utils::trim(arg->substr(0, index));

    if (const auto key = setting::find_by_name(name))
    {
      if (index != std::u32string::npos)
      {
        const auto value = utils::trim(arg->substr(index + 1));

        if (const auto error = setting::set(*key, value))
        {
          message = *error;
        }
      } else {
        message = color::get_name(setting::get(*key));
      }
    } else {
      message = U"Unrecognized setting.";
    }
  } else {
    message = U"Missing setting name.";
  }
}

static void
cmd_write(
  sheet* sheet,
  const std::u32string&,
  const std::optional<std::u32string>& arg
)
{
  if (arg)
  {
    sheet->filename = *arg;
  }
  else if (!sheet->filename)
  {
    message = U"No filename.";
    return;
  }
  if (sheet->save(*sheet->filename, sheet->separator))
  {
    message = U"File saved.";
  } else {
    message = U"Error saving file.";
  }
}

static const std::unordered_map<std::u32string, command_callback> commands =
{
  { U"ec", cmd_echo },
  { U"echo", cmd_echo },
  { U"e", cmd_edit },
  { U"edit", cmd_edit },
  { U"q", cmd_quit },
  { U"q!", cmd_quit },
  { U"quit", cmd_quit },
  { U"quit!", cmd_quit },
  { U"se", cmd_set },
  { U"set", cmd_set },
  { U"w", cmd_write },
  { U"write", cmd_write },
};

void
sheet::run_command(const std::u32string& input)
{
  using peelo::unicode::encoding::utf8::encode;

  auto command = input;

  while (!command.empty() && command[0] == ':')
  {
    command = command.substr(1);
  }
  if (utils::is_blank(command) || command[0] == '"')
  {
    return;
  }

  const auto index = command.find(' ');
  std::optional<std::u32string> arg;

  if (index != std::u32string::npos)
  {
    arg = utils::trim(command.substr(index + 1));
    if (utils::is_blank(*arg))
    {
      arg.reset();
    }
    command = utils::trim(command.substr(0, index));
  }

  const auto function = commands.find(command);

  if (function != std::end(commands))
  {
    function->second(this, command, arg);
    return;
  }

  if (const auto coords = coordinates::parse(command))
  {
    move_to(*coords);
    return;
  }

  message = U"Unknown command: " + command;
}
