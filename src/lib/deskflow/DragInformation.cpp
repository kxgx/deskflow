/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2013 - 2016 Symless Ltd.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "deskflow/DragInformation.h"

#include "base/Log.h"

#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <string>

DragInformation::DragInformation() : m_filename(), m_filesize(0)
{
}

void DragInformation::parseDragInfo(DragFileList &dragFileList, uint32_t fileNum, const std::string &data)
{
  dragFileList.clear();

  std::string slash("\\");
  if (data.find("/") != std::string::npos) {
    slash = "/";
  }

  size_t startPos = 0;
  for (uint32_t index = 0; index < fileNum; ++index) {
    // filename
    const size_t nameEnd = data.find(',', startPos);
    if (nameEnd == std::string::npos || nameEnd == startPos) {
      // file number does not match, something went wrong
      LOG_WARN("drag info parse failed at file %d, got %d files", index, dragFileList.size());
      break;
    }

    const size_t nameStart = data.find_last_of(slash, nameEnd);
    DragInformation di;
    if (nameEnd - nameStart > 1) {
      di.setFilename(data.substr(nameStart + 1, nameEnd - nameStart - 1));
    }
    dragFileList.push_back(di);

    // filesize
    const size_t sizeEnd = data.find(',', nameEnd + 1);
    if (sizeEnd == std::string::npos || sizeEnd == nameEnd + 1) {
      LOG_WARN("drag info parse failed at file %d size, got %d files", index, dragFileList.size());
      break;
    }
    dragFileList.at(index).setFilesize(stringToNum(data.substr(nameEnd + 1, sizeEnd - nameEnd - 1)));

    startPos = sizeEnd + 1;
  }

  LOG_DEBUG("drag info received, total drag file number: %d", dragFileList.size());

  for (size_t i = 0; i < dragFileList.size(); ++i) {
    LOG_DEBUG("dragging file %d name: %s", i + 1, dragFileList.at(i).getFilename().c_str());
  }
}

DragFileList DragInformation::parseDragInfoFiles(const std::string &data)
{
  DragFileList fileList;
  size_t startPos = 0;
  while (startPos < data.size()) {
    const size_t nameEnd = data.find(',', startPos);
    if (nameEnd == std::string::npos || nameEnd == startPos) {
      break;
    }

    const size_t sizeEnd = data.find(',', nameEnd + 1);
    if (sizeEnd == std::string::npos || sizeEnd == nameEnd + 1) {
      break;
    }

    DragInformation di;
    di.setFilename(data.substr(startPos, nameEnd - startPos));
    di.setFilesize(stringToNum(data.substr(nameEnd + 1, sizeEnd - nameEnd - 1)));
    fileList.push_back(di);

    startPos = sizeEnd + 1;
  }
  return fileList;
}

std::string DragInformation::getDragFileExtension(const std::string &filename)
{
  const size_t findResult = filename.find_last_of(".");
  if (findResult != std::string::npos) {
    return filename.substr(findResult + 1, filename.size() - findResult - 1);
  } else {
    return "";
  }
}

int DragInformation::setupDragInfo(DragFileList &fileList, std::string &output)
{
  const auto size = static_cast<int>(fileList.size());
  for (int i = 0; i < size; ++i) {
    output.append(fileList.at(i).getFilename());
    output.append(",");
    output.append(getFileSize(fileList.at(i).getFilename()));
    output.append(",");
  }
  return size;
}

bool DragInformation::isFileValid(const std::string &filename)
{
  std::error_code error;
  return std::filesystem::is_regular_file(filename, error);
}

size_t DragInformation::stringToNum(const std::string &str)
{
  // parse without throwing so that malformed drag info received over
  // the network can never terminate the process
  char *end = nullptr;
  errno = 0;
  const unsigned long long value = std::strtoull(str.c_str(), &end, 10);
  if (errno != 0 || end == str.c_str()) {
    return 0;
  }
  return static_cast<size_t>(value);
}

std::string DragInformation::getFileSize(const std::string &filename)
{
  std::error_code error;
  const auto size = std::filesystem::file_size(filename, error);
  if (error) {
    throw std::runtime_error("failed to get file size");
  }

  return std::to_string(size);
}
