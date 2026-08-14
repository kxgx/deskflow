/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "base/Log.h"

#include <QTest>

class DragInformationTests : public QObject
{
  Q_OBJECT
private Q_SLOTS:
  // Test are run in order top to bottom
  void initTestCase();
  void parseDragInfo_singleFile();
  void parseDragInfo_multipleFiles();
  void parseDragInfo_badData_doesNotThrow();
  void parseDragInfoFiles_fullPaths();
  void setupDragInfo_format();

private:
  Log m_log;
};
