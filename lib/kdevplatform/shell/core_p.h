
/*
 *   SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KDEVPLATFORM_PLATFORM_CORE_P_H
#define KDEVPLATFORM_PLATFORM_CORE_P_H

#include <shell/core.h>

#include <KAboutData>

#include <QPointer>

namespace KDevelop
{

class RunController;
class PartController;
class LanguageController;
class DocumentController;
class ProjectController;
class PluginController;
class UiController;
class SessionController;
class SourceFormatterController;
class ProgressManager;
class SelectionController;
class DocumentationController;
class DebugController;
class WorkingSetController;
class TestController;
class RuntimeController;

class KDEVPLATFORMSHELL_EXPORT CorePrivate {
public:
    explicit CorePrivate(Core *core);
    ~CorePrivate();
    bool initialize( Core::Setup mode, const QString& session );
    QPointer<PluginController> pluginController;
    QPointer<UiController> uiController;
    QPointer<ProjectController> projectController;
    QPointer<LanguageController> languageController;
    QPointer<PartController> partController;
    QPointer<DocumentController> documentController;
    QPointer<RunController> runController;
    QPointer<SessionController> sessionController;
    QPointer<SourceFormatterController> sourceFormatterController;
    QPointer<ProgressManager> progressController;
    QPointer<SelectionController> selectionController;
    QPointer<DocumentationController> documentationController;
    QPointer<DebugController> debugController;
    QPointer<WorkingSetController> workingSetController;
    QPointer<TestController> testController;
    QPointer<RuntimeController> runtimeController;

    const KAboutData m_aboutData;
    Core* const m_core;
    bool m_cleanedUp;
    bool m_shuttingDown;
    Core::Setup m_mode;
};

}

#endif
