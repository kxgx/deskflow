/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2014 - 2016 Symless Ltd.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "deskflow/DropHelper.h"

#include "base/Log.h"

#include <QDir>

#include <algorithm>
#include <fstream>

void DropHelper::writeToDir(
    const std::string &destination, const DragFileList &fileList, const std::vector<std::string> &fileData
)
{
  LOG_DEBUG("dropping file, files=%d target=%s", fileList.size(), destination.c_str());

  if (destination.empty() || fileList.empty()) {
    LOG_ERR("drop file failed: drop target is empty");
    return;
  }

  QDir().mkpath(QString::fromStdString(destination));

  const size_t fileCount = std::min(fileList.size(), fileData.size());
  for (size_t i = 0; i < fileCount; ++i) {
    const std::string &filename = fileList.at(i).getFilename();
    QString dropTarget = QDir(QString::fromStdString(destination)).filePath(QString::fromStdString(filename));

    std::fstream file;
    file.open(dropTarget.toStdString().c_str(), std::ios::out | std::ios::binary);
    if (!file.is_open()) {
      LOG_ERR("drop file failed: can not open %s", qPrintable(dropTarget));
      continue;
    }

    file.write(fileData.at(i).c_str(), fileData.at(i).size());
    file.close();

    LOG_INFO("received file saved to %s", qPrintable(dropTarget));
  }
}
