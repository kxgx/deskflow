/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2014 - 2016 Symless Ltd.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#pragma once

#include "deskflow/DragInformation.h"

#include <string>
#include <vector>

class DropHelper
{
public:
  //! Write received file data to the drop target directory.
  /*!
  Writes each file in \p fileList to \p destination, using the
  corresponding entry of \p fileData. If a file already exists at the
  destination it is overwritten.
  */
  static void
  writeToDir(const std::string &destination, const DragFileList &fileList, const std::vector<std::string> &fileData);
};
