/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2013 - 2016 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "deskflow/ClipboardTypes.h"

#include <atomic>
#include <memory>
#include <string>
#include <string_view>

class IEventQueue;

//! Per-transfer state used to interrupt an in-progress file transfer
struct FileTransferState
{
  std::atomic<bool> interrupted{false};
};

class StreamChunker
{
public:
  static void sendClipboard(
      const std::string_view &data, size_t size, ClipboardID id, uint32_t sequence, IEventQueue *events,
      void *eventTarget
  );

  //! Send a file as a series of file chunk events on the given event queue.
  /*!
  Returns false if the transfer was interrupted via \p state.
  */
  static bool
  sendFile(const std::string &filename, IEventQueue *events, void *eventTarget,
           const std::shared_ptr<FileTransferState> &state);
};
