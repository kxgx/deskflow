/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "base/Log.h"

#include <QTest>

class FileChunkTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  // Test are run in order top to bottom
  void initTestCase();
  void startFormatData();
  void formatDataChunk();
  void endFormatData();
  void assembleCompleteFile();
  void assembleRejectsSizeMismatch();
  void assembleRejectsBadSize();

private:
  Log m_log;
};
