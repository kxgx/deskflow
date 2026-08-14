/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2026 Deskflow Developers
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "DragInformationTests.h"

#include "base/Log.h"
#include "deskflow/DragInformation.h"

#include <QTemporaryFile>

void DragInformationTests::initTestCase()
{
  m_log.setFilter(LogLevel::Level::Debug);
}

void DragInformationTests::parseDragInfo_singleFile()
{
  DragFileList list;
  DragInformation::parseDragInfo(list, 1, "C:\\dir\\file.txt,1234,");

  QCOMPARE(list.size(), 1);
  QCOMPARE(QString::fromStdString(list.at(0).getFilename()), QStringLiteral("file.txt"));
  QCOMPARE(list.at(0).getFilesize(), 1234);
}

void DragInformationTests::parseDragInfo_multipleFiles()
{
  DragFileList list;
  DragInformation::parseDragInfo(list, 3, "/home/user/a.txt,100,/home/user/b.txt,200,/home/user/c.txt,300,");

  QCOMPARE(list.size(), 3);
  QCOMPARE(QString::fromStdString(list.at(0).getFilename()), QStringLiteral("a.txt"));
  QCOMPARE(list.at(0).getFilesize(), 100);
  QCOMPARE(QString::fromStdString(list.at(1).getFilename()), QStringLiteral("b.txt"));
  QCOMPARE(list.at(1).getFilesize(), 200);
  QCOMPARE(QString::fromStdString(list.at(2).getFilename()), QStringLiteral("c.txt"));
  QCOMPARE(list.at(2).getFilesize(), 300);
}

void DragInformationTests::parseDragInfo_badData_doesNotThrow()
{
  DragFileList list;

  // malformed data must never throw or crash the process
  DragInformation::parseDragInfo(list, 3, "garbage");
  QCOMPARE(list.size(), 0);

  DragInformation::parseDragInfo(list, 1, ",,,");
  QCOMPARE(list.size(), 0);

  DragInformation::parseDragInfo(list, 1, "file.txt,notanumber,");
  QCOMPARE(list.size(), 1);
  QCOMPARE(list.at(0).getFilesize(), 0);
}

void DragInformationTests::parseDragInfoFiles_fullPaths()
{
  const auto list = DragInformation::parseDragInfoFiles("C:\\dir\\file1.txt,100,C:\\dir\\file2.txt,200,");

  QCOMPARE(list.size(), 2);
  QCOMPARE(QString::fromStdString(list.at(0).getFilename()), QStringLiteral("C:\\dir\\file1.txt"));
  QCOMPARE(list.at(0).getFilesize(), 100);
  QCOMPARE(QString::fromStdString(list.at(1).getFilename()), QStringLiteral("C:\\dir\\file2.txt"));
  QCOMPARE(list.at(1).getFilesize(), 200);
}

void DragInformationTests::setupDragInfo_format()
{
  QTemporaryFile tempFile;
  QVERIFY(tempFile.open());
  tempFile.write(QByteArray(42, 'x'));
  tempFile.close();

  DragFileList list;
  DragInformation di;
  di.setFilename(tempFile.fileName().toStdString());
  list.push_back(di);

  std::string output;
  const int count = DragInformation::setupDragInfo(list, output);

  QCOMPARE(count, 1);
  QVERIFY(output.starts_with(tempFile.fileName().toStdString() + ",42,"));
}

QTEST_MAIN(DragInformationTests)
