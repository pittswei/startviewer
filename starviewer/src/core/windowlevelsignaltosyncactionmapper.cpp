/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#include "windowlevelsignaltosyncactionmapper.h"

#include "q2dviewer.h"
#include "voilutpresetstooldata.h"
#include "windowlevelsyncaction.h"

namespace udg {

WindowLevelSignalToSyncActionMapper::WindowLevelSignalToSyncActionMapper(QObject *parent)
 : SignalToSyncActionMapper(parent)
{
}

WindowLevelSignalToSyncActionMapper::~WindowLevelSignalToSyncActionMapper()
{
}

void WindowLevelSignalToSyncActionMapper::mapProperty()
{
    Q2DViewer *viewer2D = Q2DViewer::castFromQViewer(m_viewer);
    if (viewer2D)
    {
        mapToSyncAction(viewer2D->getWindowLevelData()->getCurrentPreset());
    }
}

void WindowLevelSignalToSyncActionMapper::mapSignal()
{
    Q2DViewer *viewer2D = Q2DViewer::castFromQViewer(m_viewer);
    if (viewer2D)
    {
        connect(viewer2D->getWindowLevelData(), SIGNAL(currentPresetChanged(WindowLevel)), SLOT(mapToSyncAction(WindowLevel)));
        connect(viewer2D->getWindowLevelData(), SIGNAL(presetSelected(WindowLevel)), SLOT(mapToSyncAction(WindowLevel)));
    }
}

void WindowLevelSignalToSyncActionMapper::unmapSignal()
{
    Q2DViewer *viewer2D = Q2DViewer::castFromQViewer(m_viewer);
    if (viewer2D)
    {
        disconnect(viewer2D->getWindowLevelData(), SIGNAL(currentPresetChanged(WindowLevel)), this, SLOT(mapToSyncAction(WindowLevel)));
        disconnect(viewer2D->getWindowLevelData(), SIGNAL(presetSelected(WindowLevel)), this, SLOT(mapToSyncAction(WindowLevel)));
    }
}

void WindowLevelSignalToSyncActionMapper::mapToSyncAction(const WindowLevel &windowLevel)
{
    if (!m_mappedSyncAction)
    {
        m_mappedSyncAction = new WindowLevelSyncAction();
    }
    static_cast<WindowLevelSyncAction*>(m_mappedSyncAction)->setVolume(m_viewer->getMainInput());
    static_cast<WindowLevelSyncAction*>(m_mappedSyncAction)->setWindowLevel(windowLevel);
    
    emit actionMapped(m_mappedSyncAction);
}

} // End namespace udg
