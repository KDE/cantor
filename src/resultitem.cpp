/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2012 Martin Kuettler <martin.kuettler@gmail.com>
*/

#include "resultitem.h"
#include "textresultitem.h"
#include "imageresultitem.h"
#include "animationresultitem.h"
#include "commandentry.h"
#include "worksheetentry.h"

#include "lib/result.h"
#include "lib/textresult.h"
#include "lib/latexresult.h"
#include "lib/imageresult.h"
#include "lib/epsresult.h"
#include "lib/animationresult.h"
#include "lib/mimeresult.h"
#include "lib/htmlresult.h"

#include <QObject>

#include <QIcon>
#include <KLocalizedString>
#include <QDebug>

ResultItem::ResultItem(Cantor::Result* result):
    m_result(result)
{
}

ResultItem* ResultItem::create(WorksheetEntry* parent, Cantor::Result* result)
{
    switch(result->type()) {
    case Cantor::TextResult::Type:
    case Cantor::LatexResult::Type:
    case Cantor::MimeResult::Type:
    case Cantor::HtmlResult::Type:
        {
            return new TextResultItem(parent, result);
        }
    case Cantor::ImageResult::Type:
    case Cantor::EpsResult::Type:
        {
            return new ImageResultItem(parent, result);
        }
    case Cantor::AnimationResult::Type:
        {
            return new AnimationResultItem(parent, result);
        }
    default:
        return nullptr;
    }
}

void ResultItem::addCommonActions(QObject* self, QMenu* menu)
{
    menu->addAction(QIcon::fromTheme(QLatin1String("document-export")), i18n("Save result"), self, SLOT(saveResult()));
    menu->addAction(QIcon::fromTheme(QLatin1String("edit-delete")), i18n("Remove result"), self, [this](){
        this->needRemove();
    });
}

QGraphicsObject* ResultItem::graphicsObject()
{
    return dynamic_cast<QGraphicsObject*>(this);
}

CommandEntry* ResultItem::parentEntry()
{
    return qobject_cast<CommandEntry*>(graphicsObject()->parentObject());
}

Cantor::Result* ResultItem::result()
{
    return m_result;
}

void ResultItem::needRemove()
{
    parentEntry()->removeResult(m_result);
}
