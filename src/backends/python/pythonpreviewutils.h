#ifndef PYTHONPREVIEWUTILS_H
#define PYTHONPREVIEWUTILS_H

#include "variablepreview.h"

namespace Cantor
{

VariablePreviewData::Type pythonPreviewType(const QString& typeName, QString dimensions = QString());
QByteArray pythonPreviewReference(const QString& variableName);
QString pythonPreviewCommand(const VariablePreviewData::Reference& reference, qsizetype offset, qsizetype limit);

}

#endif
