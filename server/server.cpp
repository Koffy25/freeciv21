/*
 * (c) Copyright 2020 The Freeciv21 contributors
 *
 * This file is part of Freeciv21.
 *
 * Freeciv21 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Freeciv21 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Freeciv21.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "server.h"

// Qt
#include <QDebug>
#include <QFile>

// Stuff to wait for input on stdin.
#ifdef Q_OS_WIN
#include <QtCore/QWinEventNotifier>
#include <windows.h>
#else
#include <QtCore/QSocketNotifier>
#include <unistd.h> // STDIN_FILENO
#endif

// Readline
#include <readline/readline.h>

// utility
#include "fciconv.h" // local_to_internal_string_malloc

// server
#include "sernet.h"
#include "stdinhand.h"

using namespace freeciv;

/*************************************************************************/ /**
   Creates a server. It starts working as soon as there is an event loop.
 *****************************************************************************/
server::server()
{
  // Get notifications when there's some input on stdin. This is OS-dependent
  // and Qt doesn't have a wrapper. Maybe it should be split to a separate
  // class.
#ifdef Q_OS_WIN
  {
    auto handle = GetStdHandle(STD_INPUT_HANDLE);
    if (handle != INVALID_HANDLE_VALUE) {
      auto notifier = new QWinEventNotifier(handle, this);
      connect(notifier, &QWinEventNotifier::activated, this,
              &server::input_on_stdin);
      m_stdin_notifier = notifier;
    }
  }
#else
  {
    // Unix-like
    auto notifier =
        new QSocketNotifier(STDIN_FILENO, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this,
            &server::input_on_stdin);
    m_stdin_notifier = notifier;
  }
#endif

  // Are we running an interactive session?
#ifdef Q_OS_WIN
  // isatty and fileno are deprecated on Windows
  m_interactive = _isatty(_fileno(stdin));
#else
  m_interactive = isatty(fileno(stdin));
#endif
  if (m_interactive) {
    init_interactive();
  }
}

/*************************************************************************/ /**
   Shut down a server.
 *****************************************************************************/
server::~server()
{
  if (m_interactive) {
    rl_callback_handler_remove();
  }
}

/*************************************************************************/ /**
   Initializes interactive handling of stdin with libreadline.
 *****************************************************************************/
void server::init_interactive()
{
  // Initialize readline
  rl_initialize();
  rl_callback_handler_install((char *) "> ", handle_readline_input_callback);
  rl_attempted_completion_function = freeciv_completion;
}

/*************************************************************************/ /**
   Called when there's something to read on stdin.
 *****************************************************************************/
void server::input_on_stdin()
{
  if (m_interactive) {
    // Readline does everything nicely in interactive sessions
    rl_callback_read_char();
  } else {
    // Read from the input
    QFile f;
    f.open(stdin, QIODevice::ReadOnly);
    if (f.atEnd() && m_stdin_notifier != nullptr) {
      // QSocketNotifier gets mad after EOF. Turn it off.
      m_stdin_notifier->deleteLater();
      m_stdin_notifier = nullptr;
      log_normal(_("Reached end of standard input."));
    } else {
      // Got something to read. Hopefully there's even a complete line and
      // we can process it.
      auto line = f.readLine();
      auto non_const_line =
          local_to_internal_string_malloc(line.constData());
      (void) handle_stdin_input(NULL, non_const_line);
      free(non_const_line);
    }
  }
}
