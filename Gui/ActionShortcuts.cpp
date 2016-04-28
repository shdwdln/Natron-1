/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2016 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "ActionShortcuts.h"

#include <stdexcept>

#include <QWidget>

#include "Gui/GuiApplicationManager.h"

NATRON_NAMESPACE_ENTER;

ActionWithShortcut::ActionWithShortcut(const std::string & group,
                                       const std::string & actionID,
                                       const std::string & actionDescription,
                                       QObject* parent,
                                       bool setShortcutOnAction)
    : QAction(parent)
    , _group( QString::fromUtf8( group.c_str() ) )
    , _shortcuts()
{
    QString actionIDStr = QString::fromUtf8( actionID.c_str() );
    std::list<QKeySequence> seq = getKeybind(_group, actionIDStr);

    if ( seq.empty() ) {
        seq.push_back( QKeySequence() );
    }
    _shortcuts.push_back( std::make_pair( actionIDStr, seq.front() ) );
    assert ( !_group.isEmpty() && !actionIDStr.isEmpty() );
    if (setShortcutOnAction) {
        setShortcut( seq.front() );
    }
    appPTR->addShortcutAction(_group, actionIDStr, this);
    setShortcutContext(Qt::WindowShortcut);
    setText( QObject::tr( actionDescription.c_str() ) );
}

ActionWithShortcut::ActionWithShortcut(const std::string & group,
                                       const std::list<std::string> & actionIDs,
                                       const std::string & actionDescription,
                                       QObject* parent,
                                       bool setShortcutOnAction)
    : QAction(parent)
    , _group( QString::fromUtf8( group.c_str() ) )
    , _shortcuts()
{
    QKeySequence seq0;

    for (std::list<std::string>::const_iterator it = actionIDs.begin(); it != actionIDs.end(); ++it) {
        QString actionIDStr = QString::fromUtf8( it->c_str() );
        std::list<QKeySequence> seq = getKeybind(_group, actionIDStr);
        if ( seq.empty() ) {
            seq.push_back( QKeySequence() );
        }
        _shortcuts.push_back( std::make_pair( actionIDStr, seq.front() ) );
        if ( it == actionIDs.begin() ) {
            seq0 = seq.front();
        }
        appPTR->addShortcutAction(_group, actionIDStr, this);
    }
    assert ( !_group.isEmpty() && !actionIDs.empty() );
    if (setShortcutOnAction) {
        setShortcut(seq0);
    }

    setShortcutContext(Qt::WindowShortcut);
    setText( QObject::tr( actionDescription.c_str() ) );
}

ActionWithShortcut::~ActionWithShortcut()
{
    assert ( !_group.isEmpty() && !_shortcuts.empty() );
    for (std::size_t i = 0; i < _shortcuts.size(); ++i) {
        appPTR->removeShortcutAction(_group, _shortcuts[i].first, this);
    }
}

void
ActionWithShortcut::setShortcutWrapper(const QString& actionID,
                                       const QKeySequence& shortcut)
{
    for (std::size_t i = 0; i < _shortcuts.size(); ++i) {
        if (_shortcuts[i].first == actionID) {
            _shortcuts[i].second = shortcut;
        }
    }
    setShortcut(shortcut);
}

TooltipActionShortcut::TooltipActionShortcut(const std::string & group,
                                             const std::string & actionID,
                                             const std::string & toolip,
                                             QWidget* parent)
    : ActionWithShortcut(group, actionID, std::string(), parent, false)
    , _widget(parent)
    , _originalTooltip( QString::fromUtf8( toolip.c_str() ) )
    , _tooltipSetInternally(false)
{
    assert(parent);
    setTooltipFromOriginalTooltip();
    _widget->installEventFilter(this);
}

TooltipActionShortcut::TooltipActionShortcut(const std::string & group,
                                             const std::list<std::string> & actionIDs,
                                             const std::string & toolip,
                                             QWidget* parent)
    : ActionWithShortcut(group, actionIDs, std::string(), parent, false)
    , _widget(parent)
    , _originalTooltip( QString::fromUtf8( toolip.c_str() ) )
    , _tooltipSetInternally(false)
{
    assert(parent);
    setTooltipFromOriginalTooltip();
    _widget->installEventFilter(this);
}

void
TooltipActionShortcut::setTooltipFromOriginalTooltip()
{
    QString finalToolTip = _originalTooltip;

    for (std::size_t i = 0; i < _shortcuts.size(); ++i) {
        finalToolTip = finalToolTip.arg( _shortcuts[i].second.toString(QKeySequence::NativeText) );
    }

    _tooltipSetInternally = true;
    _widget->setToolTip(finalToolTip);
    _tooltipSetInternally = false;
}

bool
TooltipActionShortcut::eventFilter(QObject* watched,
                                   QEvent* event)
{
    if (watched != _widget) {
        return false;
    }
    if (event->type() == QEvent::ToolTipChange) {
        if (_tooltipSetInternally) {
            return false;
        }
        _originalTooltip = _widget->toolTip();
        setTooltipFromOriginalTooltip();
    }

    return false;
}

void
TooltipActionShortcut::setShortcutWrapper(const QString& actionID,
                                          const QKeySequence& shortcut)
{
    for (std::size_t i = 0; i < _shortcuts.size(); ++i) {
        if (_shortcuts[i].first == actionID) {
            _shortcuts[i].second = shortcut;
        }
    }
    setTooltipFromOriginalTooltip();
}

NATRON_NAMESPACE_EXIT;

NATRON_NAMESPACE_USING;
#include "moc_ActionShortcuts.cpp"
