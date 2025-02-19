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
#include "./sheet.hpp"
#include "./termbox2.h"

extern std::u32string message;

using command_callback = void(*)(
  sheet&,
  const std::u32string&,
  const std::optional<std::u32string>&
);

static void
cmd_echo(
  sheet&,
  const std::u32string&,
  const std::optional<std::u32string>& arg
)
{
  message = arg.value_or(U"");
}

static void
cmd_quit(sheet&, const std::u32string&, const std::optional<std::u32string>&)
{
  tb_shutdown();
  std::exit(EXIT_SUCCESS);
}

static void
cmd_write(sheet&, const std::u32string&, const std::optional<std::u32string>&)
{
  message = U"File saving not yet implemented";
}

static const std::unordered_map<std::u32string, command_callback> commands =
{
  { U"ec", cmd_echo },
  { U"echo", cmd_echo },
  { U"q", cmd_quit },
  { U"q!", cmd_quit },
  { U"quit", cmd_quit },
  { U"quit!", cmd_quit },
  { U"w", cmd_write },
  { U"write", cmd_write },
};

void
run_command(struct sheet& sheet, const std::u32string& input)
{
  if (input.empty() || input[0] != ':')
  {
    return;
  }

  auto command = input.substr(1);
  const auto index = command.find(' ');
  std::optional<std::u32string> arg;

  if (index != std::u32string::npos)
  {
    arg = command.substr(index + 1);
    command = command.substr(0, index);
  }

  const auto function = commands.find(command);

  if (function != std::end(commands))
  {
    function->second(sheet, command, arg);
    return;
  }

  message = U"Unknown command: " + command;
}
