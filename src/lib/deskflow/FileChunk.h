/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2015 - 2016 Symless Ltd.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "deskflow/Chunk.h"
#include "deskflow/ProtocolTypes.h"

#include <string>

constexpr static auto s_fileChunkMetaSize = 2;

namespace deskflow {
class IStream;
}

struct FileChunkAssemblyState
{
  size_t expectedSize = 0;
};

class FileChunk : public Chunk
{
public:
  explicit FileChunk(size_t size);

  static FileChunk *start(const std::string &size);
  static FileChunk *data(uint8_t *data, size_t dataSize);
  static FileChunk *end();

  static TransferState assemble(deskflow::IStream *stream, std::string &dataReceived, FileChunkAssemblyState &state);

  static void send(deskflow::IStream *stream, uint8_t mark, char *data, size_t dataSize);
};
