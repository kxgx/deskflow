/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2013 - 2016 Synergy App Ltd
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "server/ClientProxy1_5.h"

#include "base/Log.h"
#include "deskflow/FileChunk.h"
#include "deskflow/ProtocolUtil.h"
#include "io/IStream.h"
#include "server/Server.h"

#include <cstring>

//
// ClientProxy1_5
//

ClientProxy1_5::ClientProxy1_5(const std::string &name, deskflow::IStream *stream, Server *server, IEventQueue *events)
    : ClientProxy1_4(name, stream, server, events)
{
  // do nothing
}

void ClientProxy1_5::sendDragInfo(uint32_t fileCount, const char *info, size_t size)
{
  std::string data(info, size);
  ProtocolUtil::writef(getStream(), kMsgDDragInfo, fileCount, &data);
}

void ClientProxy1_5::fileChunkSending(uint8_t mark, char *data, size_t dataSize)
{
  FileChunk::send(getStream(), mark, data, dataSize);
}

bool ClientProxy1_5::parseMessage(const uint8_t *code)
{
  if (memcmp(code, kMsgDFileTransfer, 4) == 0) {
    fileChunkReceived();
  } else if (memcmp(code, kMsgDDragInfo, 4) == 0) {
    dragInfoReceived();
  } else {
    return ClientProxy1_4::parseMessage(code);
  }

  return true;
}

void ClientProxy1_5::fileChunkReceived()
{
  Server *server = getServer();
  auto result = FileChunk::assemble(getStream(), server->getReceivedFileData(), server->getFileChunkState());

  if (result == TransferState::Finished) {
    server->onFileReceived(server->getReceivedFileData());
  } else if (result == TransferState::Error) {
    LOG_ERR("invalid file data received from client");
  }
}

void ClientProxy1_5::dragInfoReceived()
{
  // parse
  uint32_t fileNum = 0;
  std::string content;
  ProtocolUtil::readf(getStream(), kMsgDDragInfo + 4, &fileNum, &content);

  getServer()->dragInfoReceived(fileNum, content);
}
