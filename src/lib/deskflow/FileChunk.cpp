/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2015 - 2016 Symless Ltd.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "deskflow/FileChunk.h"

#include "base/Log.h"
#include "deskflow/ProtocolUtil.h"
#include "io/IStream.h"

#include <cstring>

FileChunk::FileChunk(size_t size) : Chunk(size)
{
  m_dataSize = size - s_fileChunkMetaSize;
}

FileChunk *FileChunk::start(const std::string &size)
{
  const size_t sizeLength = size.size();
  auto *start = new FileChunk(sizeLength + s_fileChunkMetaSize);
  char *chunk = start->m_chunk;
  chunk[0] = ChunkType::DataStart;
  memcpy(&chunk[1], size.c_str(), sizeLength);
  chunk[sizeLength + 1] = '\0';

  return start;
}

FileChunk *FileChunk::data(uint8_t *data, size_t dataSize)
{
  auto *chunk = new FileChunk(dataSize + s_fileChunkMetaSize);
  char *chunkData = chunk->m_chunk;
  chunkData[0] = ChunkType::DataChunk;
  memcpy(&chunkData[1], data, dataSize);
  chunkData[dataSize + 1] = '\0';

  return chunk;
}

FileChunk *FileChunk::end()
{
  auto *end = new FileChunk(s_fileChunkMetaSize);
  char *chunk = end->m_chunk;
  chunk[0] = ChunkType::DataEnd;
  chunk[1] = '\0';

  return end;
}

TransferState FileChunk::assemble(deskflow::IStream *stream, std::string &dataReceived, FileChunkAssemblyState &state)
{
  // parse
  uint8_t mark = 0;
  std::string content;
  if (!ProtocolUtil::readf(stream, kMsgDFileTransfer + 4, &mark, &content)) {
    return TransferState::Error;
  }

  switch (mark) {
  case ChunkType::DataStart:
    dataReceived.clear();
    try {
      state.expectedSize = std::stoull(content);
    } catch (const std::exception &) {
      LOG_ERR("invalid file size received: %s", content.c_str());
      return TransferState::Error;
    }
    LOG_DEBUG("receiving file, expected size=%zu", state.expectedSize);
    return TransferState::Started;

  case ChunkType::DataChunk:
    dataReceived.append(content);
    return TransferState::InProgress;

  case ChunkType::DataEnd:
    if (state.expectedSize != dataReceived.size()) {
      LOG_ERR(
          "corrupted file data, expected size=%zu actual size=%zu", state.expectedSize, dataReceived.size()
      );
      return TransferState::Error;
    }
    LOG_DEBUG("file transfer finished, size=%zu", dataReceived.size());
    return TransferState::Finished;
  }

  LOG_ERR("unknown file chunk mark: %d", mark);
  return TransferState::Error;
}

void FileChunk::send(deskflow::IStream *stream, uint8_t mark, char *data, size_t dataSize)
{
  std::string chunk(data, dataSize);

  switch (mark) {
  case ChunkType::DataStart:
    LOG_DEBUG("sending file chunk start: size=%s", data);
    break;

  case ChunkType::DataChunk:
    LOG_VERBOSE("sending file chunk: size=%zu", chunk.size());
    break;

  case ChunkType::DataEnd:
    LOG_DEBUG("sending file finished");
    break;
  }

  ProtocolUtil::writef(stream, kMsgDFileTransfer, mark, &chunk);
}
