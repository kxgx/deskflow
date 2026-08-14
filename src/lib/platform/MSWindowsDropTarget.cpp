/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2014 - 2016 Symless Ltd.
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "platform/MSWindowsDropTarget.h"

#include <QString>

#include <cassert>
#include <shellapi.h>
#include <Shlobj.h>

MSWindowsDropTarget *MSWindowsDropTarget::s_instance = nullptr;

MSWindowsDropTarget::MSWindowsDropTarget() : m_refCount(1), m_allowDrop(false)
{
  s_instance = this;
}

MSWindowsDropTarget &MSWindowsDropTarget::instance()
{
  assert(s_instance != nullptr);
  return *s_instance;
}

HRESULT
MSWindowsDropTarget::DragEnter(IDataObject *dataObject, DWORD keyState, POINTL point, DWORD *effect)
{
  // check if data object contains a file drop
  m_allowDrop = queryDataObject(dataObject);
  if (m_allowDrop) {
    setDraggingFilenames(getDropData(dataObject));
  }

  *effect = DROPEFFECT_NONE;

  return S_OK;
}

HRESULT
MSWindowsDropTarget::DragOver(DWORD keyState, POINTL point, DWORD *effect)
{
  *effect = DROPEFFECT_NONE;

  return S_OK;
}

HRESULT
MSWindowsDropTarget::DragLeave(void)
{
  return S_OK;
}

HRESULT
MSWindowsDropTarget::Drop(IDataObject *dataObject, DWORD keyState, POINTL point, DWORD *effect)
{
  *effect = DROPEFFECT_NONE;

  return S_OK;
}

bool MSWindowsDropTarget::queryDataObject(IDataObject *dataObject)
{
  // check if it supports CF_HDROP using a HGLOBAL
  FORMATETC fmtetc = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

  return dataObject->QueryGetData(&fmtetc) == S_OK ? true : false;
}

std::vector<std::string> MSWindowsDropTarget::getDropData(IDataObject *dataObject)
{
  std::vector<std::string> filenames;

  // construct a FORMATETC object
  FORMATETC fmtEtc = {CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  STGMEDIUM stgMed;

  // see if the dataobject contains any DROP stored as a HGLOBAL
  if (dataObject->QueryGetData(&fmtEtc) == S_OK && dataObject->GetData(&fmtEtc, &stgMed) == S_OK) {
    const HDROP hDrop = static_cast<HDROP>(GlobalLock(stgMed.hGlobal));

    const UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);
    for (UINT i = 0; i < fileCount; ++i) {
      wchar_t path[MAX_PATH];
      if (DragQueryFile(hDrop, i, path, MAX_PATH) > 0) {
        filenames.emplace_back(QString::fromWCharArray(path).toStdString());
      }
    }

    GlobalUnlock(stgMed.hGlobal);

    // release the data using the COM API
    ReleaseStgMedium(&stgMed);
  }

  return filenames;
}

void MSWindowsDropTarget::setDraggingFilenames(const std::vector<std::string> &filenames)
{
  m_dragFilenames = filenames;
}

std::vector<std::string> MSWindowsDropTarget::getDraggingFilenames()
{
  return m_dragFilenames;
}

void MSWindowsDropTarget::clearDraggingFilenames()
{
  m_dragFilenames.clear();
}

HRESULT __stdcall MSWindowsDropTarget::QueryInterface(REFIID iid, void **object)
{
  if (iid == IID_IDropTarget || iid == IID_IUnknown) {
    AddRef();
    *object = this;
    return S_OK;
  } else {
    *object = 0;
    return E_NOINTERFACE;
  }
}

ULONG __stdcall MSWindowsDropTarget::AddRef(void)
{
  return InterlockedIncrement(&m_refCount);
}

ULONG __stdcall MSWindowsDropTarget::Release(void)
{
  LONG count = InterlockedDecrement(&m_refCount);

  if (count == 0) {
    delete this;
    return 0;
  } else {
    return count;
  }
}
