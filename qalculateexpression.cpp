/*************************************************************************************
*  Copyright (C) 2009 by Milian Wolff <mail@milianw.de>                               *
*                                                                                   *
*  This program is free software; you can redistribute it and/or                    *
*  modify it under the terms of the GNU General Public License                      *
*  as published by the Free Software Foundation; either version 2                   *
*  of the License, or (at your option) any later version.                           *
*                                                                                   *
*  This program is distributed in the hope that it will be useful,                  *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
*  GNU General Public License for more details.                                     *
*                                                                                   *
*  You should have received a copy of the GNU General Public License                *
*  along with this program; if not, write to the Free Software                      *
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
*************************************************************************************/

#include "qalculateexpression.h"

#include "textresult.h"
#include "helpresult.h"
#include "qalculatesession.h"
#include <KLocale>

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include <KGlobal>
#include <KMessageBox>
#include <KColorScheme>

#include <QApplication>

QalculateExpression::QalculateExpression( QalculateSession* session )
    : Cantor::Expression(session)
{
}

QalculateExpression::~QalculateExpression()
{
}

void QalculateExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);

    if (command().isEmpty()) {
        return;
    }

    // copy'n'pasted from qalculate plasma applet

    const char *ctext = command().replace(QChar(0xA3), "GBP")
                                .replace(QChar(0xA5), "JPY")
                                .replace("$", "USD")
                                .replace(QChar(0x20AC), "EUR")
                            .toLatin1().data();

    EvaluationOptions eo;

//     eo.auto_post_conversion = m_settings->convertToBestUnits() ? POST_CONVERSION_BEST : POST_CONVERSION_NONE;
    eo.keep_zero_units = false;
    /*TODO: enable settings
    switch (m_settings->angleUnit()) {
        case 0:
            eo.parse_options.angle_unit = ANGLE_UNIT_NONE;
            break;
        case 1:
            eo.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
            break;
        case 2:
            eo.parse_options.angle_unit = ANGLE_UNIT_DEGREES;
            break;
        case 3:
            eo.parse_options.angle_unit = ANGLE_UNIT_GRADIANS;
            break;
    }

    eo.parse_options.rpn = m_settings->rpn();
    eo.parse_options.base = m_settings->base();
    eo.parse_options.preserve_format = m_settings->preserveFormat();
    eo.parse_options.read_precision = (ReadPrecisionMode) m_settings->readPrecisionMode();


    switch (m_settings->structuring()) {
        case 0:
            eo.structuring = STRUCTURING_NONE;
            break;
        case 1:
            eo.structuring = STRUCTURING_SIMPLIFY;
            break;
        case 2:
            eo.structuring = STRUCTURING_FACTORIZE;
            break;
    }
    */

    MathStructure result = CALCULATOR->calculate(ctext, eo);

    // error handling, most of it copied from qalculate-kde
    if ( CALCULATOR->message() ) {
        QString msg;
        KColorScheme scheme(QApplication::palette().currentColorGroup());
        const QString errorColor = scheme.foreground(KColorScheme::NegativeText).color().name();
        const QString warningColor = scheme.foreground(KColorScheme::NeutralText).color().name();
        const QString msgFormat("<font color=\"%1\">%2</font><br>\n");
        MessageType mtype;
        while(true) {
            mtype = CALCULATOR->message()->type();
            if(mtype == MESSAGE_ERROR || mtype == MESSAGE_WARNING) {
                QString text = CALCULATOR->message()->message().c_str();
                text.replace("&", "&amp;");
                text.replace(">", "&gt;");
                text.replace("<", "&lt;");

                if (mtype == MESSAGE_ERROR) {
                    msg.append(msgFormat.arg(errorColor, i18n("ERROR: %1", text)));
                } else {
                    msg.append(msgFormat.arg(errorColor, i18n("WARNING: %1", text)));
                }
            } else {
                KMessageBox::information(QApplication::activeWindow(), CALCULATOR->message()->message().c_str());
            }
            if(!CALCULATOR->nextMessage()) break;
        }
        if ( !msg.isEmpty() ) {
            setErrorMessage(msg);
            setStatus(Error);
            return;
        }
    }

    PrintOptions po;
    /*
    switch (m_settings->fractionDisplay()) {
    case 0:
        po.number_fraction_format = FRACTION_DECIMAL;
        break;
    case 1:
        po.number_fraction_format = FRACTION_DECIMAL_EXACT;
        break;
    case 2:
        po.number_fraction_format = FRACTION_FRACTIONAL;
        break;
    case 3:
        po.number_fraction_format = FRACTION_COMBINED;
        break;
    }
    po.indicate_infinite_series = m_settings->indicateInfiniteSeries();
    po.use_all_prefixes = m_settings->useAllPrefixes();
    po.use_denominator_prefix = m_settings->useDenominatorPrefix();
    po.negative_exponents = m_settings->negativeExponents();
    */
    //     po.lower_case_e = true;
    //     po.base = m_settings->baseDisplay();
    po.decimalpoint_sign = KGlobal::locale()->decimalSymbol().toLocal8Bit().data();

    /*
    switch (m_settings->minExp()) {
    case 0:
        po.min_exp = EXP_NONE;
        break;
    case 1:
        po.min_exp = EXP_PURE;
        break;
    case 2:
        po.min_exp = EXP_SCIENTIFIC;
        break;
    case 3:
        po.min_exp = EXP_PRECISION;
        break;
    case 4:
        po.min_exp = EXP_BASE_3;
        break;
    }
    */

    result.format(po);

//     m_lastResult = result.print(po).c_str();
//     emit resultReady(m_lastResult);

//     QalculateLabels label;
//     emit formattedResultReady(label.drawStructure(result, po));

//     Analitza::Analitza* a=static_cast<QalculateSession*>(session())->analitza();
//     a->setExpression(Analitza::Expression(command()));

//     Analitza::Expression res;
//     if(a->isCorrect())
//         res=a->evaluate();

    setResult(new Cantor::TextResult(result.print(po).c_str()));
    setStatus(Done);
}
