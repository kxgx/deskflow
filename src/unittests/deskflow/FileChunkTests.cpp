/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "FileChunkTests.h"

#include "deskflow/FileChunk.h"
#include "deskflow/ProtocolTypes.h"
#include "deskflow/ProtocolUtil.h"
#include "io/IStream.h"

#include <algorithm>
#include <cstring>
#include <deque>
#include <string>

namespace {

class MemoryStream : public deskflow::IStream
{
public:
  void push(const std::string &bytes)
  {
    m_queue.push_back(bytes);
  }

  void close() override
  {
    m_queue.clear();
    m_inputShutdown = true;
  }

  uint32_t read(void *buffer, uint32_t n) override
  {
    if (m_inputShutdown || m_queue.empty() || n == 0) {
      return 0;
    }

    auto &front = m_queue.front();
    const size_t take = std::min(static_cast<size_t>(n), front.size());
    if (buffer != nullptr) {
      std::memcpy(buffer, front.data(), take);
    }

    front.erase(0, take);
    if (front.empty()) {
      m_queue.pop_front();
    }

    return static_cast<uint32_t>(take);
  }

  void write(const void *, uint32_t) override
  {
  }

  void flush() override
  {
  }

  void shutdownInput() override
  {
    close();
  }

  void shutdownOutput() override
  {
  }

  void *getEventTarget() const override
  {
    return const_cast<MemoryStream *>(this);
  }

  bool isReady() const override
  {
    return !m_inputShutdown && !m_queue.empty();
  }

  uint32_t getSize() const override
  {
    size_t total = 0;
    for (const auto &chunk : m_queue) {
      total += chunk.size();
    }
    return static_cast<uint32_t>(std::min<size_t>(total, UINT32_MAX));
  }

private:
  std::deque<std::string> m_queue;
  bool m_inputShutdown = false;
};

class BufferWriteStream : public deskflow::IStream
{
public:
  const std::string &str() const
  {
    return m_buffer;
  }

  void close() override
  {
    m_outputShutdown = true;
  }

  uint32_t read(void *, uint32_t) override
  {
    return 0;
  }

  void write(const void *buffer, uint32_t n) override
  {
    if (!m_outputShutdown && n != 0) {
      m_buffer.append(static_cast<const char *>(buffer), n);
    }
  }

  void flush() override
  {
  }

  void shutdownInput() override
  {
  }

  void shutdownOutput() override
  {
    m_outputShutdown = true;
  }

  void *getEventTarget() const override
  {
    return const_cast<BufferWriteStream *>(this);
  }

  bool isReady() const override
  {
    return false;
  }

  uint32_t getSize() const override
  {
    return 0;
  }

private:
  std::string m_buffer;
  bool m_outputShutdown = false;
};

std::string encodeFileMsg(uint8_t mark, const std::string &data)
{
  BufferWriteStream stream;
  auto payload = data;
  // note: the 4 byte message code is consumed by the message dispatcher
  // before FileChunk::assemble() runs, so only encode the arguments
  ProtocolUtil::writef(&stream, kMsgDFileTransfer + 4, mark, &payload);
  return stream.str();
}

} // namespace

void FileChunkTests::initTestCase()
{
  m_log.setFilter(LogLevel::Level::Debug);
}

void FileChunkTests::startFormatData()
{
  FileChunk *chunk = FileChunk::start("10");
  QCOMPARE(chunk->m_chunk[0], ChunkType::DataStart);
  QCOMPARE(chunk->m_chunk[1], '1');
  QCOMPARE(chunk->m_chunk[2], '0');
  QCOMPARE(chunk->m_chunk[3], '\0');
  delete chunk;
}

void FileChunkTests::formatDataChunk()
{
  std::string mockData("mock data");
  auto *chunk = FileChunk::data(reinterpret_cast<uint8_t *>(mockData.data()), mockData.size());
  QCOMPARE(chunk->m_chunk[0], ChunkType::DataChunk);
  QCOMPARE(std::string(&chunk->m_chunk[1], mockData.size()), mockData);
  QCOMPARE(chunk->m_chunk[mockData.size() + 1], '\0');
  delete chunk;
}

void FileChunkTests::endFormatData()
{
  FileChunk *chunk = FileChunk::end();
  QCOMPARE(chunk->m_chunk[0], ChunkType::DataEnd);
  QCOMPARE(chunk->m_chunk[1], '\0');
  delete chunk;
}

void FileChunkTests::assembleCompleteFile()
{
  MemoryStream stream;
  stream.push(encodeFileMsg(ChunkType::DataStart, "4"));
  stream.push(encodeFileMsg(ChunkType::DataChunk, "AB"));
  stream.push(encodeFileMsg(ChunkType::DataChunk, "CD"));
  stream.push(encodeFileMsg(ChunkType::DataEnd, ""));

  std::string cached;
  FileChunkAssemblyState state;

  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::Started);
  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::InProgress);
  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::InProgress);
  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::Finished);

  QCOMPARE(cached, std::string("ABCD"));
  QCOMPARE(state.expectedSize, static_cast<size_t>(4));
}

void FileChunkTests::assembleRejectsSizeMismatch()
{
  MemoryStream stream;
  stream.push(encodeFileMsg(ChunkType::DataStart, "5"));
  stream.push(encodeFileMsg(ChunkType::DataChunk, "AB"));
  stream.push(encodeFileMsg(ChunkType::DataEnd, ""));

  std::string cached;
  FileChunkAssemblyState state;

  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::Started);
  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::InProgress);
  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::Error);
}

void FileChunkTests::assembleRejectsBadSize()
{
  MemoryStream stream;
  stream.push(encodeFileMsg(ChunkType::DataStart, "notanumber"));

  std::string cached;
  FileChunkAssemblyState state;

  QCOMPARE(FileChunk::assemble(&stream, cached, state), TransferState::Error);
  QVERIFY(cached.empty());
  QCOMPARE(state.expectedSize, static_cast<size_t>(0));
}

QTEST_MAIN(FileChunkTests)
