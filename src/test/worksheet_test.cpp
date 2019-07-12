/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2019 Sirgienko Nikita <warquark@gmail.com>
*/

#include <QtTest>
#include <QDebug>
#include <KLocalizedString>
#include <QMovie>

#include "worksheet_test.h"
#include "../worksheet.h"
#include "../session.h"
#include "../worksheetentry.h"
#include "../textentry.h"
#include "../markdownentry.h"
#include "../commandentry.h"
#include "../latexentry.h"
#include "../lib/backend.h"
#include "../lib/expression.h"
#include "../lib/result.h"
#include "../lib/textresult.h"
#include "../lib/imageresult.h"
#include "../lib/latexresult.h"
#include "../lib/animationresult.h"
#include "../lib/mimeresult.h"
#include "../lib/htmlresult.h"

#include "config-cantor-test.h"

static const QString dataPath = QString::fromLocal8Bit(PATH_TO_TEST_NOTEBOOKS)+QLatin1String("/");

void WorksheetTest::initTestCase()
{
    const QStringList& backends = Cantor::Backend::listAvailableBackends();
    if (backends.isEmpty())
    {
        QString reason = i18n("Testing of worksheets requires a functioning backends");
        QSKIP( reason.toStdString().c_str(), SkipAll );
    }
}

Worksheet* WorksheetTest::loadWorksheet(const QString& name)
{
    Worksheet* w = new Worksheet(Cantor::Backend::getBackend(QLatin1String("null")), nullptr);
    WorksheetView v(w, nullptr);
    w->enableEmbeddedMath(false);
    w->load(dataPath + name);
    return w;
}

QString WorksheetTest::plainMarkdown(WorksheetEntry* markdownEntry)
{
    QString plain;

    if (markdownEntry->type() == MarkdownEntry::Type)
    {
        QString text = markdownEntry->toPlain(QString(), QLatin1String("\n"), QLatin1String("\n"));
        text.remove(0,1);
        text.chop(2);
        plain = text;
    }

    return plain;
}

QString WorksheetTest::plainText(WorksheetEntry* textEntry)
{
    QString plain;

    if (textEntry->type() == TextEntry::Type)
    {
        QString text = textEntry->toPlain(QString(), QLatin1String("\n"), QLatin1String("\n"));
        text.remove(0,1);
        text.chop(2);
        plain = text;
    }

    return plain;
}

QString WorksheetTest::plainLatex(WorksheetEntry* latexEntry)
{
    QString plain;

    if (latexEntry->type() == LatexEntry::Type)
    {
        QString text = latexEntry->toPlain(QString(), QLatin1String("\n"), QLatin1String("\n"));
        text.remove(0,1);
        text.chop(2);
        plain = text;
    }

    return plain;
}

int WorksheetTest::entriesCount(Worksheet* worksheet)
{
    int count = 0;
    WorksheetEntry* entry = worksheet->firstEntry();
    while (entry)
    {
        count++;
        entry = entry->next();
    }
    return count;
}

Cantor::Expression * WorksheetTest::expression(WorksheetEntry* entry)
{
    CommandEntry* command = dynamic_cast<CommandEntry*>(entry);
    if (command)
        return command->expression();
    else
        return nullptr;
}

QString WorksheetTest::plainCommand(WorksheetEntry* commandEntry)
{
    QString plain;

    if (commandEntry->type() == CommandEntry::Type)
    {
        plain = commandEntry->toPlain(QString(), QString(), QString());
    }

    return plain;
}

void WorksheetTest::testMarkdown(WorksheetEntry* &entry, const QString& content)
{
    WorksheetEntry* current = entry;
    QVERIFY(current);
    entry = entry->next();
    QCOMPARE(current->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(current), content);
}

void WorksheetTest::testTextEntry(WorksheetEntry *& entry, const QString& content)
{
    WorksheetEntry* current = entry;
    QVERIFY(current);
    entry = entry->next();
    QCOMPARE(current->type(), (int)TextEntry::Type);
    QCOMPARE(plainText(current), content);
}

void WorksheetTest::testCommandEntry(WorksheetEntry *& entry, int id, const QString& content)
{
    WorksheetEntry* current = entry;
    QVERIFY(current);
    entry = entry->next();
    QCOMPARE(current->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(current), content);
    QVERIFY(expression(current));
    QCOMPARE(expression(current)->id(), id);
    QCOMPARE(expression(current)->results().size(), 0);
}

void WorksheetTest::testCommandEntry(WorksheetEntry* entry, int id, int resultsCount, const QString& content)
{
    QVERIFY(entry);
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), content);
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), id);
    QCOMPARE(expression(entry)->results().size(), resultsCount);
}

void WorksheetTest::testLatexEntry(WorksheetEntry *& entry, const QString& content)
{
    WorksheetEntry* current = entry;
    QVERIFY(current);
    entry = entry->next();
    QCOMPARE(current->type(), (int)LatexEntry::Type);
    QCOMPARE(plainLatex(current), content);
}

void WorksheetTest::testImageResult(WorksheetEntry* entry, int index)
{
    QVERIFY(expression(entry));
    QVERIFY(expression(entry)->results().size() > index);
    QCOMPARE(expression(entry)->results().at(index)->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->results().at(index)->data().value<QImage>().isNull() == false);
}

void WorksheetTest::testTextResult(WorksheetEntry* entry, int index, const QString& content)
{
    QVERIFY(expression(entry));
    QVERIFY(expression(entry)->results().size() > index);
    QCOMPARE(expression(entry)->results().at(index)->type(), (int)Cantor::TextResult::Type);
    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(expression(entry)->results().at(index));
    QVERIFY(result->format() == Cantor::TextResult::PlainTextFormat);
    QCOMPARE(result->plain(), content);
}

void WorksheetTest::testHtmlResult(WorksheetEntry* entry, int index, const QString& content)
{
    QVERIFY(expression(entry));
    QVERIFY(expression(entry)->results().size() > index);
    QCOMPARE(expression(entry)->results().at(index)->type(), (int)Cantor::HtmlResult::Type);
    Cantor::HtmlResult* result = static_cast<Cantor::HtmlResult*>(expression(entry)->results().at(index));
    QCOMPARE(result->plain(), content);
}

void WorksheetTest::testHtmlResult(WorksheetEntry* entry, int index, const QString& plain, const QString& html)
{
    QVERIFY(expression(entry));
    QVERIFY(expression(entry)->results().size() > index);
    QCOMPARE(expression(entry)->results().at(index)->type(), (int)Cantor::HtmlResult::Type);
    Cantor::HtmlResult* result = static_cast<Cantor::HtmlResult*>(expression(entry)->results().at(index));
    QCOMPARE(result->data().toString(), html);
    QCOMPARE(result->plain(), plain);
}

void WorksheetTest::testJupyter1()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("Lecture-2B-Single-Atom-Lasing.ipynb")));

    qDebug() << w->firstEntry();
    QCOMPARE(entriesCount(w.data()), 41);

    WorksheetEntry* entry = w->firstEntry();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String("# QuTiP lecture: Single-Atom-Lasing"));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "Author: J. R. Johansson (robert@riken.jp), http://dml.riken.jp/~rob/\n"
        "\n"
        "The latest version of this [IPython notebook](http://ipython.org/ipython-doc/dev/interactive/htmlnotebook.html) lecture is available at [http://github.com/jrjohansson/qutip-lectures](http://github.com/jrjohansson/qutip-lectures).\n"
        "\n"
        "The other notebooks in this lecture series are indexed at [http://jrjohansson.github.com](http://jrjohansson.github.com)."
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "# setup the matplotlib graphics library and configure it to show \n"
        "# figures inline in the notebook\n"
        "%matplotlib inline\n"
        "import matplotlib.pyplot as plt\n"
        "import numpy as np"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 1);
    QCOMPARE(expression(entry)->results().size(), 0);

        entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "# make qutip available in the rest of the notebook\n"
        "from qutip import *\n"
        "\n"
        "from IPython.display import Image"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 2);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "# Introduction and model\n"
        "\n"
        "Consider a single atom coupled to a single cavity mode, as illustrated in the figure below. If there atom excitation rate $\\Gamma$ exceeds the relaxation rate, a population inversion can occur in the atom, and if coupled to the cavity the atom can then act as a photon pump on the cavity."
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "Image(filename='images/schematic-lasing-model.png')"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 3);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "The coherent dynamics in this model is described by the Hamiltonian\n"
        "\n"
        "$H = \\hbar \\omega_0 a^\\dagger a + \\frac{1}{2}\\hbar\\omega_a\\sigma_z + \\hbar g\\sigma_x(a^\\dagger + a)$\n"
        "\n"
        "where $\\omega_0$ is the cavity energy splitting, $\\omega_a$ is the atom energy splitting and $g$ is the atom-cavity interaction strength.\n"
        "\n"
        "In addition to the coherent dynamics the following incoherent processes are also present: \n"
        "\n"
        "1. $\\kappa$ relaxation and thermal excitations of the cavity, \n"
        "2. $\\Gamma$ atomic excitation rate (pumping process).\n"
        "\n"
        "The Lindblad master equation for the model is:\n"
        "\n"
        "$\\frac{d}{dt}\\rho = -i[H, \\rho] + \\Gamma\\left(\\sigma_+\\rho\\sigma_- - \\frac{1}{2}\\sigma_-\\sigma_+\\rho - \\frac{1}{2}\\rho\\sigma_-\\sigma_+\\right)\n"
        "+ \\kappa (1 + n_{\\rm th}) \\left(a\\rho a^\\dagger - \\frac{1}{2}a^\\dagger a\\rho - \\frac{1}{2}\\rho a^\\dagger a\\right)\n"
        "+ \\kappa n_{\\rm th} \\left(a^\\dagger\\rho a - \\frac{1}{2}a a^\\dagger \\rho - \\frac{1}{2}\\rho a a^\\dagger\\right)$\n"
        "\n"
        "in units where $\\hbar = 1$.\n"
        "\n"
        "References:\n"
        "\n"
        " * [Yi Mu, C.M. Savage, Phys. Rev. A 46, 5944 (1992)](http://dx.doi.org/10.1103/PhysRevA.46.5944)\n"
        "\n"
        " * [D.A. Rodrigues, J. Imbers, A.D. Armour, Phys. Rev. Lett. 98, 067204 (2007)](http://dx.doi.org/10.1103/PhysRevLett.98.067204)\n"
        "\n"
        " * [S. Ashhab, J.R. Johansson, A.M. Zagoskin, F. Nori, New J. Phys. 11, 023030 (2009)](http://dx.doi.org/10.1088/1367-2630/11/2/023030)"
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String("### Problem parameters"));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "w0 = 1.0  * 2 * pi  # cavity frequency\n"
        "wa = 1.0  * 2 * pi  # atom frequency\n"
        "g  = 0.05 * 2 * pi  # coupling strength\n"
        "\n"
        "kappa = 0.04        # cavity dissipation rate\n"
        "gamma = 0.00        # atom dissipation rate\n"
        "Gamma = 0.35        # atom pump rate\n"
        "\n"
        "N = 50              # number of cavity fock states\n"
        "n_th_a = 0.0        # avg number of thermal bath excitation\n"
        "\n"
        "tlist = np.linspace(0, 150, 101)"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 5);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String("### Setup the operators, the Hamiltonian and initial state"));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "# intial state\n"
        "psi0 = tensor(basis(N,0), basis(2,0)) # start without excitations\n"
        "\n"
        "# operators\n"
        "a  = tensor(destroy(N), qeye(2))\n"
        "sm = tensor(qeye(N), destroy(2))\n"
        "sx = tensor(qeye(N), sigmax())\n"
        "\n"
        "# Hamiltonian\n"
        "H = w0 * a.dag() * a + wa * sm.dag() * sm + g * (a.dag() + a) * sx"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 6);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String("H"));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 7);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::LatexResult::Type);
    {
    Cantor::LatexResult* result = static_cast<Cantor::LatexResult*>(expression(entry)->result());
    QCOMPARE(result->code(), QLatin1String(
        "Quantum object: dims = [[50, 2], [50, 2]], shape = [100, 100], type = oper, isherm = True\\begin{equation*}\\left(\\begin{array}{*{11}c}0.0 & 0.0 & 0.0 & 0.314 & 0.0 & \\cdots & 0.0 & 0.0 & 0.0 & 0.0 & 0.0\\\\0.0 & 6.283 & 0.314 & 0.0 & 0.0 & \\cdots & 0.0 & 0.0 & 0.0 & 0.0 & 0.0\\\\0.0 & 0.314 & 6.283 & 0.0 & 0.0 & \\cdots & 0.0 & 0.0 & 0.0 & 0.0 & 0.0\\\\0.314 & 0.0 & 0.0 & 12.566 & 0.444 & \\cdots & 0.0 & 0.0 & 0.0 & 0.0 & 0.0\\\\0.0 & 0.0 & 0.0 & 0.444 & 12.566 & \\cdots & 0.0 & 0.0 & 0.0 & 0.0 & 0.0\\\\\\vdots & \\vdots & \\vdots & \\vdots & \\vdots & \\ddots & \\vdots & \\vdots & \\vdots & \\vdots & \\vdots\\\\0.0 & 0.0 & 0.0 & 0.0 & 0.0 & \\cdots & 301.593 & 2.177 & 0.0 & 0.0 & 0.0\\\\0.0 & 0.0 & 0.0 & 0.0 & 0.0 & \\cdots & 2.177 & 301.593 & 0.0 & 0.0 & 2.199\\\\0.0 & 0.0 & 0.0 & 0.0 & 0.0 & \\cdots & 0.0 & 0.0 & 307.876 & 2.199 & 0.0\\\\0.0 & 0.0 & 0.0 & 0.0 & 0.0 & \\cdots & 0.0 & 0.0 & 2.199 & 307.876 & 0.0\\\\0.0 & 0.0 & 0.0 & 0.0 & 0.0 & \\cdots & 0.0 & 2.199 & 0.0 & 0.0 & 314.159\\\\\\end{array}\\right)\\end{equation*}"
    ));
    QCOMPARE(result->plain(), QLatin1String(
        "Quantum object: dims = [[50, 2], [50, 2]], shape = [100, 100], type = oper, isherm = True\n"
        "Qobj data =\n"
        "[[   0.            0.            0.         ...,    0.            0.            0.        ]\n"
        " [   0.            6.28318531    0.31415927 ...,    0.            0.            0.        ]\n"
        " [   0.            0.31415927    6.28318531 ...,    0.            0.            0.        ]\n"
        " ..., \n"
        " [   0.            0.            0.         ...,  307.87608005\n"
        "     2.19911486    0.        ]\n"
        " [   0.            0.            0.         ...,    2.19911486\n"
        "   307.87608005    0.        ]\n"
        " [   0.            0.            0.         ...,    0.            0.\n"
        "   314.15926536]]"
    ));
    QCOMPARE(result->mimeType(), QStringLiteral("image/x-eps"));
    }

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String("### Create a list of collapse operators that describe the dissipation"));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "# collapse operators\n"
        "c_ops = []\n"
        "\n"
        "rate = kappa * (1 + n_th_a)\n"
        "if rate > 0.0:\n"
        "    c_ops.append(sqrt(rate) * a)\n"
        "\n"
        "rate = kappa * n_th_a\n"
        "if rate > 0.0:\n"
        "    c_ops.append(sqrt(rate) * a.dag())\n"
        "\n"
        "rate = gamma\n"
        "if rate > 0.0:\n"
        "    c_ops.append(sqrt(rate) * sm)\n"
        "\n"
        "rate = Gamma\n"
        "if rate > 0.0:\n"
        "    c_ops.append(sqrt(rate) * sm.dag())"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 8);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "### Evolve the system\n"
        "\n"
        "Here we evolve the system with the Lindblad master equation solver, and we request that the expectation values of the operators $a^\\dagger a$ and $\\sigma_+\\sigma_-$ are returned by the solver by passing the list `[a.dag()*a, sm.dag()*sm]` as the fifth argument to the solver."
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "opt = Odeoptions(nsteps=2000) # allow extra time-steps \n"
        "output = mesolve(H, psi0, tlist, c_ops, [a.dag() * a, sm.dag() * sm], options=opt)"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 9);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "## Visualize the results\n"
        "\n"
        "Here we plot the excitation probabilities of the cavity and the atom (these expectation values were calculated by the `mesolve` above)."
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "n_c = output.expect[0]\n"
        "n_a = output.expect[1]\n"
        "\n"
        "fig, axes = plt.subplots(1, 1, figsize=(8,6))\n"
        "\n"
        "axes.plot(tlist, n_c, label=\"Cavity\")\n"
        "axes.plot(tlist, n_a, label=\"Atom excited state\")\n"
        "axes.set_xlim(0, 150)\n"
        "axes.legend(loc=0)\n"
        "axes.set_xlabel('Time')\n"
        "axes.set_ylabel('Occupation probability');"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 10);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String("## Steady state: cavity fock-state distribution and wigner function"));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String("rho_ss = steadystate(H, c_ops)"));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 11);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "fig, axes = plt.subplots(1, 2, figsize=(12,6))\n"
        "\n"
        "xvec = np.linspace(-5,5,200)\n"
        "\n"
        "rho_cavity = ptrace(rho_ss, 0)\n"
        "W = wigner(rho_cavity, xvec, xvec)\n"
        "wlim = abs(W).max()\n"
        "\n"
        "axes[1].contourf(xvec, xvec, W, 100, norm=mpl.colors.Normalize(-wlim,wlim), cmap=plt.get_cmap('RdBu'))\n"
        "axes[1].set_xlabel(r'Im $\\alpha$', fontsize=18)\n"
        "axes[1].set_ylabel(r'Re $\\alpha$', fontsize=18)\n"
        "\n"
        "axes[0].bar(arange(0, N), real(rho_cavity.diag()), color=\"blue\", alpha=0.6)\n"
        "axes[0].set_ylim(0, 1)\n"
        "axes[0].set_xlim(0, N)\n"
        "axes[0].set_xlabel('Fock number', fontsize=18)\n"
        "axes[0].set_ylabel('Occupation probability', fontsize=18);"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 13);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String("## Cavity fock-state distribution and Wigner function as a function of time"));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "tlist = np.linspace(0, 25, 5)\n"
        "output = mesolve(H, psi0, tlist, c_ops, [], options=Odeoptions(nsteps=5000))"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 14);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "rho_ss_sublist = output.states\n"
        "\n"
        "xvec = np.linspace(-5,5,200)\n"
        "\n"
        "fig, axes = plt.subplots(2, len(rho_ss_sublist), figsize=(3*len(rho_ss_sublist), 6))\n"
        "\n"
        "for idx, rho_ss in enumerate(rho_ss_sublist):\n"
        "\n"
        "    # trace out the cavity density matrix\n"
        "    rho_ss_cavity = ptrace(rho_ss, 0)\n"
        "    \n"
        "    # calculate its wigner function\n"
        "    W = wigner(rho_ss_cavity, xvec, xvec)\n"
        "    \n"
        "    # plot its wigner function\n"
        "    wlim = abs(W).max()\n"
        "    axes[0,idx].contourf(xvec, xvec, W, 100, norm=mpl.colors.Normalize(-wlim,wlim), cmap=plt.get_cmap('RdBu'))\n"
        "    axes[0,idx].set_title(r'$t = %.1f$' % tlist[idx])\n"
        "    \n"
        "    # plot its fock-state distribution\n"
        "    axes[1,idx].bar(arange(0, N), real(rho_ss_cavity.diag()), color=\"blue\", alpha=0.8)\n"
        "    axes[1,idx].set_ylim(0, 1)\n"
        "    axes[1,idx].set_xlim(0, 15)"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 15);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "## Steady state average photon occupation in cavity as a function of pump rate\n"
        "\n"
        "References:\n"
        "\n"
        " * [S. Ashhab, J.R. Johansson, A.M. Zagoskin, F. Nori, New J. Phys. 11, 023030 (2009)](http://dx.doi.org/10.1088/1367-2630/11/2/023030)"
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "def calulcate_avg_photons(N, Gamma):\n"
        "       \n"
        "    # collapse operators\n"
        "    c_ops = []\n"
        "\n"
        "    rate = kappa * (1 + n_th_a)\n"
        "    if rate > 0.0:\n"
        "        c_ops.append(sqrt(rate) * a)\n"
        "\n"
        "    rate = kappa * n_th_a\n"
        "    if rate > 0.0:\n"
        "        c_ops.append(sqrt(rate) * a.dag())\n"
        "\n"
        "    rate = gamma\n"
        "    if rate > 0.0:\n"
        "        c_ops.append(sqrt(rate) * sm)\n"
        "\n"
        "    rate = Gamma\n"
        "    if rate > 0.0:\n"
        "        c_ops.append(sqrt(rate) * sm.dag())\n"
        "      \n"
        "    # Ground state and steady state for the Hamiltonian: H = H0 + g * H1\n"
        "    rho_ss = steadystate(H, c_ops)\n"
        "    \n"
        "    # cavity photon number\n"
        "    n_cavity = expect(a.dag() * a, rho_ss)\n"
        "    \n"
        "    # cavity second order coherence function\n"
        "    g2_cavity = expect(a.dag() * a.dag() * a * a, rho_ss) / (n_cavity ** 2)\n"
        "\n"
        "    return n_cavity, g2_cavity"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 16);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "Gamma_max = 2 * (4*g**2) / kappa\n"
        "Gamma_vec = np.linspace(0.1, Gamma_max, 50)\n"
        "\n"
        "n_avg_vec = []\n"
        "g2_vec = []\n"
        "\n"
        "for Gamma in Gamma_vec:\n"
        "    n_avg, g2 = calulcate_avg_photons(N, Gamma)\n"
        "    n_avg_vec.append(n_avg)\n"
        "    g2_vec.append(g2)"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 17);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "fig, axes = plt.subplots(1, 1, figsize=(12,6))\n"
        "\n"
        "axes.plot(Gamma_vec * kappa / (4*g**2), n_avg_vec, color=\"blue\", alpha=0.6, label=\"numerical\")\n"
        "\n"
        "axes.set_xlabel(r'$\\Gamma\\kappa/(4g^2)$', fontsize=18)\n"
        "axes.set_ylabel(r'Occupation probability $\\langle n \\rangle$', fontsize=18)\n"
        "axes.set_xlim(0, 2);"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 18);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "fig, axes = plt.subplots(1, 1, figsize=(12,6))\n"
        "\n"
        "axes.plot(Gamma_vec * kappa / (4*g**2), g2_vec, color=\"blue\", alpha=0.6, label=\"numerical\")\n"
        "\n"
        "axes.set_xlabel(r'$\\Gamma\\kappa/(4g^2)$', fontsize=18)\n"
        "axes.set_ylabel(r'$g^{(2)}(0)$', fontsize=18)\n"
        "axes.set_xlim(0, 2)\n"
        "axes.text(0.1, 1.1, \"Lasing regime\", fontsize=16)\n"
        "axes.text(1.5, 1.8, \"Thermal regime\", fontsize=16);"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 19);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "Here we see that lasing is suppressed for $\\Gamma\\kappa/(4g^2) > 1$. \n"
        "\n"
        "\n"
        "Let's look at the fock-state distribution at $\\Gamma\\kappa/(4g^2) = 0.5$  (lasing regime) and $\\Gamma\\kappa/(4g^2) = 1.5$ (suppressed regime):"
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "### Case 1: $\\Gamma\\kappa/(4g^2) = 0.5$"
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "Gamma = 0.5 * (4*g**2) / kappa"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 20);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "c_ops = [sqrt(kappa * (1 + n_th_a)) * a, sqrt(kappa * n_th_a) * a.dag(), sqrt(gamma) * sm, sqrt(Gamma) * sm.dag()]\n"
        "\n"
        "rho_ss = steadystate(H, c_ops)"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 21);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "fig, axes = plt.subplots(1, 2, figsize=(16,6))\n"
        "\n"
        "xvec = np.linspace(-10,10,200)\n"
        "\n"
        "rho_cavity = ptrace(rho_ss, 0)\n"
        "W = wigner(rho_cavity, xvec, xvec)\n"
        "wlim = abs(W).max()\n"
        "axes[1].contourf(xvec, xvec, W, 100, norm=mpl.colors.Normalize(-wlim,wlim), cmap=plt.get_cmap('RdBu'))\n"
        "axes[1].set_xlabel(r'Im $\\alpha$', fontsize=18)\n"
        "axes[1].set_ylabel(r'Re $\\alpha$', fontsize=18)\n"
        "\n"
        "axes[0].bar(arange(0, N), real(rho_cavity.diag()), color=\"blue\", alpha=0.6)\n"
        "axes[0].set_xlabel(r'$n$', fontsize=18)\n"
        "axes[0].set_ylabel(r'Occupation probability', fontsize=18)\n"
        "axes[0].set_ylim(0, 1)\n"
        "axes[0].set_xlim(0, N);"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 22);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "### Case 2: $\\Gamma\\kappa/(4g^2) = 1.5$"
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "Gamma = 1.5 * (4*g**2) / kappa"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 23);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "c_ops = [sqrt(kappa * (1 + n_th_a)) * a, sqrt(kappa * n_th_a) * a.dag(), sqrt(gamma) * sm, sqrt(Gamma) * sm.dag()]\n"
        "\n"
        "rho_ss = steadystate(H, c_ops)"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 24);
    QCOMPARE(expression(entry)->results().size(), 0);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "fig, axes = plt.subplots(1, 2, figsize=(16,6))\n"
        "\n"
        "xvec = np.linspace(-10,10,200)\n"
        "\n"
        "rho_cavity = ptrace(rho_ss, 0)\n"
        "W = wigner(rho_cavity, xvec, xvec)\n"
        "wlim = abs(W).max()\n"
        "axes[1].contourf(xvec, xvec, W, 100, norm=mpl.colors.Normalize(-wlim,wlim), cmap=plt.get_cmap('RdBu'))\n"
        "axes[1].set_xlabel(r'Im $\\alpha$', fontsize=18)\n"
        "axes[1].set_ylabel(r'Re $\\alpha$', fontsize=18)\n"
        "\n"
        "axes[0].bar(arange(0, N), real(rho_cavity.diag()), color=\"blue\", alpha=0.6)\n"
        "axes[0].set_xlabel(r'$n$', fontsize=18)\n"
        "axes[0].set_ylabel(r'Occupation probability', fontsize=18)\n"
        "axes[0].set_ylim(0, 1)\n"
        "axes[0].set_xlim(0, N);"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 26);
    QCOMPARE(expression(entry)->results().size(), 1);
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::ImageResult::Type);
    QVERIFY(expression(entry)->result()->data().value<QImage>().isNull() == false);

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "Too large pumping rate $\\Gamma$ kills the lasing process: reversed threshold."
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "### Software version"
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)CommandEntry::Type);
    QCOMPARE(plainCommand(entry), QLatin1String(
        "from qutip.ipynbtools import version_table\n"
        "\n"
        "version_table()"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->id(), 27);
    QCOMPARE(expression(entry)->results().size(), 1);
    testHtmlResult(entry, 0, QString::fromUtf8(
        "<IPython.core.display.HTML at 0x7f2f2d5a0048>"
    ), QString::fromUtf8(
        "<table><tr><th>Software</th><th>Version</th></tr><tr><td>IPython</td><td>2.0.0</td></tr><tr><td>OS</td><td>posix [linux]</td></tr><tr><td>Python</td><td>3.4.1 (default, Jun  9 2014, 17:34:49) \n"
        "[GCC 4.8.3]</td></tr><tr><td>QuTiP</td><td>3.0.0.dev-5a88aa8</td></tr><tr><td>Numpy</td><td>1.8.1</td></tr><tr><td>matplotlib</td><td>1.3.1</td></tr><tr><td>Cython</td><td>0.20.1post0</td></tr><tr><td>SciPy</td><td>0.13.3</td></tr><tr><td colspan='2'>Thu Jun 26 14:28:35 2014 JST</td></tr></table>"
    ));

    QCOMPARE(entry->next(), nullptr);
}

void WorksheetTest::testJupyter2()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("AEC.04 - Evolutionary Strategies and Covariance Matrix Adaptation.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testMarkdown(entry, QLatin1String(
        "<div align='left' style=\"width:400px;height:120px;overflow:hidden;\">\n"
        "<a href='http://www.uff.br'>\n"
        "<img align='left' style='display: block;height: 92%' src='https://github.com/lmarti/jupyter_custom/raw/master/imgs/uff.png' alt='UFF logo' title='UFF logo'/>\n"
        "</a>\n"
        "<a href='http://www.ic.uff.br'>\n"
        "<img align='left' style='display: block;height: 100%' src='https://github.com/lmarti/jupyter_custom/raw/master/imgs/logo-ic.png' alt='IC logo' title='IC logo'/>\n"
        "</a>\n"
        "</div>"
    ));

    testMarkdown(entry, QString::fromLocal8Bit(
        "# Understanding evolutionary strategies and covariance matrix adaptation\n"
        "\n"
        "## Luis Martí, [IC](http://www.ic.uff.br)/[UFF](http://www.uff.br)\n"
        "\n"
        "[http://lmarti.com](http://lmarti.com); [lmarti@ic.uff.br](mailto:lmarti@ic.uff.br) \n"
        "\n"
        "[Advanced Evolutionary Computation: Theory and Practice](http://lmarti.com/aec-2014) "
    ));

    testMarkdown(entry, QString::fromLocal8Bit(
        "The notebook is better viewed rendered as slides. You can convert it to slides and view them by:\n"
        "- using [nbconvert](http://ipython.org/ipython-doc/1/interactive/nbconvert.html) with a command like:\n"
        "  ```bash\n"
        "  $ ipython nbconvert --to slides --post serve <this-notebook-name.ipynb>\n"
        "  ```\n"
        "- installing [Reveal.js - Jupyter/IPython Slideshow Extension](https://github.com/damianavila/live_reveal)\n"
        "- using the online [IPython notebook slide viewer](https://slideviewer.herokuapp.com/) (some slides of the notebook might not be properly rendered).\n"
        "\n"
        "This and other related IPython notebooks can be found at the course github repository:\n"
        "* [https://github.com/lmarti/evolutionary-computation-course](https://github.com/lmarti/evolutionary-computation-course)"
    ));

    testCommandEntry(entry, 1, QLatin1String(
        "import numpy as np\n"
        "import matplotlib.pyplot as plt\n"
        "import matplotlib.colors as colors\n"
        "from matplotlib import cm \n"
        "from mpl_toolkits.mplot3d import axes3d\n"
        "from scipy.stats import norm, multivariate_normal\n"
        "import math\n"
        "\n"
        "%matplotlib inline\n"
        "%config InlineBackend.figure_format = 'retina'\n"
        "plt.rc('text', usetex=True)\n"
        "plt.rc('font', family='serif')\n"
        "plt.rcParams['text.latex.preamble'] ='\\\\usepackage{libertine}\\n\\\\usepackage[utf8]{inputenc}'\n"
        "\n"
        "import seaborn\n"
        "seaborn.set(style='whitegrid')\n"
        "seaborn.set_context('notebook')"
    ));

    testMarkdown(entry, QString::fromLocal8Bit(
        "### Statistics recap\n"
        "\n"
        "* [Random variable](http://en.wikipedia.org/wiki/Random_variable): a variable whose value is subject to variations due to __chance__. A random variable can take on a set of possible different values, each with an associated probability, in contrast to other mathematical variables.\n"
        "\n"
        "* [Probability distribution](http://en.wikipedia.org/wiki/Probability_distribution): mathematical function describing the possible values of a random variable and their associated probabilities.\n"
        "\n"
        "* [Probability density function (pdf)](http://en.wikipedia.org/wiki/Probability_density_function) of a __continuous random variable__ is a function that describes the relative likelihood for this random variable to take on a given value. \n"
        "     * The probability of the random variable falling within a particular range of values is given by the integral of this variable’s density over that range.\n"
        "     * The probability density function is nonnegative everywhere, and its integral over the entire space is equal to one.\n"
        "     \n"
        "<img src='http://upload.wikimedia.org/wikipedia/commons/2/25/The_Normal_Distribution.svg' width='50%' align='center'/>\n"
        " "
    ));

    testMarkdown(entry, QLatin1String(
        "### [Moments](http://en.wikipedia.org/wiki/Moment_(mathematics)\n"
        "\n"
        "The probability distribution of a random variable is often characterised by a small number of parameters, which also have a practical interpretation.\n"
        "\n"
        "* [Mean](http://en.wikipedia.org/wiki/Mean) (a.k.a expected value) refers to one measure of the central tendency either of a probability distribution or of the random variable characterized by that distribution.\n"
        "    * population mean: $\\mu = \\operatorname{E}[X]$.\n"
        "    * estimation of sample mean: $\\bar{x}$.\n"
        "* [Standard deviation](http://en.wikipedia.org/wiki/Standard_deviation) measures the amount of variation or dispersion from the mean.\n"
        "    * population deviation:\n"
        "    $$\n"
        "\\sigma = \\sqrt{\\operatorname E[X^2]-(\\operatorname E[X])^2} = \\sqrt{\\frac{1}{N} \\sum_{i=1}^N (x_i - \\mu)^2}.\n"
        "$$\n"
        "    * unbiased estimator:\n"
        "    $$ \n"
        "    s^2 = \\frac{1}{N-1} \\sum_{i=1}^N (x_i - \\overline{x})^2.\n"
        "    $$"
    ));

    testMarkdown(entry, QLatin1String("### Two samples"));

    testCommandEntry(entry, 2, QLatin1String(
        "sample1 = np.random.normal(0, 0.5, 1000)\n"
        "sample2 = np.random.normal(1,1,500)"
    ));

    testCommandEntry(entry, 3, QLatin1String(
        "def plot_normal_sample(sample, mu, sigma):\n"
        "    'Plots an histogram and the normal distribution corresponding to the parameters.'\n"
        "    x = np.linspace(mu - 4*sigma, mu + 4*sigma, 100)\n"
        "    plt.plot(x, norm.pdf(x, mu, sigma), 'b', lw=2)\n"
        "    plt.hist(sample, 30, normed=True, alpha=0.2)\n"
        "    plt.annotate('3$\\sigma$', \n"
        "                     xy=(mu + 3*sigma, 0),  xycoords='data',\n"
        "                     xytext=(0, 100), textcoords='offset points',\n"
        "                     fontsize=15,\n"
        "                     arrowprops=dict(arrowstyle=\"->\",\n"
        "                                    connectionstyle=\"arc,angleA=180,armA=20,angleB=90,armB=15,rad=7\"))\n"
        "    plt.annotate('-3$\\sigma$', \n"
        "                     xy=(mu -3*sigma, 0), xycoords='data', \n"
        "                     xytext=(0, 100), textcoords='offset points',\n"
        "                     fontsize=15,\n"
        "                     arrowprops=dict(arrowstyle=\"->\",\n"
        "                                     connectionstyle=\"arc,angleA=180,armA=20,angleB=90,armB=15,rad=7\"))"
    ));

    testCommandEntry(entry, 4, 2, QLatin1String(
        "plt.figure(figsize=(11,4))\n"
        "plt.subplot(121)\n"
        "plot_normal_sample(sample1, 0, 0.5)\n"
        "plt.title('Sample 1: $\\mu=0$, $\\sigma=0.5$')\n"
        "plt.subplot(122)\n"
        "plot_normal_sample(sample2, 1, 1)\n"
        "plt.title('Sample 2: $\\mu=1$, $\\sigma=1$')\n"
        "plt.tight_layout();"
    ));
    testTextResult(entry, 0, QLatin1String(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/axes/_axes.py:6462: UserWarning: The 'normed' kwarg is deprecated, and has been replaced by the 'density' kwarg.\n"
        "  warnings.warn(\"The 'normed' kwarg is deprecated, and has been \""
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    testCommandEntry(entry, 5, 1, QLatin1String(
        "print('Sample 1; estimated mean:', sample1.mean(), ' and std. dev.: ', sample1.std())\n"
        "print('Sample 2; estimated mean:', sample2.mean(), ' and std. dev.: ', sample2.std())"
    ));
    testTextResult(entry, 0, QLatin1String(
        "Sample 1; estimated mean: 0.007446590585087637  and std. dev.:  0.5083158965764596\n"
        "Sample 2; estimated mean: 0.969635147915706  and std. dev.:  1.0213164282805647"
    ));
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "[Covariance](http://en.wikipedia.org/wiki/Covariance) is a measure of how much two random variables change together. \n"
        "$$\n"
        "\\operatorname{cov}(X,Y) = \\operatorname{E}{\\big[(X - \\operatorname{E}[X])(Y - \\operatorname{E}[Y])\\big]},\n"
        "$$\n"
        "$$\n"
        "\\operatorname{cov}(X,X) = s(X),\n"
        "$$\n"
        "\n"
        "* The sign of the covariance therefore shows the tendency in the linear relationship between the variables. \n"
        "* The magnitude of the covariance is not easy to interpret. \n"
        "* The normalized version of the covariance, the correlation coefficient, however, shows by its magnitude the strength of the linear relation."
    ));

    testMarkdown(entry, QLatin1String("### Understanding covariance"));

    testCommandEntry(entry, 6, QLatin1String(
        "sample_2d = np.array(list(zip(sample1, np.ones(len(sample1))))).T"
    ));

    testCommandEntry(entry, 7, 1, QLatin1String(
        "plt.scatter(sample_2d[0,:], sample_2d[1,:], marker='x');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testCommandEntry(entry, 8, 1, QLatin1String(
        "np.cov(sample_2d) # computes covariance between the two components of the sample"
    ));
    testTextResult(entry, 0, QLatin1String(
        "array([[0.25864369, 0.        ],\n"
        "       [0.        , 0.        ]])"
    ));
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "As the sample is only distributed along one axis, the covariance does not detects any relationship between them."
    ));

    testMarkdown(entry, QLatin1String(
        "What happens when we rotate the sample?"
    ));

    testCommandEntry(entry, 9, QLatin1String(
        "def rotate_sample(sample, angle=-45):\n"
        "    'Rotates a sample by `angle` degrees.'\n"
        "    theta = (angle/180.) * np.pi\n"
        "    rot_matrix = np.array([[np.cos(theta), -np.sin(theta)], \n"
        "                           [np.sin(theta), np.cos(theta)]])\n"
        "    return sample.T.dot(rot_matrix).T"
    ));

    testCommandEntry(entry, 10, QLatin1String(
        "rot_sample_2d = rotate_sample(sample_2d)"
    ));

    testCommandEntry(entry, 11, 1, QLatin1String(
        "plt.scatter(rot_sample_2d[0,:], rot_sample_2d[1,:], marker='x');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testCommandEntry(entry, 12, 1, QLatin1String(
        "np.cov(rot_sample_2d)"
    ));
    testTextResult(entry, 0, QLatin1String(
        "array([[0.12932185, 0.12932185],\n"
        "       [0.12932185, 0.12932185]])"
    ));
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "### A two-dimensional normally-distributed variable"
    ));

    testCommandEntry(entry, 13, 2, QLatin1String(
        "mu = [0,1]\n"
        "cov = [[1,0],[0,0.2]] # diagonal covariance, points lie on x or y-axis\n"
        "sample = np.random.multivariate_normal(mu,cov,1000).T\n"
        "plt.scatter(sample[0], sample[1], marker='x', alpha=0.29)\n"
        "\n"
        "estimated_mean = sample.mean(axis=1)\n"
        "estimated_cov = np.cov(sample)\n"
        "e_x,e_y = np.random.multivariate_normal(estimated_mean,estimated_cov,500).T\n"
        "\n"
        "plt.plot(e_x,e_y,'rx', alpha=0.47)\n"
        "x, y = np.mgrid[-4:4:.01, -1:3:.01]\n"
        "pos = np.empty(x.shape + (2,))\n"
        "pos[:, :, 0] = x; pos[:, :, 1] = y\n"
        "rv = multivariate_normal(estimated_mean, estimated_cov)\n"
        "plt.contour(x, y, rv.pdf(pos), cmap=cm.viridis_r, lw=4)\n"
        "plt.axis('equal');"
    ));
    testTextResult(entry, 0, QLatin1String(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'lw'\n"
        "  s)"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "### This is better understood in 3D"
    ));

    testCommandEntry(entry, 14, 1, QLatin1String(
        "fig = plt.figure(figsize=(11,5))\n"
        "ax = fig.gca(projection='3d')\n"
        "ax.plot_surface(x, y, rv.pdf(pos), cmap=cm.viridis_r, rstride=30, cstride=10, linewidth=1, alpha=0.47)\n"
        "ax.plot_wireframe(x, y, rv.pdf(pos), linewidth=0.47, alpha=0.47)\n"
        "ax.scatter(e_x, e_y, 0.4, marker='.', alpha=0.47)\n"
        "ax.axis('tight');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "Again, what happens if we rotate the sample?"
    ));

    testCommandEntry(entry, 15, QLatin1String(
        "rot_sample = rotate_sample(sample)\n"
        "estimated_mean = rot_sample.mean(axis=1)\n"
        "estimated_cov = np.cov(rot_sample)\n"
        "e_x,e_y = np.random.multivariate_normal(estimated_mean,estimated_cov,500).T"
    ));

    testCommandEntry(entry, 16, 1, QLatin1String(
        "fig = plt.figure(figsize=(11,4))\n"
        "plt.subplot(121)\n"
        "plt.scatter(rot_sample[0,:], rot_sample[1,:], marker='x', alpha=0.7)\n"
        "plt.title('\"Original\" data')\n"
        "plt.axis('equal')\n"
        "plt.subplot(122)\n"
        "plt.scatter(e_x, e_y, marker='o', color='g', alpha=0.7)\n"
        "plt.title('Sampled data')\n"
        "plt.axis('equal');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "Covariance captures the dependency and can model disposition of the \"original\" sample."
    ));

    testCommandEntry(entry, 17, QLatin1String(
        "x, y = np.mgrid[-4:4:.01, -3:3:.01]\n"
        "pos = np.empty(x.shape + (2,))\n"
        "pos[:, :, 0] = x; pos[:, :, 1] = y\n"
        "rv = multivariate_normal(estimated_mean, estimated_cov)"
    ));

    testCommandEntry(entry, 18, 1, QLatin1String(
        "fig = plt.figure(figsize=(11,5))\n"
        "ax = fig.gca(projection='3d')\n"
        "ax.plot_surface(x, y, rv.pdf(pos), cmap=cm.viridis_r, rstride=30, cstride=10, linewidth=1, alpha=0.47)\n"
        "ax.plot_wireframe(x, y, rv.pdf(pos), linewidth=0.47, alpha=0.47)\n"
        "ax.scatter(e_x, e_y, 0.4, marker='.', alpha=0.47)\n"
        "ax.axis('tight');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "# Evolutionary Strategies\n"
        "\n"
        "We will be using DEAP again to present some of the ES main concepts."
    ));

    testCommandEntry(entry, 19, QLatin1String(
        "import array, random, time, copy\n"
        "\n"
        "from deap import base, creator, benchmarks, tools, algorithms\n"
        "\n"
        "random.seed(42) # Fixing a random seed: You should not do this in practice."
    ));

    testMarkdown(entry, QLatin1String(
        "Before we dive into the discussion lets code some support functions."
    ));

    testCommandEntry(entry, 20, QLatin1String(
        "def plot_problem_3d(problem, bounds, resolution=100., \n"
        "                    cmap=cm.viridis_r, rstride=10, cstride=10, \n"
        "                    linewidth=0.15, alpha=0.65, ax=None):\n"
        "    'Plots a given deap benchmark problem in 3D mesh.'\n"
        "    (minx,miny),(maxx,maxy) = bounds\n"
        "    x_range = np.arange(minx, maxx, (maxx-minx)/resolution)\n"
        "    y_range = np.arange(miny, maxy, (maxy-miny)/resolution)\n"
        "    \n"
        "    X, Y = np.meshgrid(x_range, y_range)\n"
        "    Z = np.zeros((len(x_range), len(y_range)))\n"
        "    \n"
        "    for i in range(len(x_range)):\n"
        "        for j in range(len(y_range)):\n"
        "            Z[i,j] = problem((x_range[i], y_range[j]))[0]\n"
        "    \n"
        "    if not ax:\n"
        "        fig = plt.figure(figsize=(11,6))\n"
        "        ax = fig.gca(projection='3d')\n"
        "        \n"
        "    cset = ax.plot_surface(X, Y, Z, cmap=cmap, rstride=rstride, cstride=cstride, linewidth=linewidth, alpha=alpha)"
    ));

    testCommandEntry(entry, 21, QLatin1String(
        "def plot_problem_controur(problem, bounds, optimum=None,\n"
        "                          resolution=100., cmap=cm.viridis_r, \n"
        "                          rstride=1, cstride=10, linewidth=0.15,\n"
        "                          alpha=0.65, ax=None):\n"
        "    'Plots a given deap benchmark problem as a countour plot'\n"
        "    (minx,miny),(maxx,maxy) = bounds\n"
        "    x_range = np.arange(minx, maxx, (maxx-minx)/resolution)\n"
        "    y_range = np.arange(miny, maxy, (maxy-miny)/resolution)\n"
        "    \n"
        "    X, Y = np.meshgrid(x_range, y_range)\n"
        "    Z = np.zeros((len(x_range), len(y_range)))\n"
        "    \n"
        "    for i in range(len(x_range)):\n"
        "        for j in range(len(y_range)):\n"
        "            Z[i,j] = problem((x_range[i], y_range[j]))[0]\n"
        "    \n"
        "    if not ax:\n"
        "        fig = plt.figure(figsize=(6,6))\n"
        "        ax = fig.gca()\n"
        "        ax.set_aspect('equal')\n"
        "        ax.autoscale(tight=True)\n"
        "    \n"
        "    cset = ax.contourf(X, Y, Z, cmap=cmap, rstride=rstride, cstride=cstride, linewidth=linewidth, alpha=alpha)\n"
        "    \n"
        "    if optimum:\n"
        "        ax.plot(optimum[0], optimum[1], 'bx', linewidth=4, markersize=15)"
    ));

    testCommandEntry(entry, 22, QLatin1String(
        "def plot_cov_ellipse(pos, cov, volume=.99, ax=None, fc='lightblue', ec='darkblue', alpha=1, lw=1):\n"
        "    ''' Plots an ellipse that corresponds to a bivariate normal distribution.\n"
        "    Adapted from http://www.nhsilbert.net/source/2014/06/bivariate-normal-ellipse-plotting-in-python/'''\n"
        "    from scipy.stats import chi2\n"
        "    from matplotlib.patches import Ellipse\n"
        "\n"
        "    def eigsorted(cov):\n"
        "        vals, vecs = np.linalg.eigh(cov)\n"
        "        order = vals.argsort()[::-1]\n"
        "        return vals[order], vecs[:,order]\n"
        "\n"
        "    if ax is None:\n"
        "        ax = plt.gca()\n"
        "\n"
        "    vals, vecs = eigsorted(cov)\n"
        "    theta = np.degrees(np.arctan2(*vecs[:,0][::-1]))\n"
        "\n"
        "    kwrg = {'facecolor':fc, 'edgecolor':ec, 'alpha':alpha, 'linewidth':lw}\n"
        "\n"
        "    # Width and height are \"full\" widths, not radius\n"
        "    width, height = 2 * np.sqrt(chi2.ppf(volume,2)) * np.sqrt(vals)\n"
        "    ellip = Ellipse(xy=pos, width=width, height=height, angle=theta, **kwrg)\n"
        "    ax.add_artist(ellip)"
    ));

    testMarkdown(entry, QLatin1String(
        "### Why benchmarks (test) functions?\n"
        "\n"
        "In applied mathematics, [test functions](http://en.wikipedia.org/wiki/Test_functions_for_optimization), also known as artificial landscapes, are useful to evaluate characteristics of optimization algorithms, such as:\n"
        "\n"
        "* Velocity of convergence.\n"
        "* Precision.\n"
        "* Robustness.\n"
        "* General performance.\n"
        "\n"
        "DEAP has a number of test problems already implemented. See http://deap.readthedocs.org/en/latest/api/benchmarks.html"
    ));

    testMarkdown(entry, QLatin1String(
        "### [Bohachevsky benchmark problem](http://deap.readthedocs.org/en/latest/api/benchmarks.html#deap.benchmarks.bohachevsky)\n"
        "\n"
        "$$\\text{minimize } f(\\mathbf{x}) = \\sum_{i=1}^{N-1}(x_i^2 + 2x_{i+1}^2 - 0.3\\cos(3\\pi x_i) - 0.4\\cos(4\\pi x_{i+1}) + 0.7), \\mathbf{x}\\in \\left[-100,100\\right]^n,$$\n"
        "\n"
        "> Optimum in $\\mathbf{x}=\\mathbf{0}$, $f(\\mathbf{x})=0$."
    ));

    testCommandEntry(entry, 23, QLatin1String(
        "current_problem = benchmarks.bohachevsky"
    ));

    testCommandEntry(entry, 24, 1, QLatin1String(
        "plot_problem_3d(current_problem, ((-10,-10), (10,10)))"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "The Bohachevsky problem has many local optima."
    ));

    testCommandEntry(entry, 25, 1, QLatin1String(
        "plot_problem_3d(current_problem, ((-2.5,-2.5), (2.5,2.5)))"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testCommandEntry(entry, 26, 2, QLatin1String(
        "ax = plt.figure().gca()\n"
        "plot_problem_controur(current_problem, ((-2.5,-2.5), (2.5,2.5)), optimum=(0,0), ax=ax)\n"
        "ax.set_aspect('equal')"
    ));
    testTextResult(entry, 0, QLatin1String(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    testMarkdown(entry, QLatin1String(
        "## ($\\mu$,$\\lambda$) evolutionary strategy\n"
        "\n"
        "Some basic initialization parameters."
    ));

    testCommandEntry(entry, 27, QLatin1String(
        "search_space_dims = 2 # we want to plot the individuals so this must be 2\n"
        "\n"
        "MIN_VALUE, MAX_VALUE = -10., 10.\n"
        "MIN_STRAT, MAX_STRAT = 0.0000001, 1. "
    ));

    testCommandEntry(entry, 28, QLatin1String(
        "# We are facing a minimization problem\n"
        "creator.create(\"FitnessMin\", base.Fitness, weights=(-1.0,))\n"
        "\n"
        "# Evolutionary strategies need a location (mean)\n"
        "creator.create(\"Individual\", array.array, typecode='d', \n"
        "               fitness=creator.FitnessMin, strategy=None)\n"
        "# ...and a value of the strategy parameter.\n"
        "creator.create(\"Strategy\", array.array, typecode=\"d\")"
    ));

    testMarkdown(entry, QLatin1String(
        "Evolutionary strategy individuals are more complex than those we have seen so far.\n"
        "\n"
        "They need a custom creation/initialization function."
    ));

    testCommandEntry(entry, 29, QLatin1String(
        "def init_univariate_es_ind(individual_class, strategy_class,\n"
        "                           size, min_value, max_value, \n"
        "                           min_strat, max_strat):\n"
        "    ind = individual_class(random.uniform(min_value, max_value) \n"
        "                           for _ in range(size))\n"
        "    # we modify the instance to include the strategy in run-time.\n"
        "    ind.strategy = strategy_class(random.uniform(min_strat, max_strat) for _ in range(size))\n"
        "    return ind"
    ));

    testCommandEntry(entry, 30, QLatin1String(
        "toolbox = base.Toolbox() \n"
        "toolbox.register(\"individual\", init_univariate_es_ind, \n"
        "                 creator.Individual, \n"
        "                 creator.Strategy,\n"
        "                 search_space_dims, \n"
        "                 MIN_VALUE, MAX_VALUE, \n"
        "                 MIN_STRAT, MAX_STRAT)\n"
        "toolbox.register(\"population\", tools.initRepeat, list, \n"
        "                 toolbox.individual)"
    ));

    testMarkdown(entry, QLatin1String(
        "How does an individual and a population looks like?"
    ));

    testCommandEntry(entry, 31, QLatin1String(
        "ind = toolbox.individual()\n"
        "pop = toolbox.population(n=3)"
    ));

    testCommandEntry(entry, 32, QLatin1String(
        "def plot_individual(individual, ax=None):\n"
        "    'Plots an ES indiviual as center and 3*sigma ellipsis.'\n"
        "    cov = np.eye(len(individual)) * individual.strategy\n"
        "    plot_cov_ellipse(individual, cov, volume=0.99, alpha=0.56, ax=ax)\n"
        "    if ax:\n"
        "        ax.scatter(individual[0], individual[1], \n"
        "                    marker='+', color='k', zorder=100)\n"
        "    else:\n"
        "        plt.scatter(individual[0], individual[1], \n"
        "                    marker='+', color='k', zorder=100)\n"
        "\n"
        "    \n"
        "def plot_population(pop, gen=None, max_gen=None, ax=None):\n"
        "    if gen:\n"
        "        plt.subplot(max_gen, 1, gen)\n"
        "        \n"
        "    for ind in pop:\n"
        "        plot_individual(ind, ax)"
    ));

    qDebug() << "command entry 33";
    testCommandEntry(entry, 33, 2, QString::fromUtf8(
        "plot_problem_controur(current_problem, ((-10,-10), (10,10)), optimum=(0,0))\n"
        "plot_individual(ind)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    qDebug() << "command entry 34";
    testCommandEntry(entry, 34, 2, QString::fromUtf8(
        "plot_problem_controur(current_problem, ((-10,-10), (10,10)), optimum=(0,0))\n"
        "plot_population(pop)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "### Mutation of an evolution strategy individual according to its strategy attribute. \n"
        "First the strategy is mutated according to an extended log normal rule, \n"
        "$$\n"
        "\\boldsymbol{\\sigma}_t = \\exp(\\tau_0 \\mathcal{N}_0(0, 1)) \\left[ \\sigma_{t-1, 1}\\exp(\\tau\n"
        "\\mathcal{N}_1(0, 1)), \\ldots, \\sigma_{t-1, n} \\exp(\\tau\n"
        "\\mathcal{N}_n(0, 1))\\right],\n"
        "$$\n"
        "with \n"
        "$$\\tau_0 =\n"
        "\\frac{c}{\\sqrt{2n}}\\text{ and }\\tau = \\frac{c}{\\sqrt{2\\sqrt{n}}},\n"
        "$$\n"
        "\n"
        "the individual is mutated by a normal distribution of mean 0 and standard deviation of $\\boldsymbol{\\sigma}_{t}$ (its current strategy). \n"
        "\n"
        "A recommended choice is $c=1$ when using a $(10,100)$ evolution strategy."
    ));

    qDebug() << "command entry 35";
    testCommandEntry(entry, 35, QString::fromUtf8(
        "toolbox.register(\"mutate\", tools.mutESLogNormal, c=1, indpb=0.1)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Blend crossover on both, the individual and the strategy."
    ));

    qDebug() << "command entry 36";
    testCommandEntry(entry, 36, QString::fromUtf8(
        "toolbox.register(\"mate\", tools.cxESBlend, alpha=0.1)\n"
        "toolbox.register(\"evaluate\", current_problem)\n"
        "toolbox.register(\"select\", tools.selBest)"
    ));

    qDebug() << "command entry 37";
    testCommandEntry(entry, 37, QString::fromUtf8(
        "mu_es, lambda_es = 3,21\n"
        "\n"
        "pop = toolbox.population(n=mu_es)\n"
        "hof = tools.HallOfFame(1)\n"
        "\n"
        "pop_stats = tools.Statistics(key=copy.deepcopy)\n"
        "pop_stats.register('pop', copy.deepcopy) # -- copies the populations themselves\n"
        "    \n"
        "pop, logbook = algorithms.eaMuCommaLambda(pop, toolbox, mu=mu_es, lambda_=lambda_es, \n"
        "        cxpb=0.6, mutpb=0.3, ngen=40, stats=pop_stats, halloffame=hof, verbose=False)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### The final population"
    ));

    qDebug() << "command entry 38";
    testCommandEntry(entry, 38, 2, QString::fromUtf8(
        "plot_problem_controur(current_problem, ((-10,-10), (10,10)), optimum=(0,0))\n"
        "plot_population(pop)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "The plot (most probably) shows a \"dark blue\" ellipse as all individuals are overlapping. "
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Let's see how the evolutionary process took place in animated form."
    ));

    qDebug() << "command entry 39";
    testCommandEntry(entry, 39, QString::fromUtf8(
        "from matplotlib import animation\n"
        "from IPython.display import HTML"
    ));

    qDebug() << "command entry 40";
    testCommandEntry(entry, 40, QString::fromUtf8(
        "def animate(i):\n"
        "    'Updates all plots to match frame _i_ of the animation.'\n"
        "    ax.clear()\n"
        "    plot_problem_controur(current_problem, ((-10.1,-10.1), (10.1,10.1)), optimum=(0,0), ax=ax)\n"
        "    plot_population(logbook[i]['pop'], ax=ax)\n"
        "    ax.set_title('$t=$' +str(i))\n"
        "    return []"
    ));

    qDebug() << "command entry 41";
    testCommandEntry(entry, 41, 1, QString::fromUtf8(
        "fig = plt.figure(figsize=(5,5))\n"
        "ax = fig.gca()\n"
        "anim = animation.FuncAnimation(fig, animate, frames=len(logbook), interval=300, blit=True)\n"
        "plt.close()"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    entry = entry->next();

    qDebug() << "command entry 42";
    testCommandEntry(entry, 42, 2, QString::fromUtf8(
        "HTML(anim.to_html5_video())"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)\n"
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    testHtmlResult(entry, 1, QString::fromUtf8(
        "<IPython.core.display.HTML object>"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "How the population progressed as the evolution proceeded?"
    ));

    qDebug() << "command entry 43";
    testCommandEntry(entry, 43, QString::fromUtf8(
        "pop = toolbox.population(n=mu_es)\n"
        "\n"
        "stats = tools.Statistics(lambda ind: ind.fitness.values)\n"
        "stats.register(\"avg\", np.mean)\n"
        "stats.register(\"std\", np.std)\n"
        "stats.register(\"min\", np.min)\n"
        "stats.register(\"max\", np.max)\n"
        "    \n"
        "pop, logbook = algorithms.eaMuCommaLambda(pop, toolbox, \n"
        "                                          mu=mu_es, lambda_=lambda_es, \n"
        "                                          cxpb=0.6, mutpb=0.3, \n"
        "                                          ngen=40, stats=stats, \n"
        "                                          verbose=False)"
    ));

    qDebug() << "command entry 44";
    testCommandEntry(entry, 44, 1, QString::fromUtf8(
        "plt.figure(1, figsize=(7, 4))\n"
        "plt.plot(logbook.select('avg'), 'b-', label='Avg. fitness')\n"
        "plt.fill_between(range(len(logbook)), logbook.select('max'), logbook.select('min'), facecolor='blue', alpha=0.47)\n"
        "plt.plot(logbook.select('std'), 'm--', label='Std. deviation')\n"
        "plt.legend(frameon=True)\n"
        "plt.ylabel('Fitness'); plt.xlabel('Iterations');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "What happens if we increase $\\mu$ and $\\lambda$?"
    ));

    qDebug() << "command entry 45";
    testCommandEntry(entry, 45, 1, QString::fromUtf8(
        "mu_es, lambda_es = 10,100\n"
        "pop, logbook = algorithms.eaMuCommaLambda(toolbox.population(n=mu_es), toolbox, mu=mu_es, lambda_=lambda_es, \n"
        "        cxpb=0.6, mutpb=0.3, ngen=40, stats=stats, halloffame=hof, verbose=False)\n"
        "plt.figure(1, figsize=(7, 4))\n"
        "plt.plot(logbook.select('avg'), 'b-', label='Avg. fitness')\n"
        "plt.fill_between(range(len(logbook)), logbook.select('max'), logbook.select('min'), facecolor='blue', alpha=0.47)\n"
        "plt.plot(logbook.select('std'), 'm--', label='Std. deviation')\n"
        "plt.legend(frameon=True)\n"
        "plt.ylabel('Fitness'); plt.xlabel('Iterations');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "# Covariance Matrix Adaptation Evolutionary Strategy"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "* In an evolution strategy, new candidate solutions are sampled according to a multivariate normal distribution in the $\\mathbb{R}^n$. \n"
        "* Recombination amounts to selecting a new mean value for the distribution. \n"
        "* Mutation amounts to adding a random vector, a perturbation with zero mean. \n"
        "* Pairwise dependencies between the variables in the distribution are represented by a covariance matrix. \n"
        "\n"
        "### The covariance matrix adaptation (CMA) is a method to update the covariance matrix of this distribution. \n"
        "\n"
        "> This is particularly useful, if the objective function $f()$ is ill-conditioned."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### CMA-ES features\n"
        "\n"
        "* Adaptation of the covariance matrix amounts to learning a second order model of the underlying objective function.\n"
        "* This is similar to the approximation of the inverse Hessian matrix in the Quasi-Newton method in classical optimization. \n"
        "* In contrast to most classical methods, fewer assumptions on the nature of the underlying objective function are made. \n"
        "* *Only the ranking between candidate solutions is exploited* for learning the sample distribution and neither derivatives nor even the function values themselves are required by the method."
    ));

    qDebug() << "command entry 46";
    testCommandEntry(entry, 46, QString::fromUtf8(
        "from deap import cma"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "A similar setup to the previous one."
    ));

    qDebug() << "command entry 47";
    testCommandEntry(entry, 47, 1, QString::fromUtf8(
        "creator.create(\"Individual\", list, fitness=creator.FitnessMin)\n"
        "toolbox = base.Toolbox()\n"
        "toolbox.register(\"evaluate\", current_problem)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/home/mmmm1998/.local/lib/python3.6/site-packages/deap/creator.py:141: RuntimeWarning: A class named 'Individual' has already been created and it will be overwritten. Consider deleting previous creation of that class or rename it.\n"
        "  RuntimeWarning)"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "We will place our start point by hand at $(5,5)$."
    ));

    qDebug() << "command entry 48";
    testCommandEntry(entry, 48, 1, QString::fromUtf8(
        "cma_es = cma.Strategy(centroid=[5.0]*search_space_dims, sigma=5.0, lambda_=5*search_space_dims)\n"
        "toolbox.register(\"generate\", cma_es.generate, creator.Individual)\n"
        "toolbox.register(\"update\", cma_es.update)\n"
        "\n"
        "hof = tools.HallOfFame(1)\n"
        "stats = tools.Statistics(lambda ind: ind.fitness.values)\n"
        "stats.register(\"avg\", np.mean)\n"
        "stats.register(\"std\", np.std)\n"
        "stats.register(\"min\", np.min)\n"
        "stats.register(\"max\", np.max)\n"
        "\n"
        "# The CMA-ES algorithm converge with good probability with those settings\n"
        "pop, logbook = algorithms.eaGenerateUpdate(toolbox, ngen=60, stats=stats, \n"
        "                                           halloffame=hof, verbose=False)\n"
        "    \n"
        "print(\"Best individual is %s, fitness: %s\" % (hof[0], hof[0].fitness.values))"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Best individual is [-2.524016407520609e-08, -4.0857988576506457e-08], fitness: (6.517009154549669e-14,)"
    ));
    entry = entry->next();

    qDebug() << "command entry 49";
    testCommandEntry(entry, 49, 1, QString::fromUtf8(
        "plt.figure(1, figsize=(7, 4))\n"
        "plt.plot(logbook.select('avg'), 'b-', label='Avg. fitness')\n"
        "plt.fill_between(range(len(logbook)), logbook.select('max'), logbook.select('min'), facecolor='blue', alpha=0.47)\n"
        "plt.plot(logbook.select('std'), 'm--', label='Std. deviation')\n"
        "plt.legend(frameon=True)\n"
        "plt.ylabel('Fitness'); plt.xlabel('Iterations');"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "### OK, but wouldn't it be nice to have an animated plot of how CMA-ES progressed? \n"
        "\n"
        "* We need to do some coding to make this animation work.\n"
        "* We are going to create a class named `PlotableStrategy` that inherits from `deap.cma.Strategy`. This class logs the features we need to make the plots as evolution takes place. That is, for every iteration we store:\n"
        "    * Current centroid and covariance ellipsoid.\n"
        "    * Updated centroid and covariance.\n"
        "    * Sampled individuals.\n"
        "    * Evolution path.\n"
        "    \n"
        "_Note_: I think that DEAP's implementation of CMA-ES has the drawback of storing information that should be stored as part of \"individuals\". I leave this for an afternoon hack."
    ));

    qDebug() << "command entry 50";
    testCommandEntry(entry, 50, QString::fromUtf8(
        "from math import sqrt, log, exp\n"
        "class PlotableStrategy(cma.Strategy):\n"
        "    \"\"\"This is a modification of deap.cma.Strategy class.\n"
        "    We store the execution data in order to plot it.\n"
        "    **Note:** This class should not be used for other uses than\n"
        "    the one it is meant for.\"\"\"\n"
        "    \n"
        "    def __init__(self, centroid, sigma, **kargs):\n"
        "        \"\"\"Does the original initialization and then reserves \n"
        "        the space for the statistics.\"\"\"\n"
        "        super(PlotableStrategy, self).__init__(centroid, sigma, **kargs)\n"
        "        \n"
        "        self.stats_centroids = []\n"
        "        self.stats_new_centroids = []\n"
        "        self.stats_covs = []\n"
        "        self.stats_new_covs = []\n"
        "        self.stats_offspring = []\n"
        "        self.stats_offspring_weights = []\n"
        "        self.stats_ps = []\n"
        "    \n"
        "    def update(self, population):\n"
        "        \"\"\"Update the current covariance matrix strategy from the\n"
        "        *population*.\n"
        "        \n"
        "        :param population: A list of individuals from which to update the\n"
        "                           parameters.\n"
        "        \"\"\"\n"
        "        # -- store current state of the algorithm\n"
        "        self.stats_centroids.append(copy.deepcopy(self.centroid))\n"
        "        self.stats_covs.append(copy.deepcopy(self.C))\n"
        "        \n"
        "        \n"
        "        population.sort(key=lambda ind: ind.fitness, reverse=True)\n"
        "        \n"
        "        # -- store sorted offspring\n"
        "        self.stats_offspring.append(copy.deepcopy(population))\n"
        "        \n"
        "        old_centroid = self.centroid\n"
        "        self.centroid = np.dot(self.weights, population[0:self.mu])\n"
        "        \n"
        "        # -- store new centroid\n"
        "        self.stats_new_centroids.append(copy.deepcopy(self.centroid))\n"
        "        \n"
        "        c_diff = self.centroid - old_centroid\n"
        "        \n"
        "        \n"
        "        # Cumulation : update evolution path\n"
        "        self.ps = (1 - self.cs) * self.ps \\\n"
        "             + sqrt(self.cs * (2 - self.cs) * self.mueff) / self.sigma \\\n"
        "             * np.dot(self.B, (1. / self.diagD) \\\n"
        "                          * np.dot(self.B.T, c_diff))\n"
        "        \n"
        "        # -- store new evol path\n"
        "        self.stats_ps.append(copy.deepcopy(self.ps))\n"
        "        \n"
        "        hsig = float((np.linalg.norm(self.ps) / \n"
        "                sqrt(1. - (1. - self.cs)**(2. * (self.update_count + 1.))) / self.chiN\n"
        "                < (1.4 + 2. / (self.dim + 1.))))\n"
        "        \n"
        "        self.update_count += 1\n"
        "        \n"
        "        self.pc = (1 - self.cc) * self.pc + hsig \\\n"
        "                  * sqrt(self.cc * (2 - self.cc) * self.mueff) / self.sigma \\\n"
        "                  * c_diff\n"
        "        \n"
        "        # Update covariance matrix\n"
        "        artmp = population[0:self.mu] - old_centroid\n"
        "        self.C = (1 - self.ccov1 - self.ccovmu + (1 - hsig) \\\n"
        "                   * self.ccov1 * self.cc * (2 - self.cc)) * self.C \\\n"
        "                + self.ccov1 * np.outer(self.pc, self.pc) \\\n"
        "                + self.ccovmu * np.dot((self.weights * artmp.T), artmp) \\\n"
        "                / self.sigma**2\n"
        "        \n"
        "        # -- store new covs\n"
        "        self.stats_new_covs.append(copy.deepcopy(self.C))\n"
        "        \n"
        "        self.sigma *= np.exp((np.linalg.norm(self.ps) / self.chiN - 1.) \\\n"
        "                                * self.cs / self.damps)\n"
        "        \n"
        "        self.diagD, self.B = np.linalg.eigh(self.C)\n"
        "        indx = np.argsort(self.diagD)\n"
        "        \n"
        "        self.cond = self.diagD[indx[-1]]/self.diagD[indx[0]]\n"
        "        \n"
        "        self.diagD = self.diagD[indx]**0.5\n"
        "        self.B = self.B[:, indx]\n"
        "        self.BD = self.B * self.diagD"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "It is now possible to use/test our new class."
    ));

    qDebug() << "command entry 51";
    testCommandEntry(entry, 51, QString::fromUtf8(
        "toolbox = base.Toolbox()\n"
        "toolbox.register(\"evaluate\", current_problem)"
    ));

    qDebug() << "command entry 52";
    testCommandEntry(entry, 52, QString::fromUtf8(
        "max_gens = 40\n"
        "cma_es = PlotableStrategy(centroid=[5.0]*search_space_dims, sigma=1.0, lambda_=5*search_space_dims)\n"
        "toolbox.register(\"generate\", cma_es.generate, creator.Individual)\n"
        "toolbox.register(\"update\", cma_es.update)\n"
        "\n"
        "# The CMA-ES algorithm converge with good probability with those settings\n"
        "a = algorithms.eaGenerateUpdate(toolbox, ngen=max_gens, verbose=False)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Me can now code the `animate_cma_es()` function."
    ));

    qDebug() << "command entry 53";
    testCommandEntry(entry, 53, QString::fromUtf8(
        "norm=colors.Normalize(vmin=np.min(cma_es.weights), vmax=np.max(cma_es.weights))\n"
        "sm = cm.ScalarMappable(norm=norm, cmap=plt.get_cmap('gray'))"
    ));

    qDebug() << "command entry 54";
    testCommandEntry(entry, 54, QString::fromUtf8(
        "def animate_cma_es(gen):\n"
        "    ax.cla()\n"
        "    plot_problem_controur(current_problem, ((-11,-11), (11,11)), optimum=(0,0), ax=ax)\n"
        "    \n"
        "    plot_cov_ellipse(cma_es.stats_centroids[gen], cma_es.stats_covs[gen], volume=0.99, alpha=0.29, ax=ax)\n"
        "    ax.plot(cma_es.stats_centroids[gen][0], cma_es.stats_centroids[gen][1], 'ro', markeredgecolor = 'none', ms=10)\n"
        "    \n"
        "    plot_cov_ellipse(cma_es.stats_new_centroids[gen], cma_es.stats_new_covs[gen], volume=0.99, \n"
        "                     alpha=0.29, fc='green', ec='darkgreen', ax=ax)\n"
        "    ax.plot(cma_es.stats_new_centroids[gen][0], cma_es.stats_new_centroids[gen][1], 'go', markeredgecolor = 'none', ms=10)\n"
        "    \n"
        "    for i in range(gen+1):\n"
        "        if i == 0:\n"
        "            ax.plot((0,cma_es.stats_ps[i][0]),\n"
        "                     (0,cma_es.stats_ps[i][1]), 'b--')\n"
        "        else:\n"
        "            ax.plot((cma_es.stats_ps[i-1][0],cma_es.stats_ps[i][0]),\n"
        "                     (cma_es.stats_ps[i-1][1],cma_es.stats_ps[i][1]),'b--')\n"
        "            \n"
        "    for i,ind in enumerate(cma_es.stats_offspring[gen]):\n"
        "        if i < len(cma_es.weights):\n"
        "            color = sm.to_rgba(cma_es.weights[i])\n"
        "        else:\n"
        "            color= sm.to_rgba(norm.vmin)\n"
        "        ax.plot(ind[0], ind[1], 'o', color = color, ms=5, markeredgecolor = 'none')\n"
        "    \n"
        "    ax.set_ylim((-10,10))\n"
        "    ax.set_xlim((-10,10))\n"
        "    ax.set_title('$t=$' +str(gen))\n"
        "    return []"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### CMA-ES progress "
    ));

    qDebug() << "command entry 55";
    testCommandEntry(entry, 55, 1, QString::fromUtf8(
        "fig = plt.figure(figsize=(6,6))\n"
        "ax = fig.gca()\n"
        "anim = animation.FuncAnimation(fig, animate_cma_es, frames=max_gens, interval=300, blit=True)\n"
        "plt.close()"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    entry = entry->next();

    qDebug() << "command entry 56";
    testCommandEntry(entry, 56, 2, QString::fromUtf8(
        "HTML(anim.to_html5_video())"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "/usr/local/lib/python3.6/dist-packages/matplotlib/contour.py:960: UserWarning: The following kwargs were not used by contour: 'rstride', 'cstride', 'linewidth'\n"
        "  s)"
    ));
    testHtmlResult(entry, 1, QString::fromUtf8(
        "<IPython.core.display.HTML object>"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "* Current centroid and covariance: **red**.\n"
        "* Updated centroid and covariance: **green**. \n"
        "* Sampled individuals: **shades of gray representing their corresponding weight**.\n"
        "* Evolution path: **blue line starting in (0,0)**. "
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Homework\n"
        "\n"
        "1. Make an animated plot with the covariance update process. You can rely on the notebook of the previous demonstration class.\n"
        "2. Compare ES, CMA-ES and a genetic algortihm.\n"
        "2. How do you think that evolutionary strategies and CMA-ES should be modified in order to cope with combinatorial problems?\n"
        "3. How can evolution strategies be improved?\n"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "<hr/>\n"
        "<div class=\"container-fluid\">\n"
        "  <div class='well'>\n"
        "      <div class=\"row\">\n"
        "          <div class=\"col-md-3\" align='center'>\n"
        "              <img align='center'alt=\"Creative Commons License\" style=\"border-width:0\" src=\"https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png\"/>\n"
        "          </div>\n"
        "          <div class=\"col-md-9\">\n"
        "              This work is licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-nc-sa/4.0/).\n"
        "          </div>\n"
        "      </div>\n"
        "  </div>\n"
        "</div>"
    ));

    qDebug() << "command entry 58";
    testCommandEntry(entry, 58, 1, QString::fromUtf8(
        "# To install run: pip install version_information\n"
        "%load_ext version_information\n"
        "%version_information scipy, numpy, matplotlib, seaborn, deap"
    ));
    testHtmlResult(entry, 0, QString::fromLatin1(
        "Software versions\n"
        "Python 3.6.7 64bit [GCC 8.2.0]\n"
        "IPython 6.3.1\n"
        "OS Linux 4.15.0 50 generic x86_64 with Ubuntu 18.04 bionic\n"
        "scipy 1.1.0\n"
        "numpy 1.14.5\n"
        "matplotlib 2.2.2\n"
        "seaborn 0.9.0\n"
        "deap 1.2\n"
        "Wed May 29 19:31:25 2019 MSK"
    ), QString::fromLatin1(
        "<table><tr><th>Software</th><th>Version</th></tr><tr><td>Python</td><td>3.6.7 64bit [GCC 8.2.0]</td></tr><tr><td>IPython</td><td>6.3.1</td></tr><tr><td>OS</td><td>Linux 4.15.0 50 generic x86_64 with Ubuntu 18.04 bionic</td></tr><tr><td>scipy</td><td>1.1.0</td></tr><tr><td>numpy</td><td>1.14.5</td></tr><tr><td>matplotlib</td><td>2.2.2</td></tr><tr><td>seaborn</td><td>0.9.0</td></tr><tr><td>deap</td><td>1.2</td></tr><tr><td colspan='2'>Wed May 29 19:31:25 2019 MSK</td></tr></table>"
    ));
    entry = entry->next();

    qDebug() << "command entry 59";
    testCommandEntry(entry, 59, 1, QString::fromUtf8(
        "# this code is here for cosmetic reasons\n"
        "from IPython.core.display import HTML\n"
        "from urllib.request import urlopen\n"
        "HTML(urlopen('https://raw.githubusercontent.com/lmarti/jupyter_custom/master/custom.include').read().decode('utf-8'))"
    ));
    testHtmlResult(entry, 0, QString::fromUtf8(
        "<IPython.core.display.HTML object>"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        " "
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testJupyter3()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("Population_Genetics.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python2"));

    WorksheetEntry* entry = w->firstEntry();

    testMarkdown(entry, QString::fromUtf8(
        "## Population Genetics in *an* RNA World\n"
        "\n"
        "In order to study population genetics, we first need a model of a population. And even before that, we need to define what we mean by *population*. Populations can be defined on many levels and with many diffferent criteria. For our purposes, we will simply say that a population is a set of individuals sharing a common environment. And because this is population *genetics* we can think of individuals as entities comprising of specific genes or chromosomes. \n"
        "\n"
        "So where do we get a population from? As you may have discussed in previous workshops, there are very large datasets containing sequencing information from different populations. So we could download one of these datasets and perform some analysis on it. But I find this can be dry and tedious. So why download data when we can simply create our own?\n"
        "\n"
        "In this workshop we're going to be creating and studying our own \"artificial\" populations to illustrate some important population genetics concepts and methodologies. Not only will this help you learn population genetics, but you will get a lot more programming practice than if we were to simply parse data files and go from there. \n"
        "\n"
        "More specifically, we're going to build our own RNA world.\n"
        "\n"
        "As you may know, RNA is widely thought to be the first self replicating life-form to arise x billion years ago. One of the strongest arguments for this theory is that RNA is able to carry information in its nucleotides like DNA, and like protein, it is able to adopt higher order structures to catalyze reactions, such as self replication. So it is likely, and there is growing evidence that this is the case, that the first form of replicating life was RNA. And because of this dual property of RNA as an information vessel as well as a structural/functional element we can use RNA molecules to build very nice population models. \n"
        "\n"
        "So in this notebook, I'll be walking you through building genetic populations, simulating their evolution, and using statistics and other mathematical tools for understanding key properties of populations.\n"
        "\n"
        "### Building an RNA population\n"
        "\n"
        "As we saw earlier, RNA has the nice property of posessing a strong mapping between information carrying (sequence) and function (structure). This is analogous to what is known in evolutionary terms as a genotype and a phenotype. With these properties, we have everything we need to model a population, and simulate its evolution.\n"
        "\n"
        "#### RNA sequence-structure\n"
        "\n"
        "We can think of the genotype as a sequence $s$ consisting of letters/nucleotides from the alphabet $\\{U,A,C,G\\}$. The corresponding phenotype $\\omega$ is the secondary structure of $s$ which can be thought of as a pairing between nucleotides in the primary sequence that give rise to a 2D architecture. Because it has been shown that the function of many biomolecules, including RNA, is driven by structure this gives us a good proxy for phenotype. \n"
        "\n"
        "Below is an example of what an RNA secondary structure, or pairing, looks like."
    ));

    qDebug() << "command entry 1";
    testCommandEntry(entry, 1, 1, QString::fromUtf8(
        "### 1\n"
        "\n"
        "from IPython.display import Image\n"
        "#This will load an image of an RNA secondary structure\n"
        "Image(url='http://www.tbi.univie.ac.at/~pkerp/forgi/_images/1y26_ss.png')"
    ));
    testHtmlResult(entry, 0, QString::fromLatin1(
        "<IPython.core.display.Image object>"
    ), QString::fromLatin1(
        "<img src=\"http://www.tbi.univie.ac.at/~pkerp/forgi/_images/1y26_ss.png\"/>"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "As you can see, unparied positions are forming loop-like structures, and paired positions are forming stem-like structures. It is this spatial arrangement of nucleotides that drives RNA's function. Therefore, another sequence that adopts a similar shape, is likely to behave in a similar manner. Another thing to notice is that, although in reality this is often not the case, in general we only allow pairs between $\\{C,G\\}$ and $\\{A, U\\}$ nucleotides, most modern approaches allow for non-canonical pairings and you will find some examples of this in the above structure.\n"
        "\n"
        "*How do we go from a sequence to a structure?*\n"
        "\n"
        "So a secondary structure is just a list of pairings between positions. How do we get the optimal pairing?\n"
        "\n"
        "The algorithm we're going to be using in our simulations is known as the Nussinov Algorithm. The Nussinov algorithm is one of the first and simplest attempts at predicting RNA structure. Because bonds tend to stabilize RNA, the algorithm tries to maximize the number of pairs in the structure and return that as its solution. Current approaches achieve more accurate solutions by using energy models based one experimental values to then obtain a structure that minimizes free energy. But since we're not really concerned with the accuracy of our predictions, Nussinov is a good entry point. Furthermore, the main algorithmic concepts are the same between Nussinov and state of the art RNA structure prediction algorithms. I implemented the algorithm in a separate file called `fold.py` that we can import and use its functions. I'm not going to go into detail here on how the algorithm works because it is beyond the scope of this workshop but there is a bonus exercise at the end if you're curious.\n"
        "\n"
        "You can predict a secondary structure by calling `nussinov()` with a sequence string and it will return a tuple in the form `(structure, pairs)`."
    ));

    qDebug() << "command entry 2";
    testCommandEntry(entry, 2, 1, QString::fromUtf8(
        "import numpy as np\n"
        "from fold import nussinov\n"
        "\n"
        "sequence_to_fold = \"ACCCGAUGUUAUAUAUACCU\"\n"
        "struc = nussinov(sequence_to_fold)\n"
        "print(sequence_to_fold)\n"
        "print(struc[0])"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "ACCCGAUGUUAUAUAUACCU\n"
        "(...(..(((....).))))"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "You will see a funny dot-bracket string in the output. This is a representation of the structure of an RNA. Quite simply, a matching parir of parentheses (open and close) correspond to the nucleotides at those positions being paried. Whereas, a dot means that that position is unpaired in the structure. Feel free to play around with the input sequence to get a better understanding of the notation.\n"
        "\n"
        "So that's enough about RNA structure prediction. Let's move on to building our populations.\n"
        "\n"
        "### Fitness of a sequence: Target Structure\n"
        "\n"
        "Now that we have a good way of getting a phenotype (secondary structure), we need a way to evaluate the fitness of that phenotype. If we think in real life terms, fitness is the ability of a genotype to replicate into the next generation. If you have a gene carrying a mutation that causes some kind of disease, your fitness is decreased and you have a lower chance of contributing offspring to the next generation. On a molecular level the same concept applies. A molecule needs to accomplish a certain function, i.e. bind to some other molecule or send some kind of signal. And as we've seen before, the most important factor that determines how well it can carry out this function is its structure. So we can imagine that a certain structure, we can call this a 'target' structure, is required in order to accomplish a certain function. So a sequence that folds correctly to a target structure is seen as having a greater fitness than one that does not. Since we've encoded structures as simple dot-bracket strings, we can easily compare structures and thus evaluate the fitness between a given structure and the target, or 'correct' structure. \n"
        "\n"
        "There are many ways to compare structures $w_{1}$ and $w_{2}$, but we're going to use one of the simplest ways, which is base-pair distance. This is just the number of pairs in $w_{1}$ that are not in $w_{2}$. Again, this is beyond the scope of this workshop so I'll just give you the code for it and if you would like to know more you can ask me."
    ));

    qDebug() << "command entry 3";
    testCommandEntry(entry, 3, 1, QString::fromUtf8(
        "### 3\n"
        "\n"
        "#ss_to_bp() and bp_distance() by Vladimir Reinharz.\n"
        "def ss_to_bp(ss):\n"
        "    bps = set()\n"
        "    l = []\n"
        "    for i, x in enumerate(ss):\n"
        "            if x == '(':\n"
        "                    l.append(i)\n"
        "            elif x == ')':\n"
        "                    bps.add((l.pop(), i))\n"
        "    return bps\n"
        "\n"
        "def bp_distance(w1, w2):\n"
        "    \"\"\"\n"
        "    return base pair distance between structures w1 and w1. \n"
        "    w1 and w1 are lists of tuples representing pairing indices.\n"
        "    \"\"\"\n"
        "    return len(set(w1).symmetric_difference(set(w2)))\n"
        "\n"
        "#let's fold two sequences\n"
        "w1 = nussinov(\"CCAAAAGG\")\n"
        "w2 = nussinov(\"ACAAAAGA\")\n"
        "\n"
        "print(w1)\n"
        "print(w2)\n"
        "\n"
        "#give the list of pairs to bp_distance and see what the distance is.\n"
        "print(bp_distance(w1[-1], w2[-1]))"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "('((....))', [(0, 7), (1, 6)])\n"
        "('.(....).', [(1, 6)])\n"
        "1"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Defining a cell: a little bit of Object Oriented Programming (OOP)\n"
        "\n"
        "Since we're going to be playing aroudn with sequences and structures and fitness values a lot, it's best to package it all nicely into an object. As you'll have seen with Vlad, objects are just a nice way of grouping data into an easily accessible form. \n"
        "\n"
        "We're trying to simulate evolution on a very simple kind of organism, or cell. It contains two copies of a RNA gene, each with a corresponding structure. "
    ));

    qDebug() << "command entry 4";
    testCommandEntry(entry, 4, 1, QString::fromUtf8(
        "### 4\n"
        "class Cell:\n"
        "    def __init__(self, seq_1, struc_1, seq_2, struc_2):\n"
        "        self.sequence_1 = seq_1\n"
        "        self.sequence_2 = seq_2\n"
        "        self.structure_1 = struc_1\n"
        "        self.structure_2 = struc_2\n"
        "        \n"
        "#for now just try initializing a Cell with made up sequences and structures\n"
        "cell = Cell(\"AACCCCUU\", \"((.....))\", \"GGAAAACA\", \"(....).\")\n"
        "print(cell.sequence_1, cell.structure_2, cell.sequence_1, cell.structure_2)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "AACCCCUU (....). AACCCCUU (....)."
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Populations of Cells\n"
        "\n"
        "Now we've defined a 'Cell'. Since a population is a collection of individuals our populations will naturally consist of **lists** of 'Cell' objects, each with their own sequences. Here we initialize all the Cells with random sequences and add them to the 'population' list."
    ));

    qDebug() << "command entry 5";
    testCommandEntry(entry, 5, QString::fromUtf8(
        "### 5\n"
        "import random\n"
        "\n"
        "def populate(target, pop_size=100):\n"
        "    \n"
        "    population = []\n"
        "\n"
        "    for i in range(pop_size):\n"
        "        #get a random sequence to start with\n"
        "        sequence = \"\".join([random.choice(\"AUCG\") for _ in range(len(target))])\n"
        "        #use nussinov to get the secondary structure for the sequence\n"
        "        structure = nussinov(sequence)\n"
        "        #add a new Cell object to the population list\n"
        "        new_cell = Cell(sequence, structure, sequence, structure)\n"
        "        new_cell.id = i\n"
        "        new_cell.parent = i\n"
        "        population.append(new_cell)\n"
        "            \n"
        "    return population"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Try creating a new population and printing the first 10 sequences and structures (in dot-bracket) on the first chromosome!"
    ));

    qDebug() << "command entry 6";
    testCommandEntry(entry, 6, 1, QString::fromUtf8(
        "### 6\n"
        "target = \"(.(((....).).).)....\"\n"
        "pop = populate(target, pop_size=100)\n"
        "for p in pop[:10]:\n"
        "    print(p.id, p.sequence_1, p.structure_1[0], p.sequence_2, p.structure_2[0])"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "0 GGACGGAGCAUUAUCUGCUA (((...(....).))..).. GGACGGAGCAUUAUCUGCUA (((...(....).))..)..\n"
        "1 ACAAUCGCCUCACUCACGUU (.(..((..(.....))))) ACAAUCGCCUCACUCACGUU (.(..((..(.....)))))\n"
        "2 UCCGUUCUAUUAGUUCAUAG .(..(((.....)...).)) UCCGUUCUAUUAGUUCAUAG .(..(((.....)...).))\n"
        "3 CAAACCUUGUUCGUAAUACA .(....)((((....).))) CAAACCUUGUUCGUAAUACA .(....)((((....).)))\n"
        "4 GCAUCGAGUGCGCGGCAUAA ((..((....)).).).... GCAUCGAGUGCGCGGCAUAA ((..((....)).).)....\n"
        "5 GAAUUCUGAGAUCAUACUCG (((((.....)..))..)). GAAUUCUGAGAUCAUACUCG (((((.....)..))..)).\n"
        "6 GGAACCGUAGGCUUUGCAAG (.(((....)..))..)... GGAACCGUAGGCUUUGCAAG (.(((....)..))..)...\n"
        "7 GCAAAAGACAGCCCGCAUCA ((....).)((....).).. GCAAAAGACAGCCCGCAUCA ((....).)((....).)..\n"
        "8 GGGUACCGACAACGGAGCUC ((.(.(((....)))).).) GGGUACCGACAACGGAGCUC ((.(.(((....)))).).)\n"
        "9 CUCUUAUUUCACUUAGCUGU (.(((.....)...))..). CUCUUAUUUCACUUAGCUGU (.(((.....)...))..)."
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## The Fitness of a Cell\n"
        "\n"
        "Now that we can store populatoins of cells, we need a way to evaluate the fitness of a given Cell. Recall that a Cell is simply an object that contains two RNA sequences (think of it as two copies of a gene on each chromosome). \n"
        "\n"
        "So we simply need to loop through each Cell in a population and compute base pair distance to the target structure. However, simply using base-pair distance is not a very good measure of fitness. There are two reasons for this: \n"
        "\n"
        "1. We want fitness to represent a *probability* that a cell will reproduce, and base pair distance is an integer.\n"
        "2. We want this probability to be a *relative* measure. That is, we want to be the fitness to be proportional to how good a cell is with respect to all others in the population. This touches on an important principle in evolution where we only need to be 'better' than the competition and not good in some absolute measure. For example, if you and I are being chased by a bear. In order to survive, I only need to be faster than you, and not necessarily some absolute level of fitness.\n"
        "\n"
        "In order to get a probability (number between 0 and 1) we use the following equation to define the fitness of a structure $\\omega$ on a target structure $T$:\n"
        "\n"
        "$$P(\\omega, T) = N^{-1} exp(\\frac{-\\beta \\texttt{dist}(\\omega, T)}{\\texttt{len}(\\omega)})$$\n"
        "\n"
        "$$N = \\sum_{i \\in Pop}{P(\\omega_i, T})$$\n"
        "\n"
        "Here, the $N$ is what gives us the 'relative' measure because we divide the fitness of the Cell by the sum of the fitness of every other Cell. \n"
        "\n"
        "Let's take a quick look at how this function behaves if we plot different base pair distance values.\n"
        "\n"
        "What is the effect of the parameter $\\beta$? Try plotting the same function but with different values of $\\beta$."
    ));

    qDebug() << "command entry 8";
    testCommandEntry(entry, 8, 2, QString::fromUtf8(
        "%matplotlib inline\n"
        "import matplotlib.pyplot as plt\n"
        "import math\n"
        "import seaborn as sns\n"
        "\n"
        "target_length = 50\n"
        "beta = -2\n"
        "\n"
        "plt.plot([math.exp(beta * (bp_dist / float(target_length))) for bp_dist in range(target_length)])\n"
        "plt.xlabel(\"Base pair distance to target structure\")\n"
        "plt.ylabel(\"P(w, T)\")"
    ));
    testTextResult(entry, 0, QString::fromLatin1("Text(0,0.5,'P(w, T)')"));
    testImageResult(entry, 1);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "As you can see, it's a very simple function that evaluates to 1 (highest fitness) if the base pair distance is 0, and decreases as the structures get further and further away from the target. I didn't include the $N$ in the plotting as it will be a bit more annoying to compute, but it is simply a scaling factor so the shape and main idea won't be different.\n"
        "\n"
        "Now we can use this function to get a fitness value for each Cell in our population."
    ));

    qDebug() << "command entry 9";
    testCommandEntry(entry, 9, 1, QString::fromUtf8(
        "### 7\n"
        "\n"
        "def compute_fitness(population, target, beta=-2):\n"
        "    \"\"\"\n"
        "    Assigns a fitness and bp_distance value to each cell in the population.\n"
        "    \"\"\"\n"
        "    #store the fitness values of each cell\n"
        "    tot = []\n"
        "    #iterate through each cell\n"
        "    for cell in population:\n"
        "        \n"
        "        #calculate the bp_distance of each chromosome using the cell's structure\n"
        "        bp_distance_1 = bp_distance(cell.structure_1[-1], ss_to_bp(target))\n"
        "        bp_distance_2 = bp_distance(cell.structure_2[-1], ss_to_bp(target))\n"
        "        \n"
        "        #use the bp_distances and the above fitness equation to calculate the fitness of each chromosome\n"
        "        fitness_1 = math.exp((beta * bp_distance_1 / float(len(cell.sequence_1))))\n"
        "        fitness_2 =  math.exp((beta * bp_distance_2 / float(len(cell.sequence_2))))\n"
        "\n"
        "        #get the fitness of the whole cell by multiplying the fitnesses of each chromosome\n"
        "        cell.fitness = fitness_1 * fitness_2\n"
        "               \n"
        "        #store the bp_distance of each chromosome.\n"
        "        cell.bp_distance_1 = bp_distance_1\n"
        "        cell.bp_distance_2 = bp_distance_2\n"
        "    \n"
        "        \n"
        "        #add the cell's fitness value to the list of all fitness values (used for normalization later)\n"
        "        tot.append(cell.fitness)\n"
        "\n"
        "    #normalization factor is sum of all fitness values in population\n"
        "    norm = np.sum(tot)\n"
        "    #divide all fitness values by the normalization factor.\n"
        "    for cell in population:\n"
        "        cell.fitness = cell.fitness / norm\n"
        "\n"
        "    return None\n"
        "\n"
        "compute_fitness(pop, target)\n"
        "for cell in pop[:10]:\n"
        "    print(cell.fitness, cell.bp_distance_1, cell.bp_distance_2)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "0.013612068231863143 6 6\n"
        "0.007470461436952334 9 9\n"
        "0.009124442203822766 8 8\n"
        "0.007470461436952334 9 9\n"
        "0.013612068231863143 6 6\n"
        "0.007470461436952334 9 9\n"
        "0.02030681957427158 4 4\n"
        "0.009124442203822766 8 8\n"
        "0.006116296518116008 10 10\n"
        "0.009124442203822766 8 8"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Introducing diversity: Mutations\n"
        "\n"
        "Evolution would go nowhere without random mutations. While mutations are technically just random errors in the copying of genetic material, they are essential in the process of evolution. This is because they introduce novel diversity to populatons, which with a low frequency can be beneficial. And when a beneficial mutation arises (i.e. a mutation that increases fitness, or replication probability) it quickly takes over the population and the populatioin as a whole has a higher fitness.\n"
        "\n"
        "Implementing mutations in our model will be quite straightforward. Since mutations happen at the genotype/sequence level, we simply have to iterate through our strings of nucleotides (sequences) and randomly introduce changes."
    ));

    qDebug() << "command entry 10";
    testCommandEntry(entry, 10, 1, QString::fromUtf8(
        "def mutate(sequence, mutation_rate=0.001):\n"
        "    \"\"\"Takes a sequence and mutates bases with probability mutation_rate\"\"\"\n"
        "    \n"
        "    #start an empty string to store the mutated sequence\n"
        "    new_sequence = \"\"\n"
        "    #boolean storing whether or not the sequence got mutated\n"
        "    mutated = False\n"
        "    #go through every bp in the sequence\n"
        "    for bp in sequence:\n"
        "        #generate a random number between 0 and 1\n"
        "        r = random.random()\n"
        "        #if r is below mutation rate, introduce a mutation\n"
        "        if r < mutation_rate:\n"
        "            #add a randomly sampled nucleotide to the new sequence\n"
        "            new_sequence = new_sequence + random.choice(\"aucg\")\n"
        "            mutated = True\n"
        "        else:\n"
        "            #if the mutation condition did not get met, copy the current bp to the new sequence\n"
        "            new_sequence = new_sequence + bp\n"
        "            \n"
        "    return (new_sequence, mutated)\n"
        "\n"
        "sequence_to_mutate = 'AAAAGGAGUGUGUAUGU'\n"
        "print(sequence_to_mutate)\n"
        "print(mutate(sequence_to_mutate, mutation_rate=0.5))"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "AAAAGGAGUGUGUAUGU\n"
        "('AcAAGgAuUGUuaAaGa', True)"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Selection\n"
        "\n"
        "The final process in this evolution model is selection. Once you have populations with a diverse range of fitnesses, we need to select the fittest individuals and let them replicate and contribute offspring to the next generation. In real populations this is just the process of reproduction. If you're fit enough you will be likely to reproduce more than another individual who is not as well suited to the environment.\n"
        "\n"
        "In order to represent this process in our model, we will use the fitness values that we assigned to each Cell earlier and use that to select replicating Cells. This is equivalent to sampling from a population with the sampling being weighted by the fitness of each Cell. Thankfully, `numpy.random.choice` comes to the rescue here. Once we have sampled enough Cells to build our next generation, we introduce mutations and compute the fitness values of the new generation."
    ));

    qDebug() << "command entry 11";
    testCommandEntry(entry, 11, 1, QString::fromUtf8(
        "def selection(population, target, mutation_rate=0.001, beta=-2):\n"
        "    \"\"\"\n"
        "    Returns a new population with offspring of the input population\n"
        "    \"\"\"\n"
        "\n"
        "    #select the sequences that will be 'parents' and contribute to the next generation\n"
        "    parents = np.random.choice(population, len(population), p=[rna.fitness for rna in population], replace=True)\n"
        "\n"
        "    #build the next generation using the parents list\n"
        "    next_generation = []    \n"
        "    for i, p in enumerate(parents):\n"
        "        new_cell = Cell(p.sequence_1, p.structure_1, p.sequence_2, p.structure_2)\n"
        "        new_cell.id = i\n"
        "        new_cell.parent = p.id\n"
        "        \n"
        "        next_generation.append(new_cell)\n"
        "\n"
        "    #introduce mutations in next_generation sequeneces and re-fold when a mutation occurs\n"
        "    for rna in next_generation:      \n"
        "        mutated_sequence_1, mutated_1 = mutate(rna.sequence_1, mutation_rate=mutation_rate)\n"
        "        mutated_sequence_2, mutated_2 = mutate(rna.sequence_2, mutation_rate=mutation_rate)\n"
        "        \n"
        "        if mutated_1:\n"
        "            rna.sequence_1 = mutated_sequence_1\n"
        "            rna.structure_1 = nussinov(mutated_sequence_1)\n"
        "        if mutated_2:\n"
        "            rna.sequence_2 = mutated_sequence_2\n"
        "            rna.structure_2 = nussinov(mutated_sequence_2)\n"
        "        else:\n"
        "            continue\n"
        "\n"
        "    #update fitness values for the new generation\n"
        "    compute_fitness(next_generation, target, beta=beta)\n"
        "\n"
        "    return next_generation\n"
        "\n"
        "next_gen = selection(pop, target)\n"
        "for cell in next_gen[:10]:\n"
        "    print(cell.sequence_1)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "GAGCUUUAAACUAAUCUAAU\n"
        "GCAAAAGACAGCCaGCAUCA\n"
        "UUUCUUUUUCCCCCCCGAUG\n"
        "AAGCCCUAGGUAUGUUGUAG\n"
        "AAGAAGUACCCAUACAGAUG\n"
        "CUAAGACGACUUUUAGUUCA\n"
        "ACCUGCCAUCAUCACCAGAC\n"
        "AGAAUUGCUGUUCUCUAUCU\n"
        "GCGGAUCAUACUCCAAGUCG\n"
        "GAGCUUUAAACUAAUCUAAU"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Gathering information on our populations\n"
        "\n"
        "Here we simply store some statistics (in a dictionary) on the population at each generation such as the average base pair distance and the average fitness of the populations. No coding to do here, it's not a very interesting function but feel free to give it a look."
    ));

    qDebug() << "command entry 12";
    testCommandEntry(entry, 12, QString::fromUtf8(
        "def record_stats(pop, population_stats):\n"
        "    \"\"\"\n"
        "    Takes a population list and a dictionary and updates it with stats on the population.\n"
        "    \"\"\"\n"
        "    generation_bp_distance_1 = [rna.bp_distance_1 for rna in pop]\n"
        "    generation_bp_distance_2 = [rna.bp_distance_2 for rna in pop]\n"
        "\n"
        "    mean_bp_distance_1 = np.mean(generation_bp_distance_1)\n"
        "    mean_bp_distance_2 = np.mean(generation_bp_distance_2)\n"
        "    \n"
        "    mean_fitness = np.mean([rna.fitness for rna in pop])\n"
        "\n"
        "\n"
        "    population_stats.setdefault('mean_bp_distance_1', []).append(mean_bp_distance_1)\n"
        "    population_stats.setdefault('mean_bp_distance_2', []).append(mean_bp_distance_2)\n"
        "    \n"
        "    population_stats.setdefault('mean_fitness', []).append(mean_fitness)\n"
        "    \n"
        "    return None"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## And finally.... evolution\n"
        "\n"
        "We can put all the above parts together in a simple function that does the following:\n"
        "\n"
        "1. start a new population and compute its fitness\n"
        "2. repeat the following for the desired number of generations:\n"
        "    1. record statistics on population\n"
        "    2. perform selection+mutation\n"
        "    3. store new population\n"
        "\n"
        "And that's it! We have an evolutionary reactor!"
    ));

    qDebug() << "command entry 13";
    testCommandEntry(entry, 13, QString::fromUtf8(
        "def evolve(target, generations=10, pop_size=100, mutation_rate=0.001, beta=-2):\n"
        "    \"\"\"\n"
        "    Takes target structure and sets up initial population, performs selection and iterates for desired generations.\n"
        "    \"\"\"\n"
        "    #store list of all populations throughotu generations [[cells from generation 1], [cells from gen. 2]...]\n"
        "    populations = []\n"
        "    #start a dictionary that will hold some stats on the populations.\n"
        "    population_stats = {}\n"
        "    \n"
        "    #get a starting population\n"
        "    initial_population = populate(target, pop_size=pop_size)\n"
        "    #compute fitness of initial population\n"
        "    compute_fitness(initial_population, target)\n"
        "\n"
        "    #set current_generation to initial population.\n"
        "    current_generation = initial_population\n"
        "\n"
        "    #iterate the selection process over the desired number of generations\n"
        "    for i in range(generations):\n"
        "\n"
        "        #let's get some stats on the structures in the populations   \n"
        "        record_stats(current_generation, population_stats)\n"
        "        \n"
        "        #add the current generation to our list of populations.\n"
        "        populations.append(current_generation)\n"
        "\n"
        "        #select the next generation\n"
        "        new_gen = selection(current_generation, target, mutation_rate=mutation_rate, beta=beta)\n"
        "        #set current generation to be the generation we just obtained.\n"
        "        current_generation = new_gen \n"
        "    \n"
        "    return (populations, population_stats)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Try a run of the `evolve()` function."
    ));

    qDebug() << "command entry 14";
    testCommandEntry(entry, 14, QString::fromUtf8(
        "pops, pops_stats = evolve(\"(((....)))\", generations=20, pop_size=1000, mutation_rate=0.005, beta=-2)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Let's see if it actually worked by plotting the average base pair distance as a function of generations for both genes in each cell. We should expect a gradual decrease as the populations get closer to the target structure."
    ));

    qDebug() << "command entry 15";
    testCommandEntry(entry, 15, 1, QString::fromUtf8(
        "def evo_plot(pops_stats):\n"
        "    \"\"\"\n"
        "    Plot base pair distance for each chromosome over generations.\n"
        "    \"\"\"\n"
        "    for m in ['mean_bp_distance_1', 'mean_bp_distance_2']:\n"
        "        plt.plot(pops_stats[m], label=m)\n"
        "    plt.legend()\n"
        "    plt.xlabel(\"Generations\")\n"
        "    plt.ylabel(\"Mean Base Pair Distance\")\n"
        "    \n"
        "evo_plot(pops_stats)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "You should see a nice drop in base pair distance! Another way of visualizing this is by plotting a histogram of the base pair distance of all Cells in the initial population versus the final population."
    ));

    qDebug() << "command entry 16";
    testCommandEntry(entry, 16, 1, QString::fromUtf8(
        "def bp_distance_distributions(pops):\n"
        "    \"\"\"\n"
        "    Plots histograms of base pair distance in initial and final populations.\n"
        "    \"\"\"\n"
        "    #plot bp_distance_1 for rnas in first population\n"
        "    g = sns.distplot([rna.bp_distance_1 for rna in pops[0]], label='initial population')\n"
        "    #plot bp_distance_1 for rnas in first population\n"
        "    g = sns.distplot([rna.bp_distance_1 for rna in pops[-1]], label='final population')\n"
        "    g.set(xlabel='Mean Base Pair Distance')\n"
        "    g.legend()\n"
        "bp_distance_distributions(pops)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Studying our evolved sequences with some Population Genetics tools\n"
        "\n"
        "Now that we've generated some sequences, we can analyze them!\n"
        "\n"
        "So after several rounds of selection, what do we get? We have a bunch of different sequences. We would like a way to characterize this diversity. One important tool for doing this is by making what is known as phylogenetic trees. \n"
        "\n"
        "Phylogenetic trees tell us about which groups of similar sequences are present and how they are likely related in evolutionary time. \n"
        "\n"
        "There are several ways of building phylogenetic trees using BioPython. Here we will go over one type and I'll leave another one as an exercise.\n"
        "\n"
        "### UPGMA (Unweighted Pair Group Method with Arithmetic Means)\n"
        "\n"
        "This is basically a clustering method based on the distance (or number of differences) between every pair of sequences. It assumes that sequences that are more similar are more likely to be related than the other way around. \n"
        "\n"
        "For $N$ sequences, the algorithm builds an $NxN$ matrix that stores the distance between each sequence to every other sequence. The algorithm goes through this matrix and finds the pair of sequences that is most similar and merges it into a 'cluster' or in tree terms, connects them to a common node. This process is repeated until all the sequences have been assigned to a group. Refer to the wikipedia article on [UPGMA](https://en.wikipedia.org/wiki/UPGMA) for a more detailed explanation. \n"
        ""
    ));

    qDebug() << "command entry 18";
    testCommandEntry(entry, 18, QString::fromUtf8(
        "from Bio import SeqIO\n"
        "from Bio.Seq import Seq\n"
        "from Bio.SeqRecord import SeqRecord\n"
        "from Bio import AlignIO"
    ));

    qDebug() << "command entry 19";
    testCommandEntry(entry, 19, QString::fromUtf8(
        "sequences = []\n"
        "#let's take the first 10 sequences of our population to keep things simple\n"
        "for seq in pops[-1][:10]:\n"
        "    #store each sequence in the sequences list as a SeqRecord object\n"
        "    sequences.append(SeqRecord(Seq(seq.sequence_1), id=str(seq.id)))\n"
        "    \n"
        "\n"
        "#write our sequences to fasta format\n"
        "with open(\"seq.fasta\", \"w+\") as f:\n"
        "    SeqIO.write(sequences, f, \"fasta\")"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "The UPGMA algorithm requires a `MultipleSeqAlignment` object to build the distance matrix. So now that we have the `seq.fasta` file, we can give it to an online multiple sequence alignment tool. We can do this through BioPython but it requires some installation and setup so we will skip that for now. Go to the [MUSCLE Web Server](http://www.ebi.ac.uk/Tools/msa/muscle/) and give it the `seq.fasta` file. It will take a few seconds and it will give you an alignment and click *Download Alignment File*, copy paste the whole thing to a new file called `aln.clustal`. This is the alignment we will use to build our tree."
    ));

    qDebug() << "command entry 21";
    testCommandEntry(entry, 21, QString::fromUtf8(
        "#open the alignmnent file\n"
        "with open(\"aln.clustal\", \"r\") as aln:\n"
        "    #use AlignIO to read the alignment file in 'clustal' format\n"
        "    alignment = AlignIO.read(aln, \"clustal\")"
    ));

    qDebug() << "command entry 22";
    testCommandEntry(entry, 22, 1, QString::fromUtf8(
        "from Bio.Phylo.TreeConstruction import DistanceCalculator\n"
        "\n"
        "#calculate the distance matrix\n"
        "calculator = DistanceCalculator('identity')\n"
        "#adds distance matrix to the calculator object and returns it\n"
        "dm = calculator.get_distance(alignment)\n"
        "print(dm)\n"
        ""
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "8\t0\n"
        "7\t0.6\t0\n"
        "0\t0.8\t0.6\t0\n"
        "3\t1.0\t0.7\t0.4\t0\n"
        "6\t1.0\t0.7\t0.4\t0.09999999999999998\t0\n"
        "9\t0.8\t1.0\t0.5\t0.7\t0.6\t0\n"
        "1\t0.9\t0.9\t0.8\t0.9\t0.8\t0.4\t0\n"
        "2\t0.9\t0.9\t0.8\t0.9\t0.8\t0.4\t0.0\t0\n"
        "4\t0.9\t0.9\t0.9\t1.0\t0.9\t0.5\t0.09999999999999998\t0.09999999999999998\t0\n"
        "5\t0.9\t0.9\t0.9\t1.0\t0.9\t0.5\t0.09999999999999998\t0.09999999999999998\t0.0\t0\n"
        "\t8\t7\t0\t3\t6\t9\t1\t2\t4\t5"
    ));
    entry = entry->next();

    qDebug() << "command entry 23";
    testCommandEntry(entry, 23, QString::fromUtf8(
        "from Bio.Phylo.TreeConstruction import DistanceTreeConstructor\n"
        "\n"
        "#initialize a DistanceTreeConstructor object based on our distance calculator object\n"
        "constructor = DistanceTreeConstructor(calculator)\n"
        "\n"
        "#build the tree\n"
        "upgma_tree = constructor.build_tree(alignment)"
    ));

    qDebug() << "command entry 24";
    testCommandEntry(entry, 24, 1, QString::fromUtf8(
        "from Bio import Phylo\n"
        "import pylab\n"
        "#draw the tree\n"
        "Phylo.draw(upgma_tree)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Introducing mating to the model\n"
        "\n"
        "The populations we generated evolved asexually. This means that individuals do not mate or exchange genetic information. So to make our simulation a bit more interesting let's let the Cells mate. This is going to require a few small changes in the `selection()` function. Previously, when we selected sequences to go into the next generation we just let them provide one offspring which was a copy of itself and introduced mutations. Now instead of choosing one Cell at a time, we will randomly choose two 'parents' that will mate. When they mate, each parent will contribute one of its chromosomes to the child. We'll repeat this process until we have filled the next generation."
    ));

    qDebug() << "command entry 25";
    testCommandEntry(entry, 25, 1, QString::fromUtf8(
        "def selection_with_mating(population, target, mutation_rate=0.001, beta=-2):\n"
        "    next_generation = []\n"
        "    \n"
        "    counter = 0\n"
        "    while len(next_generation) < len(population):\n"
        "        #select two parents based on their fitness\n"
        "        parents_pair = np.random.choice(population, 2, p=[rna.fitness for rna in population], replace=False)\n"
        "        \n"
        "        #take the sequence and structure from the first parent's first chromosome and give it to the child\n"
        "        child_chrom_1 = (parents_pair[0].sequence_1, parents_pair[0].structure_1)\n"
        "\n"
        "        #do the same for the child's second chromosome and the second parent.\n"
        "        child_chrom_2 = (parents_pair[1].sequence_2, parents_pair[1].structure_2)\n"
        "\n"
        "\n"
        "        #initialize the new child Cell witht he new chromosomes.\n"
        "        child_cell = Cell(child_chrom_1[0], child_chrom_1[1], child_chrom_2[0], child_chrom_2[1])\n"
        "\n"
        "        #give the child and id and store who its parents are\n"
        "        child_cell.id = counter\n"
        "        child_cell.parent_1 = parents_pair[0].id\n"
        "        child_cell.parent_2 = parents_pair[1].id\n"
        "\n"
        "        #add the child to the new generation\n"
        "        next_generation.append(child_cell)\n"
        "        \n"
        "        counter = counter + 1\n"
        "            \n"
        "        \n"
        "    #introduce mutations in next_generation sequeneces and re-fold when a mutation occurs (same as before)\n"
        "    for rna in next_generation:      \n"
        "        mutated_sequence_1, mutated_1 = mutate(rna.sequence_1, mutation_rate=mutation_rate)\n"
        "        mutated_sequence_2, mutated_2 = mutate(rna.sequence_2, mutation_rate=mutation_rate)\n"
        "\n"
        "        if mutated_1:\n"
        "            rna.sequence_1 = mutated_sequence_1\n"
        "            rna.structure_1 = nussinov(mutated_sequence_1)\n"
        "        if mutated_2:\n"
        "            rna.sequence_2 = mutated_sequence_2\n"
        "            rna.structure_2 = nussinov(mutated_sequence_2)\n"
        "        else:\n"
        "            continue\n"
        "\n"
        "    #update fitness values for the new generation\n"
        "    compute_fitness(next_generation, target, beta=beta)\n"
        "\n"
        "    return next_generation    \n"
        "\n"
        "#run a small test to make sure it works\n"
        "next_gen = selection_with_mating(pop, target)\n"
        "for cell in next_gen[:10]:\n"
        "    print(cell.sequence_1)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "CGUACCUGAAAAGCUAACUA\n"
        "GGAACCGUAGGCUUUGCAAG\n"
        "ACCUGCCAUCAUCACCAGAC\n"
        "UAGAGGUAGAAUUGUAGGCU\n"
        "GAUUCCGCGCGAAUACCGCG\n"
        "GCAUCGAGUGCGCGGCAUAA\n"
        "UAAUAAAAAGGUGCUGAUAU\n"
        "GAUUCCGCGCGAAUACCGCG\n"
        "UCACUAAACUCCUCGACUAC\n"
        "AUGAUCAUGGUGAGCAGUUU"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Now we just have to update our `evolution()` function to call the new `selection_with_mating()` function."
    ));

    qDebug() << "command entry 26";
    testCommandEntry(entry, 26, QString::fromUtf8(
        "def evolve_with_mating(target, generations=10, pop_size=100, mutation_rate=0.001, beta=-2):\n"
        "    populations = []\n"
        "    population_stats = {}\n"
        "    \n"
        "    initial_population = populate(target, pop_size=pop_size)\n"
        "    compute_fitness(initial_population, target)\n"
        "        \n"
        "    current_generation = initial_population\n"
        "\n"
        "    #iterate the selection process over the desired number of generations\n"
        "    for i in range(generations):\n"
        "        #let's get some stats on the structures in the populations   \n"
        "        record_stats(current_generation, population_stats)\n"
        "        \n"
        "        #add the current generation to our list of populations.\n"
        "        populations.append(current_generation)\n"
        "\n"
        "        #select the next generation, but this time with mutations\n"
        "        new_gen = selection_with_mating(current_generation, target, mutation_rate=mutation_rate, beta=beta)\n"
        "        current_generation = new_gen \n"
        "    \n"
        "    return (populations, population_stats)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Try out the new evolution model!"
    ));

    qDebug() << "command entry 28";
    testCommandEntry(entry, 28, 1, QString::fromUtf8(
        "pops_mating, pops_stats_mating = evolve_with_mating(\"(((....)))\", generations=20, pop_size=1000, beta=0)\n"
        "\n"
        "evo_plot(pops_stats_mating)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Hardy Weinberg Equilibrium\n"
        "\n"
        "When we are presented with data from a population we don't know much about. It is often useful to try to learn whether there are any evolutionary or behavioural influences that are shaping population dynamics. This could be in the form of selective pressure, mating preference, genetic drift, mutations, gene flow, etc. So in order to detect if something like this is happening we need to develop a test. This is where Hardy Weinberg comes in. \n"
        "\n"
        "The Hardy Weinberg equilibrium states that \"allele and genotype frequencies remain constant in the absence of other evolutionary influences. (such as the ones we mentioned above)\" - Wikipedia.\n"
        "\n"
        "So if we can measure allele/genotype frequencies (which we can do because we have sequences), we can see whether the HW principle holds true. If it does not, then we can do more digging to see what could be happening to shift populations away from equilibrium.\n"
        "\n"
        "In order to do this we need to define an 'allele'. An allele (for our purproses) will be a locus (position in a sequence) that can take one of two states, a *reference* state or an *alternate* state. For example, we can look at locus number **5** (position 5 in our RNA sequences) and call reference **C**, and alternate **G**. If we are in HW we can predict the frequency of each allele in our population.\n"
        "\n"
        "To simplify our notation we will call the alternate allele *A* and the reference allele *a*. We can write the probability of each allele as $p_{A} + p_{a} = 1$. Since we are dealing with diploid populations, each individual will have two copies of each locus so it can be $p_{AA}, p{Aa}, p{aA}, p{aa}$. By simple probability laws we can get an expression for the probability of each genotype based on the probabilities of the single loci $p_{a}$ and $p_{A}$.\n"
        "\n"
        "$$p_{aa}\\simeq p_{a}^2$$\n"
        "\n"
        "$$p_{AA}\\simeq p_{A}^2$$\n"
        "\n"
        "$$p_{Aa,~aA} \\simeq 2 p_{a} p_{A}.$$\n"
        "\n"
        "Since it is hard to know what the true probability of observing either $p_{a}$ and $p_{A}$ we can estimate this probability from our data as follows:\n"
        "\n"
        "$$\\hat p_a=\\frac{2N_{aa}+N_{aA}}{2N}=1-\\hat p_A.$$\n"
        "\n"
        "Where $N$ denotes the number of each genotype that we observe in our sequences. \n"
        "\n"
        "Based on these estimates we can expect the following frequencies for each genotype: \n"
        "\n"
        "$N_{aa}\\simeq e_{aa}=N \\hat p_a^2$\n"
        "\n"
        "$N_{AA}\\simeq e_{AA}= N \\hat p_{A}^2$\n"
        "\n"
        "$N_{Aa,~aA} \\simeq e_{Aa} = 2 N \\hat p_{a} \\hat p_{A}.$\n"
        "\n"
        "Now we have expected values, and observed values. We need a test to determine whether we have a significant departure from the hypothesis of Hardy Weinberg equilibrium. The statistical test that is commonly used is known as the $\\chi^{2}$ test. If you take a look at the equation you'll see that the statistic simply takes the squared difference between our observed value and the expected value (divided by expected) and sums this for each possible genotype. The reason we take the squared difference is because we want to deal only with positive values, hence the name $\\chi^{2}$.\n"
        "\n"
        "$$X^2= \\frac{(N_{aa}-e_{aa})^2}{e_{aa}}+ \\frac{(N_{Aa}-e_{Aa})^2}{e_{Aa}}+ \\frac{(N_{AA}-e_{AA})^2}{e_{AA}}.$$\n"
        "\n"
        "The first thing we need to do is get alleles from our sequence data. This boils down to going through each sequence at the position of interest and counting the number of $AA$, $Aa$, $aa$ we get.\n"
        "\n"
        "\n"
        "\\** the sections on Hardy Weinberg and F-statistics are adapted from Simon Gravel's HGEN 661 Notes"
    ));

    qDebug() << "command entry 29";
    testCommandEntry(entry, 29, QString::fromUtf8(
        "def allele_finder(pop, locus, ref, alt):\n"
        "    genotypes = []\n"
        "    for p in pop:\n"
        "        #get the nucleotide at the locus from the first chromosome \n"
        "        locus_1 = p.sequence_1[locus].upper()\n"
        "        #same for the second\n"
        "        locus_2 = p.sequence_2[locus].upper()\n"
        "        \n"
        "        #check that it is either ref or alt, we don't care about other alleles for now.\n"
        "        if locus_1 in (ref, alt) and locus_2 in (ref, alt):\n"
        "            #if the alelle is ref, store a value of 1 in allele_1, and 0 otherwise\n"
        "            allele_1 = int(locus_1 == ref)\n"
        "            #same for the second allele\n"
        "            allele_2 = int(locus_2 == ref)\n"
        "        \n"
        "            #add allele to our list of alleles as a tuple. \n"
        "            genotypes.append((allele_1, allele_2))\n"
        "    return genotypes"
    ));

    qDebug() << "command entry 30";
    testCommandEntry(entry, 30, 1, QString::fromUtf8(
        "pop_hw, stats_hw = evolve_with_mating(\"(((....)))\", pop_size=1000, generations=10, beta=0, mutation_rate=0.005)\n"
        "alleles = allele_finder(pop_hw[-1], 5,  'C', 'G')\n"
        "print(alleles[:10])"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "[(0, 0), (0, 0), (0, 1), (1, 0), (0, 1), (1, 0), (1, 0), (1, 0), (1, 0), (0, 1)]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Now that we have alleles represented in the right form, we can see if our population is at Hardy Weinberg equilibrium using the $\\chi_{2}$ test and the equations above."
    ));

    qDebug() << "command entry 31";
    testCommandEntry(entry, 31, 1, QString::fromUtf8(
        "from scipy import stats\n"
        "from scipy.stats import chi2\n"
        "\n"
        "def hardy_weinberg_chi2_test(alleles):\n"
        "    \n"
        "    #store counts for N_AA, N_Aa/aA, N_aa\n"
        "    hom_ref_count = 0\n"
        "    het_count = 0\n"
        "    hom_alt_count = 0\n"
        "    \n"
        "    #each allele in the list alleles is in the form (0,0) or (0,1) or (1,0) or (1,1)\n"
        "    #count how many of each type we have\n"
        "    for a in alleles:\n"
        "        if (a[0]==0 and a[1]==0):\n"
        "            hom_ref_count += 1\n"
        "        elif ((a[0]==0 and a[1]==1) or (a[0]==1 and a[1]==0)):\n"
        "            het_count += 1\n"
        "        elif (a[0]==1 and a[1]==1):\n"
        "            hom_alt_count += 1\n"
        "        else:\n"
        "            continue\n"
        "    \n"
        "    #total number of genotypes: N\n"
        "    genotype_count = hom_ref_count + het_count + hom_alt_count\n"
        "\n"
        "    #estimate p_a, p_A\n"
        "    alt_counts = (2 * hom_alt_count) + het_count\n"
        "    ref_counts = (2 * hom_ref_count) + het_count\n"
        "    \n"
        "    \n"
        "    #get expectations e_AA, e_aA,Aa, e_aa\n"
        "    hom_ref_expectation = ref_counts**2 / (4.*genotype_count) # the expected number of homozygote references  \n"
        "    het_expectation = ref_counts * alt_counts / (2.*genotype_count)  # the expected number of hets  \n"
        "    hom_alt_expectation = alt_counts**2 / (4.*genotype_count)  # the expected number of homozygote nonreferences  \n"
        "\n"
        "    #store observed values in list in the form [N_AA, N_aA,Aa, N_aa]\n"
        "    observations = [hom_ref_count, het_count, hom_alt_count]\n"
        "    #store expected values in the same form\n"
        "    expectations = [hom_ref_expectation, het_expectation, hom_alt_expectation]\n"
        "    \n"
        "    #start a dictionary that will store our results.\n"
        "    statistics = {\n"
        "                'hom_ref': (hom_ref_count, hom_ref_expectation),\n"
        "                'het': (het_count, het_expectation),\n"
        "                'hom_alt': (hom_alt_count, hom_alt_expectation), \n"
        "                'ref_counts': ref_counts, \n"
        "                'alt_counts': alt_counts,\n"
        "                'genotype_count': genotype_count\n"
        "                }\n"
        "\n"
        "    #call scipy function for chi2 test.\n"
        "    chi_2_statistic = stats.chisquare(observations, f_exp=expectations, ddof=1, axis=0)\n"
        "    \n"
        "    #return chi2 and statistics dictionary\n"
        "    return (chi_2_statistic, statistics)\n"
        "\n"
        "hardy_weinberg_chi2_test(alleles)"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
       "(Power_divergenceResult(statistic=0.001476611280908458, pvalue=0.9693474730942313),\n"
       " {'hom_ref': (81, 81.14693877551021),\n"
       "  'het': (120, 119.70612244897958),\n"
       "  'hom_alt': (44, 44.14693877551021),\n"
       "  'ref_counts': 282,\n"
       "  'alt_counts': 208,\n"
       "  'genotype_count': 245})"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Can we say that our population is at equilibrium? Can you find parameters for `evolution_with_mating()` that will give us populations outside of the HW equilibrium?"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## A brief interlude on the p-value\n"
        "\n"
        "Let's take a minute to understand what the p-value means. The p-value is a probability. Specifically, it is the probability of observing a value equal to or more extreme than that our statistic given the test distribution. So in our case, it is the probability of observing a $X^2$ greater than or equal to the one the test gives us under a $\\chi^2$ distribution. When this value is very small, it suggests that it is unlikely that we are sampling from our assumed 'null' distribution and that some other alternate distribution is the true distribution. So a low p-value here would be evidence against the neutral Hardy Weinberg model and would suggest that our population is experiencing some influences such as mating preference, selection, mutation etc.\n"
        "\n"
        "A lot of research bases its conclusions solely on p-value and it is important to be very wary of this bad practice. It has become a bad convention that people say a p-value lower than some arbitrary threshold means one's findings are significant. However, very often the p-value does not give us the whole story and we need to know about things like sample size, size of impact, reproducibility, power of the test, etc. (check this out [American Statistical Association statement on p-values](http://www.nature.com/news/statisticians-issue-warning-over-misuse-of-p-values-1.19503), [p-hacking](http://fivethirtyeight.com/features/science-isnt-broken/#part1), and [this](http://allendowney.blogspot.ca/2016/06/there-is-still-only-one-test.html))\n"
        "\n"
        "Let's just visualize this very quickly using the $\\chi^{2}_{1}$ distribution. You will see that the p-value corresponds to the shaded red area under the curve. That area is the probability of observing a value as extreme or more than the one we found. When that is a very small area, we can be more confident that our assumption of HW is false."
    ));

    qDebug() << "command entry 32";
    testCommandEntry(entry, 32, 2, QString::fromUtf8(
        "#number of samples to take from the x2 distribution.\n"
        "number_of_samples = 1000\n"
        "\n"
        "range_points = 2000\n"
        "range_start = 0\n"
        "\n"
        "degrees_of_freedom = 1\n"
        "\n"
        "range_end = chi2.ppf(1-1./number_of_samples, degrees_of_freedom)\n"
        "                     \n"
        "x_range = np.linspace(range_start, range_end, range_points) \n"
        "plt.plot(x_range, chi2.pdf(x_range, degrees_of_freedom))\n"
        "\n"
        "#find the index value of our statistic value. you can put in different values here.\n"
        "statistic = 0.5\n"
        "\n"
        "#find the index in x_range corresponding to the statistic value (within 0.01)\n"
        "point = 0\n"
        "for i, nb in enumerate(x_range):\n"
        "    if nb < statistic + .01 and nb > statistic - .01:\n"
        "        point = i\n"
        "\n"
        "#fill area under the curve representing p-value\n"
        "plt.fill_between(x_range[point:], chi2.pdf(x_range, degrees_of_freedom)[point:], alpha=0.3, color=\"red\")\n"
        "\n"
        "plt.xlabel(\"X-statistic\")\n"
        "plt.ylabel(r\"$\\chi^2_%d$\" % degrees_of_freedom)\n"
        ""
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Text(0,0.5,'$\\\\chi^2_1$')"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Population structure: F-statistics\n"
        "\n"
        "The last topic we'll cover is F-statistics. \n"
        "\n"
        "Once we find that our population strays from the HW condition we can begin to ask why that is the case. Often this deviation from the expected allele frequencies under HW is due to mating preference. Hardy Weinberg assumes that all individuals in a population have an equal probability of mating with any other individual (random mating). However, when certain individuals prefer to mate with specific others (in real populations this can be due to culture, race, geographic barriers, etc.), you get what is known as population structure. Population structure means that we begin to see *sub-populations* within our total population where individuals prefer to mate within their sub-population. This biased mating will result in a higher number of homozygotes than we would expect under Hardy-Weinberg equilibrium. Simply because mating preferences will tend to drive populations toward similar genotypes. So if this is the case, and no other factors are biasing allele dynamics, within sub-populations we should have Hardy-Weinberg like conditions. \n"
        "\n"
        "For example, if Raptors fans prefer to mate with other Raptors fans, then when we consider only Raptors fans, we should observe random mating. Simply because if the mating preference criterion is 'being a Raptor's fan' then any Raptor's fan will be equally likely to mate with any other Raptor's fan so we have Hardy Weinberg again.\n"
        "\n"
        "Let's express this in quantities we can measure.\n"
        "\n"
        "From before we calculated the observed and expected number of heterozygotes in a population. Let's call these $\\hat H$ and $H_{IT}$ respectively. $\\hat H$ is just the count of heterozygotes, and $H_{IT}$ is the same as the expected number of heterozygotes we calculated earlier.\n"
        "\n"
        "We define a quantity $e_{IT}$ as a measure of the 'excess heterozygosity' in the population when we consider all individuals $I$ in the total population $T$. $e_{IT} > 1$ when we have more heterozygotes than we expect under HW. And $0 < e_{IT} < 1$ if we have less heterozygotes than we would expect under HW.\n"
        "\n"
        "\n"
        "$$e_{IT}=\\frac{\\mbox{observed proportion of hets}}{\\mbox{expected proportion of hets}}=\\frac{ H_{obs}}{H_{IT}}$$\n"
        "\n"
        "We use $e_{IT}$ to define the statistic $F_{IT}$\n"
        "\n"
        "$$F_{IT}=1-e_{IT}$$\n"
        "\n"
        "So $F_{IT} > 0$ when we have a lack of heterozygotes and $F_{IT} < 0$ when we have an excess of heterozygotes. $F_{IT} = 0$ under random mating.\n"
        "\n"
        "When we have a subpropulation $S$ we can calculate the equivalent quantity but instead of considering heterozygosity in the whole population we only take a sub-population into account.\n"
        "\n"
        "$$e_{IS} = \\frac{H_{obs}}{H_{IS}}$$\n"
        "\n"
        "And lastly, we have $F_{ST}$. This one is not as intuitive to derive so I'm not including the derivation here. But basically it measure the excess heterozygosity in the total population due to the presence of two subpopulations with allele frequencies $p_{1}$ and $p_{2}$.\n"
        "\n"
        "$$F_{ST}= \\frac{(p_1-p_2)^2}{4 p (1-p)}$$"
    ));

    qDebug() << "command entry 33";
    testCommandEntry(entry, 33, QString::fromUtf8(
        "def F_statistics(total_pop, sub_pop_1, sub_pop_2):   \n"
        "    \"\"\"\n"
        "    Uses definitions above and allele counts from two sub-populations and a total population to compute F-statistics.\n"
        "    \"\"\"\n"
        "    #recall that the input dictionaries each contain a tuple in the form(observed, expected) for each genotype\n"
        "    f_IT = 1 - total_pop['het'][0] / (1. * total_pop['het'][1])\n"
        "    \n"
        "        \n"
        "    f_IS_1 = 1 - sub_pop_1['het'][0] / (1. * sub_pop_1['het'][1])\n"
        "    f_IS_2 = 1 - sub_pop_2['het'][0] / (1. * sub_pop_2['het'][1])    \n"
        "    \n"
        "    p1 = sub_pop_1['ref_counts'] / (1. * sub_pop_1['genotype_count'])\n"
        "    p2 = sub_pop_2['ref_counts'] / (1. * sub_pop_2['genotype_count'])\n"
        "    \n"
        "    p = total_pop['ref_counts'] / (1. * total_pop['genotype_count'])\n"
        "    \n"
        "    f_ST = ((p1 - p2) ** 2) / (4.0 * p * (1 - p)) \n"
        "    \n"
        "    F_dict = {\n"
        "        'f_IT': f_IT,\n"
        "        'f_IS_1': f_IS_1,\n"
        "        'f_IS_2': f_IS_2,\n"
        "        'f_ST': f_ST\n"
        "    }\n"
        "    \n"
        "    return F_dict"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Let's get some data for our F-tests. First we need to evolve two populations indepenently of each other, to simulate isolated mating. Then to simulate the total population we combine the two sub-populations. We then use our `allele_finder()` function to get all the alleles, and the `hardy_weinberg_chi_2_test()` function to get our expected and observed counts. Finally we plug those into the `f_statistics()` function."
    ));

    qDebug() << "command entry 34";
    testCommandEntry(entry, 34, QString::fromUtf8(
        "generation = -1\n"
        "\n"
        "#run two independent simulations\n"
        "sub_pop_1, sub_pop_1_stats= evolve_with_mating(\"(((....)))\", pop_size=1000, generations=15, beta=-1, mutation_rate=0.005)\n"
        "sub_pop_2, sub_pop_2_stats= evolve_with_mating(\"(((....)))\", pop_size=1000, generations=15, beta=-1, mutation_rate=0.005)"
    ));

    qDebug() << "command entry 35";
    testCommandEntry(entry, 35, 1, QString::fromUtf8(
        "#merge the two populations into a total population.\n"
        "total_pop = sub_pop_1[generation] + sub_pop_2[generation]\n"
        "\n"
        "\n"
        "#choose a reference and alternate allele\n"
        "ref_allele = \"A\"\n"
        "alt_allele = \"G\"\n"
        "\n"
        "#choose the position of the locus of interest.\n"
        "locus = 1\n"
        "\n"
        "#get list of alleles for each population\n"
        "total_pop_alleles = allele_finder(total_pop, locus, ref_allele, alt_allele)\n"
        "sub_pop_1_alleles = allele_finder(sub_pop_1[generation],locus, ref_allele, alt_allele)\n"
        "sub_pop_2_alleles = allele_finder(sub_pop_2[generation],locus, ref_allele, alt_allele)\n"
        "\n"
        "#get homo/het expectations using hardy weinberg function\n"
        "total_pop_counts = hardy_weinberg_chi2_test(total_pop_alleles)[1]\n"
        "sub_pop_1_counts = hardy_weinberg_chi2_test(sub_pop_1_alleles)[1]\n"
        "sub_pop_2_counts = hardy_weinberg_chi2_test(sub_pop_2_alleles)[1]\n"
        "\n"
        "#call f-statistics function\n"
        "f_statistics = F_statistics(total_pop_counts, sub_pop_1_counts, sub_pop_2_counts)\n"
        "print(f_statistics)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "{'f_IT': 0.054216581725422874, 'f_IS_1': 0.037559168553200184, 'f_IS_2': -0.08899167437557809, 'f_ST': -0.3885918781521351}"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Try playing with different evolution parameters and see the effect on the different F-statistics. This workshop is a work in progress so there may be some biases in our simulation scheme that can make for come confusing F-statistics. If you come up with anything interesting I would love to know about it."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Exercises / Extra Material\n"
        "\n"
        "### Programming Exercises\n"
        "\n"
        "i. *Heatmap of mutation rates vs. population sizes.* (short) \n"
        "\n"
        "Make a heatmap that plots the base pair distance of the average base pair distance of the population at generation `-1` for mutation rates $\\mu = \\{0, 0.001, 0.01, 0.1, 0.5\\}$ and population sizes $N=\\{10, 100, 1000, 10000\\}$. The resulting heatmap will be `5x4` dimensions. You may choose how many generations to evolve your populations, just plot the last one in the heatmap."
    ));

    qDebug() << "command entry 36";
    testCommandEntry(entry, 36, 2, QString::fromUtf8(
        "#lists of mutation rates and population sizes to test\n"
        "mutation_rates = [0, 0.001, 0.01, 0.1, 0.5]\n"
        "population_sizes = [10, 100, 1000, 10000]\n"
        "\n"
        "#number of generations to run each simulation\n"
        "generations = 1\n"
        "#target structure\n"
        "target = \"(.((....)))\"\n"
        "\n"
        "#list to store our results\n"
        "bp_distances = []\n"
        "\n"
        "#nested for loop to go through each combination of mutation rates and population sizes.\n"
        "for m in mutation_rates:\n"
        "    #list to store the population size results for current mutation rate.\n"
        "    bp_distances_by_pop_size = []\n"
        "    #try each population size\n"
        "    for p in population_sizes:\n"
        "        #call evolve() with m and p \n"
        "        pop, pop_stats = evolve(target, mutation_rate=m, pop_size=p, generations=generations)\n"
        "        #add bp_distance of chromosome 1 at generation -1 (last generation) to bp_distances_by_pop_size\n"
        "        bp_distances_by_pop_size.append(pop_stats['mean_bp_distance_1'][-1])\n"
        "    #add to global list once all combinations of current mutation rate and population sizes.\n"
        "    bp_distances.append(bp_distances_by_pop_size)\n"
        "    \n"
        "#use bp_distances matrxi to make a heatmap\n"
        "sns.heatmap(bp_distances)\n"
        "\n"
        "#labels\n"
        "plt.xlabel(\"Population Size\")\n"
        "#xticks/yticks takes a list of numbers that specify the position of the ticks and a list with the tick labels\n"
        "plt.xticks([i + .5 for i in range(len(population_sizes))], population_sizes)\n"
        "plt.ylabel(\"Mutation Rate\")\n"
        "plt.yticks([i + .5 for i in range(len(mutation_rates))], mutation_rates)"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "([<matplotlib.axis.YTick at 0x7fa02ba18ac8>,\n"
        "  <matplotlib.axis.YTick at 0x7fa02ba183c8>,\n"
        "  <matplotlib.axis.YTick at 0x7fa02ba59a58>,\n"
        "  <matplotlib.axis.YTick at 0x7fa02acbe978>,\n"
        "  <matplotlib.axis.YTick at 0x7fa02acc5048>],\n"
        " <a list of 5 Text yticklabel objects>)"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "ii. *Introduce mating preferences within a population.* (medium length) \n"
        "\n"
        "Modify the `selection_with_mating()` function to allow for mating preferences within a population. In our example above we were just running two independent simulations to study barriers to gene flow. But now you will implement mating preferences within a single simulation. Your function will assign each Cell a new attribute called `self.preference` which will take a string value denoting the mating type the current cell prefers to mate with. For example we can have a population with three mating types: $\\{A, B, C\\}$. Your function will randomly assign preferences to each cell in the initial population. We will define a preference between types $A$ and $B$ as the probability that two cells of those given types will mate if selected. \n"
        "\n"
        "$$\n"
        "preferences(A,B,C) = \n"
        "\\begin{bmatrix}\n"
        "   0.7 & 0.1 & 0.2 \\\\\n"
        "   0.1 & 0.9 & 0   \\\\\n"
        "   0.2 & 0   & 0.8 \\\\\n"
        "\\end{bmatrix}\n"
        "$$\n"
        "\n"
        "Once you selected two potential parents for mating (as we did earlier) you will use the matrix to evaluate whether or not the two parents will mate and contribute an offspring to the next generation. "
    ));

    qDebug() << "command entry 37";
    testCommandEntry(entry, 37, 1, QString::fromUtf8(
        "def populate_with_preferences(target, preference_types, pop_size=100):\n"
        "    \n"
        "    population = []\n"
        "\n"
        "    for i in range(pop_size):\n"
        "        #get a random sequence to start with\n"
        "        sequence = \"\".join([random.choice(\"AUCG\") for _ in range(len(target))])\n"
        "        #use nussinov to get the secondary structure for the sequence\n"
        "        structure = nussinov(sequence)\n"
        "        #add a new Cell object to the population list\n"
        "        new_cell = Cell(sequence, structure, sequence, structure)\n"
        "        new_cell.id = i\n"
        "        new_cell.parent = i\n"
        "        \n"
        "        #assign preference\n"
        "        new_cell.preference = random.choice(preference_types)\n"
        "        population.append(new_cell)\n"
        "            \n"
        "    return population\n"
        "\n"
        "def selection_with_mating_preference(population, target, preference_matrix, preference_types, mutation_rate=0.001, beta=-2):\n"
        "    next_generation = []\n"
        "    \n"
        "    counter = 0\n"
        "    while len(next_generation) < len(population):\n"
        "        #select two parents based on their fitness\n"
        "        parents_pair = np.random.choice(population, 2, p=[rna.fitness for rna in population], replace=False)\n"
        "        \n"
        "        #look up probabilty of mating in the preference_matrix\n"
        "        mating_probability = preference_matrix[parents_pair[0].preference][parents_pair[1].preference]\n"
        "        \n"
        "        r = random.random()\n"
        "        #if random number below mating_probability, mate the Cells as before\n"
        "        if r < mating_probability:\n"
        "            #take the sequence and structure from the first parent's first chromosome and give it to the child\n"
        "            child_chrom_1 = (parents_pair[0].sequence_1, parents_pair[0].structure_1)\n"
        "\n"
        "            #do the same for the child's second chromosome and the second parent.\n"
        "            child_chrom_2 = (parents_pair[1].sequence_2, parents_pair[1].structure_2)\n"
        "\n"
        "\n"
        "            #initialize the new child Cell witht he new chromosomes.\n"
        "            child_cell = Cell(child_chrom_1[0], child_chrom_1[1], child_chrom_2[0], child_chrom_2[1])\n"
        "\n"
        "            #give the child and id and store who its parents are\n"
        "            child_cell.id = counter\n"
        "            child_cell.parent_1 = parents_pair[0].id\n"
        "            child_cell.parent_2 = parents_pair[1].id\n"
        "            \n"
        "            #give the child a random preference\n"
        "            child_cell.preference = random.choice(preference_types)\n"
        "\n"
        "            #add the child to the new generation\n"
        "            next_generation.append(child_cell)\n"
        "\n"
        "            counter = counter + 1\n"
        "            \n"
        "        \n"
        "    #introduce mutations in next_generation sequeneces and re-fold when a mutation occurs (same as before)\n"
        "    for rna in next_generation:      \n"
        "        mutated_sequence_1, mutated_1 = mutate(rna.sequence_1, mutation_rate=mutation_rate)\n"
        "        mutated_sequence_2, mutated_2 = mutate(rna.sequence_2, mutation_rate=mutation_rate)\n"
        "\n"
        "        if mutated_1:\n"
        "            rna.sequence_1 = mutated_sequence_1\n"
        "            rna.structure_1 = nussinov(mutated_sequence_1)\n"
        "        if mutated_2:\n"
        "            rna.sequence_2 = mutated_sequence_2\n"
        "            rna.structure_2 = nussinov(mutated_sequence_2)\n"
        "        else:\n"
        "            continue\n"
        "\n"
        "    #update fitness values for the new generation\n"
        "    compute_fitness(next_generation, target, beta=beta)\n"
        "\n"
        "    return next_generation    \n"
        "\n"
        "\n"
        "def evolve_with_mating_preferences(target, preference_types, preference_matrix,\\\n"
        "                                   generations=10, pop_size=100, mutation_rate=0.001, beta=-2):\n"
        "    populations = []\n"
        "    population_stats = {}\n"
        "    \n"
        "    initial_population = populate_with_preferences(target, preference_types, pop_size=pop_size)\n"
        "    compute_fitness(initial_population, target)\n"
        "        \n"
        "    current_generation = initial_population\n"
        "\n"
        "    #iterate the selection process over the desired number of generations\n"
        "    for i in range(generations):\n"
        "        #let's get some stats on the structures in the populations   \n"
        "        record_stats(current_generation, population_stats)\n"
        "        \n"
        "        #add the current generation to our list of populations.\n"
        "        populations.append(current_generation)\n"
        "\n"
        "        #select the next generation, but this time with mutations\n"
        "        new_gen = selection_with_mating_preference(current_generation, target, preference_matrix, \\\n"
        "                                                   preference_types, mutation_rate=mutation_rate, beta=beta)\n"
        "        current_generation = new_gen \n"
        "    \n"
        "    return (populations, population_stats)\n"
        "\n"
        "\n"
        "\n"
        "#run a small test to make sure it works\n"
        "target = \".(((....)))\"\n"
        "#for convenience, let's give the preference types integer values in sequential order\n"
        "preference_types = [0,1,2]\n"
        "\n"
        "preference_matrix = np.array([[0.7, 0.1, 0.2],[0.1, 0.9, 0],[0.2, 0, 0.8]])\n"
        "    \n"
        "pops, pop_stats = evolve_with_mating_preferences(target, preference_types, preference_matrix)\n"
        "\n"
        "for cell in pops[-1][:10]:\n"
        "    print(cell.sequence_1)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "UAUCUCUAGAA\n"
        "UCAAACGGUUU\n"
        "CCUAGACUUUC\n"
        "UAUCUCUAGAA\n"
        "CCUAGACUUUC\n"
        "GGCAaUGGUGC\n"
        "GGCAaUGGUGC\n"
        "CGGUGCCAUGG\n"
        "CCCGGUUACGU\n"
        "CGGGGAGUUUU"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "### Population Genetics / Bioinformatics Exercises\n"
        "\n"
        "*Exercise 1.  Make a tree using maximum parsimony.*\n"
        "\n"
        "We saw how to make trees using a distance score. Another popular method is known as the maximum parsimony approach. I won't go into too much detail on this since we are short on time, but I will give a quick intro and we'll look at how ot make a tree using maximum parsimony.\n"
        "\n"
        "This approach is based on the principle of parsimony, which states that the simplest explanation for our data is the most likely to be true. So given an alignment, we assume that the best tree is the one that minimizes the number of changes, or mutations. This is often a reasonable assumption to make since mutation rates in real populations are generally low, and things like back-mutations (e.g. A --> C --> A) are unlikely. Computing the tree that that maximizes parsimony directly is a difficult task, but evaluating the parsimony score of a tree given the tree is easy. So this approach basically generates many random trees for the data and scores them based on parsimony keeping the most parsimonious tree. Take a look at [the biopython manual to work through this example](http://biopython.org/wiki/Phylo), and [this one](http://biopython.org/DIST/docs/api/Bio.Phylo.TreeConstruction.ParsimonyTreeConstructor-class.html).\n"
        "\n"
        "Since we already have an alignment (`aln.clustal`) we will just re-use it and make a maximum parsimony tree instead. "
    ));

    qDebug() << "command entry 38";
    testCommandEntry(entry, 38, 1, QString::fromUtf8(
        "from Bio.Phylo.TreeConstruction import *\n"
        "\n"
        "#open our alignment file (or make a new one if you want)\n"
        "with open('aln.clustal', 'r') as align:\n"
        "    aln = AlignIO.read(align, 'clustal')\n"
        "\n"
        "#create a parsimony scorer object\n"
        "scorer = ParsimonyScorer()\n"
        "#the searcher object will search through possible trees and score them.\n"
        "searcher = NNITreeSearcher(scorer)\n"
        "\n"
        "#takes our searcher object and a seed tree (upgma_tree) to find the best tree\n"
        "constructor = ParsimonyTreeConstructor(searcher, upgma_tree)\n"
        "\n"
        "#build the tree \n"
        "parsimony_tree = constructor.build_tree(aln)\n"
        "\n"
        "#draw the tree\n"
        "Phylo.draw(parsimony_tree)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "*Exercise 2. Bootstrapping*\n"
        "\n"
        "We just saw two methods of growing phylogenetic trees given an alignment. However, as we saw with the maximum parsimony approach, there can be many different trees for a single data set. How do we know our tree is a good representation of the data? By 'good' here we will instead use the word 'robust'. Is the tree we use too sensitive to the particularities of the data we gave it? If we make a small change in the sequence will we get a very different tree? Normally these problems would be addressed by re-sampling and seeing if we obtain similar results. But we can't really re-sample evolution. It happened once and we can't make it happen again. So we use something called *bootstrapping* which is a technique often used in statistics where instead of generating new data, you re-sample from your present data.\n"
        "\n"
        "So we have a multiple sequence alignment with $M$ sequences (rows) each with sequences of length $N$ nucleotides (columns). For each row, we can randomly sample $N$ nucleotides with replacement to make a new 'bootstrapped' sequence also of length $N$. Think of it as a kind of shuffling of the data. This gives us a whole new alignment that we can again use to make a new tree.\n"
        "\n"
        "This process is repeated many times to obtain many trees. The differences in topology (shape/structure) of the trees we obtained are assessed. If after this shuffling/perturbations we still get similar enough looking trees we can say that our final tree is robust to small changes in the data. ([some more reading on this](http://projecteuclid.org/download/pdf_1/euclid.ss/1063994979))\n"
        "\n"
        "Let's run a small example of this using the bootstrapping functions in `BioPython`."
    ));

    qDebug() << "command entry 39";
    testCommandEntry(entry, 39, 1, QString::fromUtf8(
        "from Bio.Phylo.Consensus import *\n"
        "\n"
        "#open our alignment file.\n"
        "with open('aln.clustal', 'r') as align:\n"
        "    aln = AlignIO.read(align, 'clustal')\n"
        "\n"
        "#take 5 bootstrap samples from our alignment\n"
        "bootstraps = bootstrap(aln,5)\n"
        "\n"
        "#let's print each new alignment in clustal format. you should see 5 different alignments.\n"
        "for b in bootstraps:\n"
        "    print(b.format('clustal'))"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "CLUSTAL X (1.81) multiple sequence alignment\n"
        "\n"
        "\n"
        "8                                   GAAGAGACAC\n"
        "7                                   GGGGGAGCGC\n"
        "0                                   GCCACAGCGC\n"
        "3                                   ACCACAGUGU\n"
        "6                                   CCCACAGUGU\n"
        "9                                   CCCACUAGAG\n"
        "1                                   CCCUCUCGCG\n"
        "2                                   CCCUCUCGCG\n"
        "4                                   CUUUUUCGCG\n"
        "5                                   CUUUUUCGCG\n"
        "                                              \n"
        "\n"
        "\n"
        "\n"
        "CLUSTAL X (1.81) multiple sequence alignment\n"
        "\n"
        "\n"
        "8                                   GGAUUUCUUA\n"
        "7                                   GAGCCCCCCG\n"
        "0                                   AAGGGGCGGG\n"
        "3                                   AAGGGGUGGG\n"
        "6                                   AAGGGGUGGG\n"
        "9                                   AUAUUUGUUA\n"
        "1                                   UUCUUUGUUC\n"
        "2                                   UUCUUUGUUC\n"
        "4                                   UUCUUUGUUC\n"
        "5                                   UUCUUUGUUC\n"
        "                                              \n"
        "\n"
        "\n"
        "\n"
        "CLUSTAL X (1.81) multiple sequence alignment\n"
        "\n"
        "\n"
        "8                                   UCUUGAGAUC\n"
        "7                                   CCCGGGGGGC\n"
        "0                                   GCGAGGGCAC\n"
        "3                                   GUGCAGACCU\n"
        "6                                   GUGCCGCCCU\n"
        "9                                   UGUACACCAG\n"
        "1                                   UGUGCCCCGG\n"
        "2                                   UGUGCCCCGG\n"
        "4                                   UGUGCCCUGG\n"
        "5                                   UGUGCCCUGG\n"
        "                                              \n"
        "\n"
        "\n"
        "\n"
        "CLUSTAL X (1.81) multiple sequence alignment\n"
        "\n"
        "\n"
        "8                                   GGUGCGCAUG\n"
        "7                                   GACGCAUGGG\n"
        "0                                   AAGGUAAGAA\n"
        "3                                   AAGAUAUGCA\n"
        "6                                   AAGCUAUGCA\n"
        "9                                   AUUCUUAAAA\n"
        "1                                   UUUCAUACGU\n"
        "2                                   UUUCAUACGU\n"
        "4                                   UUUCAUACGU\n"
        "5                                   UUUCAUACGU\n"
        "                                              \n"
        "\n"
        "\n"
        "\n"
        "CLUSTAL X (1.81) multiple sequence alignment\n"
        "\n"
        "\n"
        "8                                   GCUAGGAACC\n"
        "7                                   GCGGGGGGCU\n"
        "0                                   AUACGGGGCA\n"
        "3                                   AUCCAAGGUU\n"
        "6                                   AUCCCCGGUU\n"
        "9                                   AUACCCAAGA\n"
        "1                                   UAGCCCCCGA\n"
        "2                                   UAGCCCCCGA\n"
        "4                                   UAGUCCCCGA\n"
        "5                                   UAGUCCCCGA"
    ));
    entry = entry->next();

    qDebug() << "command entry 40";
    testCommandEntry(entry, 40, 5, QString::fromUtf8(
        "#now we want to use the bootstrapping to make new trees based on the new samples. we'll go back to making UPGMA trees.\n"
        "\n"
        "#start a calculator that uses sequence identity to calculate differences\n"
        "calculator = DistanceCalculator('identity')\n"
        "#start a distance tree constructor object \n"
        "constructor = DistanceTreeConstructor(calculator)\n"
        "#generate 5 bootstrap UPGMA trees\n"
        "trees =  bootstrap_trees(aln, 5, constructor)\n"
        "\n"
        "#let's look at the trees. (if you have few samples, short sequences the trees might look very similar)\n"
        "for t in trees:\n"
        "    Phylo.draw(t)"
    ));
    testImageResult(entry, 0);
    testImageResult(entry, 1);
    testImageResult(entry, 2);
    testImageResult(entry, 3);
    testImageResult(entry, 4);
    entry = entry->next();

    qDebug() << "command entry 41";
    testCommandEntry(entry, 41, 1, QString::fromUtf8(
        "#biopython gives us a useful function that puts all this together by bootstrapping trees and making a 'consensus' tree.\n"
        "consensus_tree = bootstrap_consensus(aln, 100, constructor, majority_consensus)\n"
        "Phylo.draw(consensus_tree)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "*Exercise 3. T-tests*\n"
        "\n"
        "Similarly to the $\\chi^{2}$ test we saw for testing deviations from HW equilibrium, we can use a T-test to compare differences in means between two independent samples. We can use this to revisit a the first programming question in the exercsies section. Does mutation rate and population size have an effect on the fitness of populations? We can translate this question to, is there a difference in the mean base pair distance between populations under different mutation and population size regimes?\n"
        "\n"
        "Scipy has a very useful function that implements the T-test called `scipy.stats.ttest_ind`. Run two independent simulations (with different mutation rates) and compute the difference in mean bp distance between the two at their final generation. Store the populations in two different variables. Give a list of `bp_distance_1` values for each memeber of the population to `ttest_ind()`. \n"
        "\n"
        "Make sure to read teh `ttest_ind()` documentation, particularly about the argumetn `equal_var`. What should we set it to?"
    ));

    qDebug() << "command entry 42";
    testCommandEntry(entry, 42, 1, QString::fromUtf8(
        "import collections\n"
        "\n"
        "target = \"..(((....).))\"\n"
        "\n"
        "#run two simulations\n"
        "hi_mut_pop, hi_mut_stats = evolve(target, generations=5, pop_size=1000, mutation_rate=0.5)\n"
        "lo_mut_pop, hi_mut_stats = evolve(target, generations=5, pop_size=1000, mutation_rate=0.05)\n"
        "\n"
        "#store lits of base pair distances for each population at last generation.\n"
        "hi_bps = [p.bp_distance_1 for p in hi_mut_pop[-1]]\n"
        "lo_bps = [p.bp_distance_1 for p in lo_mut_pop[-1]]\n"
        "\n"
        "#run the \n"
        "stats.ttest_ind(hi_bps, lo_bps, equal_var=False)"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "Ttest_indResult(statistic=1.3671266704990508, pvalue=0.17188793847221653)"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "### Bonus! (difficult programming exercise)\n"
        "1. *Nussinov Algorithm (Only try this if you are feeling brave and are done with the other exercises or are interested in getting a taste of Computer Science. It is beyond the scope of this workshop.)*\n"
        "\n"
        "There are several approaches for solving this problem, we will look at the simplest one here which is known as the Nussinov Algorithm. This algorithm is a popular example of a class of algorithms know as dynamic programming algorithms. The main idea behind these algorithms is that we can break down the problem into many subproblems which are easier to compute than the full problem. Once we have obtained the solution for the subproblems, we can retrieve the solution to the full problem by doing something called a backtrace (more on the backtrace later). \n"
        "\n"
        "Here, the problem is obtaining the optimal pairing on a string of nucleotides. In order to know how good our structure is, we assign a score to it. One possible scoring scheme could be adding 1 to the score per paired set of nucleotides, and 0 otherwise. So in other words, we want a pairing that will give us the highest possible score. We can write this quantity as $OPT(i, j)$ where $i$ and $j$ are the indices of the sequence between which we obtain the pairing score. Our algorithm is therefore going to compute a folding score for all substrings bound by $i$ and $j$ and store the value in what is known as a dynamic programming table. Our dynamic programming table will be a $N$ x $N$ array where $N$ is the length of our sequence. So now that we have a way of measuring how good a structure is, we need a way to evaluate scores given a subsequence. To do this, we set some rules on the structure of an RNA sequence:\n"
        "\n"
        "\n"
        "If $i$ and $j$ form a pair:\n"
        "1. The pair $i$ and $j$ must form a valid watson-crick pair.\n"
        "2. $i < j-4$. This ensures that bonding is not happening between positions that are too close to each other, which would produce steric clashes.\n"
        "3. If pair $(i,j)$ and $(k, l)$ are in the structure, then $i < k < j < l$. This ensures that there is no crossing over of pairs which would result in pseudoknots.\n"
        "4. No base appears in more than one pair.\n"
        "\n"
        "Using these rules we can begin to build our algorithm. The first part of our algorithm needs to take as input indices $i$ and $j$ and return the value $OPT(i,j)$ which is the optimal score of a structure between $i$ and $j$. We start by thinking about values of $i$ and $j$ for which we can immediately know the solution, this is known as a 'base case'.  This is a case where the solution is known and no further recursion is required. Once the algorithm reaches the base case, it can return a solution and propagate it upward to the first recursive call. So once we have reached $i$ and $j$ that are too close to form a structure (rule number 2), we know that the score is 0. \n"
        "\n"
        "Otherwise, we must weigh the possibility of forming a pair or not forming a pair. If $i$ and $j$ are unpaired, then $OPT(i,j)$ is just $OPT(i, j-1)$ since the score will not increase for unpaired indices. \n"
        "\n"
        "The other case is that $i$ is paired to some index $t$ on the interval $[i,j]$. We then add 1 to the score and consider the structure formed before and after the pairing between $i$ and $t$. We can write these two cases as $OPT(i, t-1)$ and $OPT(t+1, j)$. But how do we know which $t$ to pair $i$ with? Well we simply try all possible values of $t$ within the allowed range and choose the best one. \n"
        "\n"
        "All of this can be summed up as follows:\n"
        "\n"
        "$$ OPT(i,j) = max\\begin{cases}\n"
        "                OPT(i, j-1) \\quad \\text{If $i$ and $j$ are not paired with each other.}\\\\\n"
        "                max(1 + OPT(i, t-1) + OPT(t+1, j)) \\quad \\text{Where we try all values of $t$ < j - 4}\n"
        "                \\end{cases}$$\n"
        "\n"
        "\n"
        "We can now use this recursion to fill our dynamic programming table. Once we have filled the table with scores, we can retrieve the optimal folding by a process called backtracking. We won't go into detail on how this works, but the main idea is that we can start by looking at the entry containing the score for the full sequence $OPT[0][N]$. We can then look at adjacent entries and deduce which case (pairing or not pairing) resulted in the current value. We can continue like this for the full table until we have retrieved the full structure."
    ));

    qDebug() << "command entry 43";
    testCommandEntry(entry, 43, 1, QString::fromUtf8(
        "min_loop_length = 4\n"
        "\n"
        "def pair_check(tup):\n"
        "    if tup in [('A', 'U'), ('U', 'A'), ('C', 'G'), ('G', 'C')]:\n"
        "        return True\n"
        "    return False\n"
        "\n"
        "def OPT(i,j, sequence):\n"
        "    \"\"\" returns the score of the optimal pairing between indices i and j\"\"\"\n"
        "    #base case: no pairs allowed when i and j are less than 4 bases apart\n"
        "    if i >= j-min_loop_length:\n"
        "        return 0\n"
        "    else:\n"
        "        #i and j can either be paired or not be paired, if not paired then the optimal score is OPT(i,j-1)\n"
        "        unpaired = OPT(i, j-1, sequence)\n"
        "\n"
        "        #check if j can be involved in a pairing with a position t\n"
        "        pairing = [1 + OPT(i, t-1, sequence) + OPT(t+1, j-1, sequence) for t in range(i, j-4)\\\n"
        "                   if pair_check((sequence[t], sequence[j]))]\n"
        "        if not pairing:\n"
        "            pairing = [0]\n"
        "        paired = max(pairing)\n"
        "\n"
        "\n"
        "        return max(unpaired, paired)\n"
        "\n"
        "\n"
        "def traceback(i, j, structure, DP, sequence):\n"
        "    #in this case we've gone through the whole sequence. Nothing to do.\n"
        "    if j <= i:\n"
        "        return\n"
        "    #if j is unpaired, there will be no change in score when we take it out, so we just recurse to the next index\n"
        "    elif DP[i][j] == DP[i][j-1]:\n"
        "        traceback(i, j-1, structure, DP, sequence)\n"
        "    #hi\n"
        "    else:\n"
        "        #try pairing j with a matching index k to its left.\n"
        "        for k in [b for b in range(i, j-min_loop_length) if pair_check((sequence[b], sequence[j]))]:\n"
        "            #if the score at i,j is the result of adding 1 from pairing (j,k) and whatever score\n"
        "            #comes from the substructure to its left (i, k-1) and to its right (k+1, j-1)\n"
        "            if k-1 < 0:\n"
        "                if DP[i][j] == DP[k+1][j-1] + 1:\n"
        "                    structure.append((k,j))\n"
        "                    traceback(k+1, j-1, structure, DP, sequence)\n"
        "                    break\n"
        "            elif DP[i][j] == DP[i][k-1] + DP[k+1][j-1] + 1:\n"
        "                #add the pair (j,k) to our list of pairs\n"
        "                structure.append((k,j))\n"
        "                #move the recursion to the two substructures formed by this pairing\n"
        "                traceback(i, k-1, structure, DP, sequence)\n"
        "                traceback(k+1, j-1, structure, DP, sequence)\n"
        "                break\n"
        "\n"
        "def write_structure(sequence, structure):\n"
        "    dot_bracket = [\".\" for _ in range(len(sequence))]\n"
        "    for s in structure:\n"
        "        dot_bracket[min(s)] = \"(\"\n"
        "        dot_bracket[max(s)] = \")\"\n"
        "    return \"\".join(dot_bracket)\n"
        "\n"
        "\n"
        "#initialize matrix with zeros where can't have pairings\n"
        "def initialize(N):\n"
        "    #NxN matrix that stores the scores of the optimal pairings.\n"
        "    DP = np.empty((N,N))\n"
        "    DP[:] = np.NAN\n"
        "    for k in range(0, min_loop_length):\n"
        "        for i in range(N-k):\n"
        "            j = i + k\n"
        "            DP[i][j] = 0\n"
        "    return DP\n"
        "\n"
        "def nussinov(sequence):\n"
        "    N = len(sequence)\n"
        "    DP = initialize(N)\n"
        "    structure = []\n"
        "\n"
        "    #fill the DP matrix\n"
        "    for k in range(min_loop_length, N):\n"
        "        for i in range(N-k):\n"
        "            j = i + k\n"
        "            DP[i][j] = OPT(i,j, sequence)\n"
        "\n"
        "    #copy values to lower triangle to avoid null references\n"
        "    for i in range(N):\n"
        "        for j in range(0, i):\n"
        "            DP[i][j] = DP[j][i]\n"
        "\n"
        "\n"
        "    traceback(0,N-1, structure, DP, sequence)\n"
        "    return (sequence, write_structure(sequence, structure))\n"
        "\n"
        "print(nussinov(\"ACCCGAUGUUAUAUAUACCU\"))"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "('ACCCGAUGUUAUAUAUACCU', '(...(..(((....).))))')"
    ));
    entry = entry->next();

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testJupyter4()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("A Reaction-Diffusion Equation Solver in Python with Numpy.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python2"));

    WorksheetEntry* entry = w->firstEntry();

    testMarkdown(entry, QString::fromUtf8(
        "This notebook demonstrates how IPython notebooks can be used to discuss the theory and implementation of numerical algorithms on one page.\n"
        "\n"
        "With `ipython nbconvert --to markdown name.ipynb` a notebook like this one can be made into a \n"
        "[blog post](http://georg.io/2013/12/Crank_Nicolson) in one easy step. To display the graphics in your resultant blog post use,\n"
        "for instance, your [Dropbox Public folder](https://www.dropbox.com/help/16/en) that you can \n"
        "[activate here](https://www.dropbox.com/enable_public_folder)."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "# The Crank-Nicolson Method"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "The [Crank-Nicolson method](http://en.wikipedia.org/wiki/Crank%E2%80%93Nicolson_method) is a well-known finite difference method for the\n"
        "numerical integration of the heat equation and closely related partial differential equations.\n"
        "\n"
        "We often resort to a Crank-Nicolson (CN) scheme when we integrate numerically reaction-diffusion systems in one space dimension\n"
        "\n"
        "$$\\frac{\\partial u}{\\partial t} = D \\frac{\\partial^2 u}{\\partial x^2} + f(u),$$\n"
        "\n"
        "$$\\frac{\\partial u}{\\partial x}\\Bigg|_{x = 0, L} = 0,$$\n"
        "\n"
        "where $u$ is our concentration variable, $x$ is the space variable, $D$ is the diffusion coefficient of $u$, $f$ is the reaction term,\n"
        "and $L$ is the length of our one-dimensional space domain.\n"
        "\n"
        "Note that we use [Neumann boundary conditions](http://en.wikipedia.org/wiki/Neumann_boundary_condition) and specify that the solution\n"
        "$u$ has zero space slope at the boundaries, effectively prohibiting entrance or exit of material at the boundaries (no-flux boundary conditions)."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Finite Difference Methods"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Many fantastic textbooks and tutorials have been written about finite difference methods, for instance a free textbook by\n"
        "[Lloyd Trefethen](http://people.maths.ox.ac.uk/trefethen/pdetext.html).\n"
        "\n"
        "Here we describe a few basic aspects of finite difference methods.\n"
        "\n"
        "The above reaction-diffusion equation describes the time evolution of variable $u(x,t)$ in one space dimension ($u$ is a line concentration).\n"
        "If we knew an analytic expression for $u(x,t)$ then we could plot $u$ in a two-dimensional coordinate system with axes $t$ and $x$.\n"
        "\n"
        "To approximate $u(x,t)$ numerically we discretize this two-dimensional coordinate system resulting, in the simplest case, in a\n"
        "two-dimensional [regular grid](http://en.wikipedia.org/wiki/Regular_grid).\n"
        "This picture is employed commonly when constructing finite differences methods, see for instance \n"
        "[Figure 3.2.1 of Trefethen](http://people.maths.ox.ac.uk/trefethen/3all.pdf).\n"
        "\n"
        "Let us discretize both time and space as follows:\n"
        "\n"
        "$$t_n = n \\Delta t,~ n = 0, \\ldots, N-1,$$\n"
        "\n"
        "$$x_j = j \\Delta x,~ j = 0, \\ldots, J-1,$$\n"
        "\n"
        "where $N$ and $J$ are the number of discrete time and space points in our grid respectively.\n"
        "$\\Delta t$ and $\\Delta x$ are the time step and space step respectively and defined as follows:\n"
        "\n"
        "$$\\Delta t = T / N,$$\n"
        "\n"
        "$$\\Delta x = L / J,$$\n"
        "\n"
        "where $T$ is the point in time up to which we will integrate $u$ numerically.\n"
        "\n"
        "Our ultimate goal is to construct a numerical method that allows us to approximate the unknonwn analytic solution $u(x,t)$\n"
        "reasonably well in these discrete grid points.\n"
        "\n"
        "That is we want construct a method that computes values $U(j \\Delta x, n \\Delta t)$ (note: capital $U$) so that\n"
        "\n"
        "$$U(j \\Delta x, n \\Delta t) \\approx u(j \\Delta x, n \\Delta t)$$\n"
        "\n"
        "As a shorthand we will write $U_j^n = U(j \\Delta x, n \\Delta t)$ and $(j,n)$ to refer to grid point $(j \\Delta x, n \\Delta t)$."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## The Crank-Nicolson Stencil"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Based on the two-dimensional grid we construct we then approximate the operators of our reaction-diffusion system.\n"
        "\n"
        "For instance, to approximate the time derivative on the left-hand side in grid point $(j,n)$ we use the values of $U$ in two specific grid points:\n"
        "\n"
        "$$\\frac{\\partial u}{\\partial t}\\Bigg|_{x = j \\Delta x, t = n \\Delta t} \\approx \\frac{U_j^{n+1} - U_j^n}{\\Delta t}.$$\n"
        "\n"
        "We can think of this scheme as a stencil that we superimpose on our $(x,t)$-grid and this particular stencil is\n"
        "commonly referred to as [forward difference](http://en.wikipedia.org/wiki/Finite_difference#Forward.2C_backward.2C_and_central_differences).\n"
        "\n"
        "The spatial part of the [Crank-Nicolson stencil](http://journals.cambridge.org/abstract_S0305004100023197)\n"
        "(or see [Table 3.2.2 of Trefethen](http://people.maths.ox.ac.uk/trefethen/3all.pdf))\n"
        "for the heat equation ($u_t = u_{xx}$) approximates the \n"
        "[Laplace operator](http://en.wikipedia.org/wiki/Laplace_operator) of our equation and takes the following form\n"
        "\n"
        "$$\\frac{\\partial^2 u}{\\partial x^2}\\Bigg|_{x = j \\Delta x, t = n \\Delta t} \\approx \\frac{1}{2 \\Delta x^2} \\left( U_{j+1}^n - 2 U_j^n + U_{j-1}^n + U_{j+1}^{n+1} - 2 U_j^{n+1} + U_{j-1}^{n+1}\\right).$$\n"
        "\n"
        "To approximate $f(u(j \\Delta x, n \\Delta t))$ we write simply $f(U_j^n)$.\n"
        "\n"
        "These approximations define the stencil for our numerical method as pictured on [Wikipedia](http://en.wikipedia.org/wiki/Crank%E2%80%93Nicolson_method).\n"
        "\n"
        "![SVG](https://dl.dropboxusercontent.com/u/129945779/georgio/CN-stencil.svg)\n"
        "\n"
        "Applying this stencil to grid point $(j,n)$ gives us the following approximation of our reaction-diffusion equation:\n"
        "\n"
        "$$\\frac{U_j^{n+1} - U_j^n}{\\Delta t} = \\frac{D}{2 \\Delta x^2} \\left( U_{j+1}^n - 2 U_j^n + U_{j-1}^n + U_{j+1}^{n+1} - 2 U_j^{n+1} + U_{j-1}^{n+1}\\right) + f(U_j^n).$$"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Reordering Stencil into Linear System"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Let us define $\\sigma = \\frac{D \\Delta t}{2 \\Delta x^2}$ and reorder the above approximation of our reaction-diffusion equation:\n"
        "\n"
        "$$-\\sigma U_{j-1}^{n+1} + (1+2\\sigma) U_j^{n+1} -\\sigma U_{j+1}^{n+1} = \\sigma U_{j-1}^n + (1-2\\sigma) U_j^n + \\sigma U_{j+1}^n + \\Delta t f(U_j^n).$$\n"
        "\n"
        "This equation makes sense for space indices $j = 1,\\ldots,J-2$ but it does not make sense for indices $j=0$ and $j=J-1$ (on the boundaries):\n"
        "\n"
        "$$j=0:~-\\sigma U_{-1}^{n+1} + (1+2\\sigma) U_0^{n+1} -\\sigma U_{1}^{n+1} = \\sigma U_{-1}^n + (1-2\\sigma) U_0^n + \\sigma U_{1}^n + \\Delta t f(U_0^n),$$\n"
        "\n"
        "$$j=J-1:~-\\sigma U_{J-2}^{n+1} + (1+2\\sigma) U_{J-1}^{n+1} -\\sigma U_{J}^{n+1} = \\sigma U_{J-2}^n + (1-2\\sigma) U_{J-1}^n + \\sigma U_{J}^n + \\Delta t f(U_{J-1}^n).$$\n"
        "\n"
        "The problem here is that the values $U_{-1}^n$ and $U_J^n$ lie outside our grid.\n"
        "\n"
        "However, we can work out what these values should equal by considering our Neumann boundary condition.\n"
        "Let us discretize our boundary condition at $j=0$ with the \n"
        "[backward difference](http://en.wikipedia.org/wiki/Finite_difference#Forward.2C_backward.2C_and_central_differences) and\n"
        "at $j=J-1$ with the\n"
        "[forward difference](http://en.wikipedia.org/wiki/Finite_difference#Forward.2C_backward.2C_and_central_differences):\n"
        "\n"
        "$$\\frac{U_1^n - U_0^n}{\\Delta x} = 0,$$\n"
        "\n"
        "$$\\frac{U_J^n - U_{J-1}^n}{\\Delta x} = 0.$$\n"
        "\n"
        "These two equations make it clear that we need to amend our above numerical approximation for\n"
        "$j=0$ with the identities $U_0^n = U_1^n$ and $U_0^{n+1} = U_1^{n+1}$, and\n"
        "for $j=J-1$ with the identities $U_{J-1}^n = U_J^n$ and $U_{J-1}^{n+1} = U_J^{n+1}$.\n"
        "\n"
        "Let us reinterpret our numerical approximation of the line concentration of $u$ in a fixed point in time as a vector $\\mathbf{U}^n$:\n"
        "\n"
        "$$\\mathbf{U}^n = \n"
        "\\begin{bmatrix} U_0^n \\\\ \\vdots \\\\ U_{J-1}^n \\end{bmatrix}.$$\n"
        "\n"
        "Using this notation we can now write our above approximation for a fixed point in time, $t = n \\Delta t$, compactly as a linear system:\n"
        "\n"
        "$$\n"
        "\\begin{bmatrix}\n"
        "1+\\sigma & -\\sigma & 0 & 0 & 0 & \\cdots & 0 & 0 & 0 & 0\\\\\n"
        "-\\sigma & 1+2\\sigma & -\\sigma & 0 & 0 & \\cdots & 0 & 0 & 0 & 0 \\\\\n"
        "0 & -\\sigma & 1+2\\sigma & -\\sigma & \\cdots & 0 & 0 & 0 & 0 & 0 \\\\\n"
        "0 & 0 & \\ddots & \\ddots & \\ddots & \\ddots & 0 & 0 & 0 & 0 \\\\\n"
        "0 & 0 & 0 & 0 & 0 & 0 & 0 & -\\sigma & 1+2\\sigma & -\\sigma \\\\\n"
        "0 & 0 & 0 & 0 & 0 & 0 & 0 & 0 & -\\sigma & 1+\\sigma\n"
        "\\end{bmatrix}\n"
        "\\begin{bmatrix}\n"
        "U_0^{n+1} \\\\\n"
        "U_1^{n+1} \\\\\n"
        "U_2^{n+1} \\\\\n"
        "\\vdots \\\\\n"
        "U_{J-2}^{n+1} \\\\\n"
        "U_{J-1}^{n+1}\n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "1-\\sigma & \\sigma & 0 & 0 & 0 & \\cdots & 0 & 0 & 0 & 0\\\\\n"
        "\\sigma & 1-2\\sigma & \\sigma & 0 & 0 & \\cdots & 0 & 0 & 0 & 0 \\\\\n"
        "0 & \\sigma & 1-2\\sigma & \\sigma & \\cdots & 0 & 0 & 0 & 0 & 0 \\\\\n"
        "0 & 0 & \\ddots & \\ddots & \\ddots & \\ddots & 0 & 0 & 0 & 0 \\\\\n"
        "0 & 0 & 0 & 0 & 0 & 0 & 0 & \\sigma & 1-2\\sigma & \\sigma \\\\\n"
        "0 & 0 & 0 & 0 & 0 & 0 & 0 & 0 & \\sigma & 1-\\sigma\n"
        "\\end{bmatrix}\n"
        "\\begin{bmatrix}\n"
        "U_0^{n} \\\\\n"
        "U_1^{n} \\\\\n"
        "U_2^{n} \\\\\n"
        "\\vdots \\\\\n"
        "U_{J-2}^{n} \\\\\n"
        "U_{J-1}^{n}\n"
        "\\end{bmatrix} +\n"
        "\\begin{bmatrix}\n"
        "\\Delta t f(U_0^n) \\\\\n"
        "\\Delta t f(U_1^n) \\\\\n"
        "\\Delta t f(U_2^n) \\\\\n"
        "\\vdots \\\\\n"
        "\\Delta t f(U_{J-2}^n) \\\\\n"
        "\\Delta t f(U_{J-1}^n)\n"
        "\\end{bmatrix}.\n"
        "$$\n"
        "\n"
        "Note that since our numerical integration starts with a well-defined initial condition at $n=0$, $\\mathbf{U}^0$, the\n"
        "vector $\\mathbf{U}^{n+1}$ on the left-hand side is the only unknown in this system of linear equations.\n"
        "\n"
        "Thus, to integrate numerically our reaction-diffusion system from time point $n$ to $n+1$ we need to solve numerically for vector $\\mathbf{U}^{n+1}$.\n"
        "\n"
        "Let us call the matrix on the left-hand side $A$, the one on the right-hand side $B$,\n"
        "and the vector on the right-hand side $\\mathbf{f}^n$.\n"
        "Using this notation we can write the above system as\n"
        "\n"
        "$$A \\mathbf{U}^{n+1} = B \\mathbf{U}^n + f^n.$$\n"
        "\n"
        "In this linear equation, matrices $A$ and $B$ are defined by our problem: we need to specify these matrices once for our\n"
        "problem and incorporate our boundary conditions in them.\n"
        "Vector $\\mathbf{f}^n$ is a function of $\\mathbf{U}^n$ and so needs to be reevaluated in every time point $n$.\n"
        "We also need to carry out one matrix-vector multiplication every time point, $B \\mathbf{U}^n$, and\n"
        "one vector-vector addition, $B \\mathbf{U}^n + f^n$.\n"
        "\n"
        "The most expensive numerical operation is inversion of matrix $A$ to solve for $\\mathbf{U}^{n+1}$, however we may\n"
        "get away with doing this only once and store the inverse of $A$ as $A^{-1}$:\n"
        "\n"
        "$$\\mathbf{U}^{n+1} = A^{-1} \\left( B \\mathbf{U}^n + f^n \\right).$$"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## A Crank-Nicolson Example in Python"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Let us apply the CN method to a two-variable reaction-diffusion system that was introduced by \n"
        "[Mori *et al.*](http://www.sciencedirect.com/science/article/pii/S0006349508704442):\n"
        "\n"
        "$$\\frac{\\partial u}{\\partial t} = D_u \\frac{\\partial^2 u}{\\partial x^2} + f(u,v),$$\n"
        "\n"
        "$$\\frac{\\partial v}{\\partial t} = D_v \\frac{\\partial^2 v}{\\partial x^2} - f(u,v),$$\n"
        "\n"
        "with Neumann boundary conditions\n"
        "\n"
        "$$\\frac{\\partial u}{\\partial x}\\Bigg|_{x=0,L} = 0,$$\n"
        "\n"
        "$$\\frac{\\partial v}{\\partial x}\\Bigg|_{x=0,L} = 0.$$\n"
        "\n"
        "The variables of this system, $u$ and $v$, represent the concetrations of the active form and its inactive form respectively.\n"
        "The reaction term $f(u,v)$ describes the interchange (activation and inactivation) between these two states of the protein.\n"
        "A particular property of this system is that the inactive has much greater diffusivity that the active form, $D_v \\gg D_u$.\n"
        "\n"
        "Using the CN method to integrate this system numerically, we need to set up two separate approximations\n"
        "\n"
        "$$A_u \\mathbf{U}^{n+1} = B_u \\mathbf{U}^n + \\mathbf{f}^n,$$\n"
        "\n"
        "$$A_v \\mathbf{V}^{n+1} = B_v \\mathbf{V}^n - \\mathbf{f}^n,$$\n"
        "\n"
        "with two different $\\sigma$ terms, $\\sigma_u = \\frac{D_u \\Delta t}{2 \\Delta x^2}$ and $\\sigma_v = \\frac{D_v \\Delta t}{2 \\Delta x^2}$."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Import Packages"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "For the matrix-vector multiplication, vector-vector addition, and matrix inversion that we will need to carry\n"
        "out we will use the Python library [NumPy](http://www.numpy.org/).\n"
        "To visualize our numerical solutions, we will use [pyplot](http://matplotlib.org/api/pyplot_api.html)."
    ));

    qDebug() << "command entry 1";
    testCommandEntry(entry, 1, QString::fromUtf8(
        "import numpy\n"
        "from matplotlib import pyplot"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Numpy allows us to truncate the numerical values of matrices and vectors to improve their display with \n"
        "[`set_printoptions`](http://docs.scipy.org/doc/numpy/reference/generated/numpy.set_printoptions.html)."
    ));

    qDebug() << "command entry 2";
    testCommandEntry(entry, 2, QString::fromUtf8(
        "numpy.set_printoptions(precision=3)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Specify Grid"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Our one-dimensional domain has unit length and we define `J = 100` equally spaced\n"
        "grid points in this domain.\n"
        "This divides our domain into `J-1` subintervals, each of length `dx`."
    ));

    qDebug() << "command entry 3";
    testCommandEntry(entry, 3, QString::fromUtf8(
        "L = 1.\n"
        "J = 100\n"
        "dx = float(L)/float(J-1)\n"
        "x_grid = numpy.array([j*dx for j in range(J)])"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Equally, we define `N = 1000` equally spaced grid points on our time domain of length `T = 200` thus dividing our time domain into `N-1` intervals of length `dt`."
    ));

    qDebug() << "command entry 4";
    testCommandEntry(entry, 4, QString::fromUtf8(
        "T = 200\n"
        "N = 1000\n"
        "dt = float(T)/float(N-1)\n"
        "t_grid = numpy.array([n*dt for n in range(N)])"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Specify System Parameters and the Reaction Term"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "We choose our parameter values based on the work by\n"
        "[Mori *et al.*](http://www.sciencedirect.com/science/article/pii/S0006349508704442)."
    ));

    qDebug() << "command entry 5";
    testCommandEntry(entry, 5, QString::fromUtf8(
        "D_v = float(10.)/float(100.)\n"
        "D_u = 0.01 * D_v\n"
        "\n"
        "k0 = 0.067\n"
        "f = lambda u, v: dt*(v*(k0 + float(u*u)/float(1. + u*u)) - u)\n"
        "g = lambda u, v: -f(u,v)\n"
        " \n"
        "sigma_u = float(D_u*dt)/float((2.*dx*dx))\n"
        "sigma_v = float(D_v*dt)/float((2.*dx*dx))\n"
        "\n"
        "total_protein = 2.26"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Specify the Initial Condition"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "As discussed by\n"
        "[Mori *et al.*](http://www.sciencedirect.com/science/article/pii/S0006349508704442),\n"
        "we can expect to observe interesting behaviour in the steady state of this system\n"
        "if we choose a heterogeneous initial condition for $u$.\n"
        "\n"
        "Here, we initialize $u$ with a step-like heterogeneity:"
    ));

    qDebug() << "command entry 7";
    testCommandEntry(entry, 7, QString::fromUtf8(
        "no_high = 10\n"
        "U =  numpy.array([0.1 for i in range(no_high,J)] + [2. for i in range(0,no_high)])\n"
        "V = numpy.array([float(total_protein-dx*sum(U))/float(J*dx) for i in range(0,J)])"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Note that we make certain that total protein amounts equal a certain value,\n"
        "`total_protein`.\n"
        "The importance of this was discussed by \n"
        "[Walther *et al.*](http://link.springer.com/article/10.1007%2Fs11538-012-9766-5).\n"
        "\n"
        "Let us plot our initial condition for confirmation:"
    ));

    qDebug() << "command entry 9";
    testCommandEntry(entry, 9, 1, QString::fromUtf8(
        "pyplot.ylim((0., 2.1))\n"
        "pyplot.xlabel('x'); pyplot.ylabel('concentration')\n"
        "pyplot.plot(x_grid, U)\n"
        "pyplot.plot(x_grid, V)\n"
        "pyplot.show()"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "The blue curve is the initial condition for $U$, stored in Python variable `U`,\n"
        "and the green curve is the initial condition for $V$ stored in `V`."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Create Matrices"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "The matrices that we need to construct are all tridiagonal so they are easy to\n"
        "construct with \n"
        "[`numpy.diagflat`](http://docs.scipy.org/doc/numpy/reference/generated/numpy.diagflat.html)."
    ));

    qDebug() << "command entry 10";
    testCommandEntry(entry, 10, QString::fromUtf8(
        "A_u = numpy.diagflat([-sigma_u for i in range(J-1)], -1) +\\\n"
        "      numpy.diagflat([1.+sigma_u]+[1.+2.*sigma_u for i in range(J-2)]+[1.+sigma_u]) +\\\n"
        "      numpy.diagflat([-sigma_u for i in range(J-1)], 1)\n"
        "        \n"
        "B_u = numpy.diagflat([sigma_u for i in range(J-1)], -1) +\\\n"
        "      numpy.diagflat([1.-sigma_u]+[1.-2.*sigma_u for i in range(J-2)]+[1.-sigma_u]) +\\\n"
        "      numpy.diagflat([sigma_u for i in range(J-1)], 1)\n"
        "        \n"
        "A_v = numpy.diagflat([-sigma_v for i in range(J-1)], -1) +\\\n"
        "      numpy.diagflat([1.+sigma_v]+[1.+2.*sigma_v for i in range(J-2)]+[1.+sigma_v]) +\\\n"
        "      numpy.diagflat([-sigma_v for i in range(J-1)], 1)\n"
        "        \n"
        "B_v = numpy.diagflat([sigma_v for i in range(J-1)], -1) +\\\n"
        "      numpy.diagflat([1.-sigma_v]+[1.-2.*sigma_v for i in range(J-2)]+[1.-sigma_v]) +\\\n"
        "      numpy.diagflat([sigma_v for i in range(J-1)], 1)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "To confirm, this is what `A_u` looks like:"
    ));

    qDebug() << "command entry 11";
    testCommandEntry(entry, 11, 1, QString::fromUtf8(
        "print A_u"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "[[ 1.981 -0.981  0.    ...  0.     0.     0.   ]\n"
        " [-0.981  2.962 -0.981 ...  0.     0.     0.   ]\n"
        " [ 0.    -0.981  2.962 ...  0.     0.     0.   ]\n"
        " ...\n"
        " [ 0.     0.     0.    ...  2.962 -0.981  0.   ]\n"
        " [ 0.     0.     0.    ... -0.981  2.962 -0.981]\n"
        " [ 0.     0.     0.    ...  0.    -0.981  1.981]]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "### Solve the System Iteratively"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "To advance our system by one time step, we need to do one matrix-vector multiplication followed by one vector-vector addition on the right hand side.\n"
        "\n"
        "To facilitate this, we rewrite our reaction term so that it accepts concentration vectors $\\mathbf{U}^n$ and $\\mathbf{V}^n$ as arguments\n"
        "and returns vector $\\mathbf{f}^n$.\n"
        "\n"
        "As a reminder, this is our non-vectorial definition of $f$\n"
        "\n"
        "    f = lambda u, v: v*(k0 + float(u*u)/float(1. + u*u)) - u"
    ));

    qDebug() << "command entry 12";
    testCommandEntry(entry, 12, QString::fromUtf8(
        "f_vec = lambda U, V: numpy.multiply(dt, numpy.subtract(numpy.multiply(V, \n"
        "                     numpy.add(k0, numpy.divide(numpy.multiply(U,U), numpy.add(1., numpy.multiply(U,U))))), U))"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Let us make certain that this produces the same values as our non-vectorial `f`:"
    ));

    qDebug() << "command entry 13";
    testCommandEntry(entry, 13, 1, QString::fromUtf8(
        "print f(U[0], V[0])"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "0.009961358982745121"
    ));
    entry = entry->next();

    qDebug() << "command entry 14";
    testCommandEntry(entry, 14, 1, QString::fromUtf8(
        "print f(U[-1], V[-1])"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "-0.06238322322322325"
    ));
    entry = entry->next();

    qDebug() << "command entry 15";
    testCommandEntry(entry, 15, 1, QString::fromUtf8(
        "print f_vec(U, V)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "[ 0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        "  0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01   0.01\n"
        " -0.062 -0.062 -0.062 -0.062 -0.062 -0.062 -0.062 -0.062 -0.062 -0.062]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Accounting for rounding of the displayed values due to the `set_printoptions` we set above, we\n"
        "can see that `f` and `f_vec` generate the same values for our initial condition at both ends of our domain."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "We will use [`numpy.linalg.solve`](http://docs.scipy.org/doc/numpy/reference/generated/numpy.linalg.solve.html) to solve\n"
        "our linear system each time step."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "While we integrate our system over time we will record both `U` and `V` at each\n"
        "time step in `U_record` and `V_record` respectively so that we can plot\n"
        "our numerical solutions over time."
    ));

    qDebug() << "command entry 16";
    testCommandEntry(entry, 16, QString::fromUtf8(
        "U_record = []\n"
        "V_record = []\n"
        "\n"
        "U_record.append(U)\n"
        "V_record.append(V)\n"
        "\n"
        "for ti in range(1,N):\n"
        "    U_new = numpy.linalg.solve(A_u, B_u.dot(U) + f_vec(U,V))\n"
        "    V_new = numpy.linalg.solve(A_v, B_v.dot(V) - f_vec(U,V))\n"
        "    \n"
        "    U = U_new\n"
        "    V = V_new\n"
        "    \n"
        "    U_record.append(U)\n"
        "    V_record.append(V)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Plot the Numerical Solution"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Let us take a look at the numerical solution we attain after `N` time steps."
    ));

    qDebug() << "command entry 18";
    testCommandEntry(entry, 18, 1, QString::fromUtf8(
        "pyplot.ylim((0., 2.1))\n"
        "pyplot.xlabel('x'); pyplot.ylabel('concentration')\n"
        "pyplot.plot(x_grid, U)\n"
        "pyplot.plot(x_grid, V)\n"
        "pyplot.show()"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "And here is a [kymograph](http://en.wikipedia.org/wiki/Kymograph) of the values of `U`.\n"
        "This plot shows concisely the behaviour of `U` over time and we can clear observe the wave-pinning\n"
        "behaviour described by [Mori *et al.*](http://www.sciencedirect.com/science/article/pii/S0006349508704442).\n"
        "Furthermore, we observe that this wave pattern is stable for about 50 units of time and we therefore\n"
        "conclude that this wave pattern is a stable steady state of our system."
    ));

    qDebug() << "command entry 21";
    testCommandEntry(entry, 21, 1, QString::fromUtf8(
        "U_record = numpy.array(U_record)\n"
        "V_record = numpy.array(V_record)\n"
        "\n"
        "fig, ax = pyplot.subplots()\n"
        "pyplot.xlabel('x'); pyplot.ylabel('t')\n"
        "heatmap = ax.pcolor(x_grid, t_grid, U_record, vmin=0., vmax=1.2)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testJupyter5()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("Automata and Computability using Jupyter.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testMarkdown(entry, QString::fromUtf8(
        "# Jove helps teach models of computation using Jupyter \n"
        "\n"
        "Included are modules on:\n"
        "\n"
        "* Sets, strings and languages\n"
        "* Language operations\n"
        "* Construction of and operations on DFA and NFA\n"
        "* Regular expression parsing and automata inter-conversion\n"
        "* Derivate-based parsing\n"
        "* Pushdown automata\n"
        "* The construction of parsers using context-free productions, including\n"
        "  a full lexer/parser for Jove's own markdown syntax\n"
        "* Studies of parsing: ambiguity, associativity, precedence\n"
        "* Turing machines (including one for the Collatz problem)\n"
        "\n"
        "For a complete Jove top-level reference, kindly refer to https://github.com/ganeshutah/Jove from where you can download and obtain Jove. You can also visit this Github link now and poke around (the NBViewer will display the contents).\n"
        "\n"
        "Once you are in the top-level Gallery link we provide, feel free to explore the hierarchy of modules found there.\n"
        "\n"
        "These notebooks should give you an idea of the contents.\n"
        "\n"
        "* [DFA Illustrations (has a Youtube)](http://nbviewer.jupyter.org/github/ganeshutah/Jove/blob/master/notebooks/tutorial/DFAUnit2.ipynb)\n"
        "\n"
        "* [Regular Operations](http://nbviewer.jupyter.org/github/ganeshutah/Jove/blob/master/notebooks/driver/Drive_AllRegularOps.ipynb)\n"
        "\n"
        "* [PDA Operations](http://nbviewer.jupyter.org/github/ganeshutah/Jove/blob/master/notebooks/driver/Drive_PDA_Based_Parsing.ipynb)\n"
        "\n"
        "* [TM Operations](http://nbviewer.jupyter.org/github/ganeshutah/Jove/blob/master/notebooks/driver/Drive_TM.ipynb)"
    ));

    qDebug() << "command entry 1";
    testCommandEntry(entry, 1, 1, QString::fromUtf8(
        "from IPython.display import YouTubeVideo\n"
        "YouTubeVideo('dGcLHtYLgDU')"
    ));
    testHtmlResult(entry, 0, QString::fromLatin1(
        "<IPython.lib.display.YouTubeVideo at 0x7fa7a1ee4c50>"
    ));
    entry = entry->next();

    qDebug() << "command entry 2";
    testCommandEntry(entry, 2, 1, QString::fromUtf8(
        "import sys\n"
        "sys.path[0:0] = ['/home/mmmm1998/Документы/Репозитории/Jove','/home/mmmm1998/Документы/Репозитории/Jove/3rdparty'] # Put these at the head of the search path\n"
        "from jove.DotBashers import *\n"
        "from jove.Def_DFA import *\n"
        "from jove.Def_NFA import *\n"
        "from jove.Def_RE2NFA import *\n"
        "from jove.Def_NFA2RE import *\n"
        "from jove.Def_md2mc import *"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "You may use any of these help commands:\n"
        "help(ResetStNum)\n"
        "help(NxtStateStr)\n"
        "\n"
        "You may use any of these help commands:\n"
        "help(mkp_dfa)\n"
        "help(mk_dfa)\n"
        "help(totalize_dfa)\n"
        "help(addtosigma_delta)\n"
        "help(step_dfa)\n"
        "help(run_dfa)\n"
        "help(accepts_dfa)\n"
        "help(comp_dfa)\n"
        "help(union_dfa)\n"
        "help(intersect_dfa)\n"
        "help(pruneUnreach)\n"
        "help(iso_dfa)\n"
        "help(langeq_dfa)\n"
        "help(same_status)\n"
        "help(h_langeq_dfa)\n"
        "help(fixptDist)\n"
        "help(min_dfa)\n"
        "help(pairFR)\n"
        "help(state_combos)\n"
        "help(sepFinNonFin)\n"
        "help(bash_eql_classes)\n"
        "help(listminus)\n"
        "help(bash_1)\n"
        "help(mk_rep_eqc)\n"
        "help(F_of)\n"
        "help(rep_of_s)\n"
        "help(q0_of)\n"
        "help(Delta_of)\n"
        "help(mk_state_eqc_name)\n"
        "\n"
        "You may use any of these help commands:\n"
        "help(mk_nfa)\n"
        "help(totalize_nfa)\n"
        "help(step_nfa)\n"
        "help(run_nfa)\n"
        "help(ec_step_nfa)\n"
        "help(Eclosure)\n"
        "help(Echelp)\n"
        "help(accepts_nfa)\n"
        "help(nfa2dfa)\n"
        "help(n2d)\n"
        "help(inSets)\n"
        "help(rev_dfa)\n"
        "help(min_dfa_brz)\n"
        "\n"
        "You may use any of these help commands:\n"
        "help(re2nfa)\n"
        "\n"
        "You may use any of these help commands:\n"
        "help(RE2Str)\n"
        "help(mk_gnfa)\n"
        "help(mk_gnfa_from_D)\n"
        "help(dfa2nfa)\n"
        "help(del_gnfa_states)\n"
        "help(gnfa_w_REStr)\n"
        "help(del_one_gnfa_state)\n"
        "help(Edges_Exist_Via)\n"
        "help(choose_state_to_del)\n"
        "help(form_alt_RE)\n"
        "help(form_concat_RE)\n"
        "help(form_kleene_RE)\n"
        "\n"
        "You may use any of these help commands:\n"
        "help(md2mc)\n"
        ".. and if you want to dig more, then ..\n"
        "help(default_line_attr)\n"
        "help(length_ok_input_items)\n"
        "help(union_line_attr_list_fld)\n"
        "help(extend_rsltdict)\n"
        "help(form_delta)\n"
        "help(get_machine_components)"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        " # Jove allows you to set problems in markdown and have students solve"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "1) LOdd1Three0 : Set of strings over {0,1} with an odd # of 1s OR exactly three 0s. \n"
        "\n"
        "* Hint on how to arrive at the language:\n"
        "\n"
        "  - develop NFAs for the two cases and perform their union. Obtain DFA\n"
        "\n"
        "  - develop REs for the two cases and perform the union. \n"
        "\n"
        "  - Testing the creations:\n"
        "\n"
        "    .   Come up with language for even # of 1s and separately for \"other than three 0s\". \n"
        " \n"
        "    .   Do two intersections. \n"
        " \n"
        "    .   Is the language empty?\n"
        "\n"
        "\n"
        "2) Language of strings over {0,1} with exactly two occurrences of 0101 in it.\n"
        "\n"
        " * Come up with it directly (take overlaps into account, i.e. 010101 has two occurrences in it\n"
        "\n"
        " * Come up in another way\n"
        "\n"
        "Notes:\n"
        "\n"
        "* Most of the problem students will have in this course is interpreting English (technical English)\n"
        "\n"
        "* So again, read the writeup at the beginning of Module6 (should be ready soon today) and work on using the tool.\n"
        "\n"
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "__Solutions__\n"
        "\n"
        "1) LOdd1Three0 : Set of strings over {0,1} with an odd # of 1s OR exactly three 0s. \n"
        "\n"
        "* Hint on how to arrive at the language:\n"
        "\n"
        "  - develop NFAs for the two cases and perform their union. Obtain DFA\n"
        "\n"
        "  - develop REs for the two cases and perform the union. \n"
        "\n"
        "  - Testing the creations:\n"
        "\n"
        "    .   Come up with language for even # of 1s and separately for \"other than three 0s\". \n"
        " \n"
        "    .   Do two intersections. \n"
        " \n"
        "    .   Is the language empty?\n"
        "\n"
        "\n"
        "2) Language of strings over {0,1} with exactly two occurrences of 0101 in it.\n"
        "\n"
        " * Come up with it directly (take overlaps into account, i.e. 010101 has two occurrences in it\n"
        "\n"
        " * Come up in another way\n"
        "\n"
        "Notes:\n"
        "\n"
        "* Most of the problem students will have in this course is interpreting English (technical English)\n"
        "\n"
        "* So again, read the writeup at the beginning of Module6 (should be ready soon today) and work on using the tool.\n"
        "\n"
        "\n"
        "\n"
        ""
    ));

    qDebug() << "command entry 3";
    testCommandEntry(entry, 3, 1, QString::fromUtf8(
        "RE_Odd1s  = \"(0* 1 0* (1 0* 1 0)*)*\"\n"
        "NFA_Odd1s = re2nfa(RE_Odd1s)\n"
        "DO_Odd1s  = dotObj_dfa(min_dfa(nfa2dfa(NFA_Odd1s)))\n"
        "DO_Odd1s"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    qDebug() << "command entry 4";
    testCommandEntry(entry, 4, 1, QString::fromUtf8(
        "RE_Ex3z = \"1* 0 1* 0 1* 0 1* + (0* 1 0* (1 0* 1 0*)*)\"\n"
        "NFA_Ex3z = re2nfa(RE_Ex3z)\n"
        "DO_Ex3z  = dotObj_dfa(min_dfa(nfa2dfa(NFA_Ex3z)))\n"
        "DO_Ex3z"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "# Check out all remaining modules of Jove covering these\n"
        "\n"
        "* Brzozowski derivatives for parsing\n"
        "* Brzozowski minimization\n"
        "* Context-free parsing\n"
        "* (soon to come) [Binary Decision Diagrams; obtain now from software/ at](http://www.cs.utah.edu/fv)\n"
        "* (soon to come) Post Correspondence Problem"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "# Brzozowski's minimization defined\n"
        "\n"
        "It is nothing but these steps done in this order:\n"
        "\n"
        "* Reverse\n"
        "* Determinize\n"
        "* Reverse\n"
        "* Determinize\n"
        "\n"
        "Voila! The machine is now minimal!"
    ));

    qDebug() << "command entry 5";
    testCommandEntry(entry, 5, QString::fromUtf8(
        "# The above example, with min_dfa replaced by the rev;det;rev;det\n"
        "\n"
        "DofNFA_Ex3z = nfa2dfa(re2nfa(\"1* 0 1* 0 1* 0 1* + (0* 1 0* (1 0* 1 0*)*)\"))\n"
        "dotObj_dfa(DofNFA_Ex3z)\n"
        "dotObj_dfa(DofNFA_Ex3z)\n"
        "minDofNFA_Ex3z = nfa2dfa(rev_dfa(nfa2dfa(rev_dfa(DofNFA_Ex3z))))"
    ));

    qDebug() << "command entry 6";
    testCommandEntry(entry, 6, 1, QString::fromUtf8(
        "dotObj_dfa(minDofNFA_Ex3z)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "# What's the largest postage that can't be made using 3,5 and 7 cents?\n"
        "\n"
        "Answer is 4. Find it out."
    ));

    qDebug() << "command entry 7";
    testCommandEntry(entry, 7, 1, QString::fromUtf8(
        "dotObj_dfa(min_dfa_brz(nfa2dfa(re2nfa(\"(111+11111+1111111)*\"))))"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "# Show ambiguity in parsing"
    ));

    qDebug() << "command entry 8";
    testCommandEntry(entry, 8, 1, QString::fromUtf8(
        "# Parsing an arithmetic expression\n"
        "pdaEamb = md2mc('''PDA\n"
        "!!E -> E * E | E + E | ~E | ( E ) | 2 | 3\n"
        "I : '', #  ; E#  -> M\n"
        "M : '', E  ; ~E  -> M\n"
        "M : '', E  ; E+E -> M\n"
        "M : '', E  ; E*E -> M\n"
        "M : '', E  ; (E) -> M\n"
        "M : '', E  ; 2   -> M\n"
        "M : '', E  ; 3   -> M\n"
        "M : ~,  ~  ; ''  -> M\n"
        "M : 2,  2  ; ''  -> M\n"
        "M : 3,  3  ; ''  -> M\n"
        "M : (,  (  ; ''  -> M\n"
        "M : ),  )  ; ''  -> M\n"
        "M : +,  +  ; ''  -> M\n"
        "M : *,  *  ; ''  -> M\n"
        "M : '', #  ; #   -> F\n"
        "'''\n"
        ")"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Generating LALR tables"
    ));
    entry = entry->next();

    qDebug() << "command entry 9";
    testCommandEntry(entry, 9, 1, QString::fromUtf8(
        "from jove.Def_PDA       import *"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "You may use any of these help commands:\n"
        "help(explore_pda)\n"
        "help(run_pda)\n"
        "help(classify_l_id_path)\n"
        "help(h_run_pda)\n"
        "help(interpret_w_eps)\n"
        "help(step_pda)\n"
        "help(suvivor_id)\n"
        "help(term_id)\n"
        "help(final_id)\n"
        "help(cvt_str_to_sym)\n"
        "help(is_surv_id)\n"
        "help(subsumed)\n"
        "help(is_term_id)\n"
        "help(is_final_id)"
    ));
    entry = entry->next();

    qDebug() << "command entry 10";
    testCommandEntry(entry, 10, 1, QString::fromUtf8(
        "explore_pda(\"3+2*3+2*3\", pdaEamb, STKMAX=7)"
    ));

    testTextResult(entry, 0, QString::fromUtf8(
        "*** Exploring wrt STKMAX =  7 ; increase it if needed ***\n"
        "String 3+2*3+2*3 accepted by your PDA in 13 ways :-) \n"
        "Here are the ways: \n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E*E*E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E*E*E#')\n"
        "->  ('M', '+2*3+2*3', '+E*E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E*E#')\n"
        "->  ('M', '*3+2*3', '*E*E#')\n"
        "->  ('M', '3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3+2*3', '+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E*E#')\n"
        "->  ('M', '*3+2*3', '*E*E#')\n"
        "->  ('M', '3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3+2*3', '+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E#')\n"
        "->  ('M', '*3+2*3', '*E#')\n"
        "->  ('M', '3+2*3', 'E#')\n"
        "->  ('M', '3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3+2*3', '+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E#')\n"
        "->  ('M', '*3+2*3', '*E#')\n"
        "->  ('M', '3+2*3', 'E#')\n"
        "->  ('M', '3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3', '3+E#')\n"
        "->  ('M', '+2*3', '+E#')\n"
        "->  ('M', '2*3', 'E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3+2*3', '+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E+E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E+E*E#')\n"
        "->  ('M', '*3+2*3', '*E+E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E+E*E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E+E*E#')\n"
        "->  ('M', '+2*3+2*3', '+E+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E+E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E+E*E#')\n"
        "->  ('M', '*3+2*3', '*E+E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3+2*3', 'E*E+E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E*E+E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E*E+E#')\n"
        "->  ('M', '+2*3+2*3', '+E*E+E#')\n"
        "->  ('M', '2*3+2*3', 'E*E+E#')\n"
        "->  ('M', '2*3+2*3', '2*E+E#')\n"
        "->  ('M', '*3+2*3', '*E+E#')\n"
        "->  ('M', '3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3', '3+E#')\n"
        "->  ('M', '+2*3', '+E#')\n"
        "->  ('M', '2*3', 'E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E#')\n"
        "->  ('M', '+2*3+2*3', '+E#')\n"
        "->  ('M', '2*3+2*3', 'E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E*E#')\n"
        "->  ('M', '*3+2*3', '*E*E#')\n"
        "->  ('M', '3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E#')\n"
        "->  ('M', '+2*3+2*3', '+E#')\n"
        "->  ('M', '2*3+2*3', 'E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E#')\n"
        "->  ('M', '*3+2*3', '*E#')\n"
        "->  ('M', '3+2*3', 'E#')\n"
        "->  ('M', '3+2*3', 'E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E#')\n"
        "->  ('M', '+2*3+2*3', '+E#')\n"
        "->  ('M', '2*3+2*3', 'E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E#')\n"
        "->  ('M', '*3+2*3', '*E#')\n"
        "->  ('M', '3+2*3', 'E#')\n"
        "->  ('M', '3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3', '3+E#')\n"
        "->  ('M', '+2*3', '+E#')\n"
        "->  ('M', '2*3', 'E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E#')\n"
        "->  ('M', '+2*3+2*3', '+E#')\n"
        "->  ('M', '2*3+2*3', 'E#')\n"
        "->  ('M', '2*3+2*3', 'E*E#')\n"
        "->  ('M', '2*3+2*3', 'E+E*E#')\n"
        "->  ('M', '2*3+2*3', 'E*E+E*E#')\n"
        "->  ('M', '2*3+2*3', '2*E+E*E#')\n"
        "->  ('M', '*3+2*3', '*E+E*E#')\n"
        "->  ('M', '3+2*3', 'E+E*E#')\n"
        "->  ('M', '3+2*3', '3+E*E#')\n"
        "->  ('M', '+2*3', '+E*E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E#')\n"
        "->  ('M', '+2*3+2*3', '+E#')\n"
        "->  ('M', '2*3+2*3', 'E#')\n"
        "->  ('M', '2*3+2*3', 'E+E#')\n"
        "->  ('M', '2*3+2*3', 'E*E+E#')\n"
        "->  ('M', '2*3+2*3', '2*E+E#')\n"
        "->  ('M', '*3+2*3', '*E+E#')\n"
        "->  ('M', '3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3', '3+E#')\n"
        "->  ('M', '+2*3', '+E#')\n"
        "->  ('M', '2*3', 'E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') .\n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+E+E#')\n"
        "->  ('M', '3+2*3+2*3', '3+E+E#')\n"
        "->  ('M', '+2*3+2*3', '+E+E#')\n"
        "->  ('M', '2*3+2*3', 'E+E#')\n"
        "->  ('M', '2*3+2*3', 'E*E+E#')\n"
        "->  ('M', '2*3+2*3', '2*E+E#')\n"
        "->  ('M', '*3+2*3', '*E+E#')\n"
        "->  ('M', '3+2*3', 'E+E#')\n"
        "->  ('M', '3+2*3', '3+E#')\n"
        "->  ('M', '+2*3', '+E#')\n"
        "->  ('M', '2*3', 'E#')\n"
        "->  ('M', '2*3', 'E*E#')\n"
        "->  ('M', '2*3', '2*E#')\n"
        "->  ('M', '*3', '*E#')\n"
        "->  ('M', '3', 'E#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') ."
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "# Show how to disambiguate"
    ));

    qDebug() << "command entry 11";
    testCommandEntry(entry, 11, 1, QString::fromUtf8(
        "# Parsing an arithmetic expression\n"
        "pdaE = md2mc('''PDA\n"
        "!!E -> E+T | T\n"
        "!!T -> T*F | F\n"
        "!!F -> 2 | 3 | ~F | (E)\n"
        "I : '', #  ; E#  -> M\n"
        "M : '', E  ; E+T -> M\n"
        "M : '', E  ; T   -> M\n"
        "M : '', T  ; T*F -> M\n"
        "M : '', T  ; F   -> M\n"
        "M : '', F  ; 2   -> M\n"
        "M : '', F  ; 3   -> M\n"
        "M : '', F  ; ~F  -> M\n"
        "M : '', F  ; (E) -> M\n"
        "M : ~,  ~  ; ''  -> M\n"
        "M : 2,  2  ; ''  -> M\n"
        "M : 3,  3  ; ''  -> M\n"
        "M : (,  (  ; ''  -> M\n"
        "M : ),  )  ; ''  -> M\n"
        "M : +,  +  ; ''  -> M\n"
        "M : *,  *  ; ''  -> M\n"
        "M : '', #  ; #   -> F\n"
        "'''\n"
        ")"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Generating LALR tables"
    ));
    entry = entry->next();

    qDebug() << "command entry 12";
    testCommandEntry(entry, 12, 1, QString::fromUtf8(
        "explore_pda(\"3+2*3+2*3\", pdaE, STKMAX=7)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "*** Exploring wrt STKMAX =  7 ; increase it if needed ***\n"
        "String 3+2*3+2*3 accepted by your PDA in 1 ways :-) \n"
        "Here are the ways: \n"
        "Final state  ('F', '', '#')\n"
        "Reached as follows:\n"
        "->  ('I', '3+2*3+2*3', '#')\n"
        "->  ('M', '3+2*3+2*3', 'E#')\n"
        "->  ('M', '3+2*3+2*3', 'E+T#')\n"
        "->  ('M', '3+2*3+2*3', 'E+T+T#')\n"
        "->  ('M', '3+2*3+2*3', 'T+T+T#')\n"
        "->  ('M', '3+2*3+2*3', 'F+T+T#')\n"
        "->  ('M', '3+2*3+2*3', '3+T+T#')\n"
        "->  ('M', '+2*3+2*3', '+T+T#')\n"
        "->  ('M', '2*3+2*3', 'T+T#')\n"
        "->  ('M', '2*3+2*3', 'T*F+T#')\n"
        "->  ('M', '2*3+2*3', 'F*F+T#')\n"
        "->  ('M', '2*3+2*3', '2*F+T#')\n"
        "->  ('M', '*3+2*3', '*F+T#')\n"
        "->  ('M', '3+2*3', 'F+T#')\n"
        "->  ('M', '3+2*3', '3+T#')\n"
        "->  ('M', '+2*3', '+T#')\n"
        "->  ('M', '2*3', 'T#')\n"
        "->  ('M', '2*3', 'T*F#')\n"
        "->  ('M', '2*3', 'F*F#')\n"
        "->  ('M', '2*3', '2*F#')\n"
        "->  ('M', '*3', '*F#')\n"
        "->  ('M', '3', 'F#')\n"
        "->  ('M', '3', '3#')\n"
        "->  ('M', '', '#')\n"
        "->  ('F', '', '#') ."
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "# And finally, run a Turing Machine with \"dynamic tape allocation\" :-)\n"
        "\n"
        "* Why not show how TMs are encoded? \n"
        "* This markdown gets parsed to build a TM!\n"
        "* This TM is for the famous \"3x+1\" problem (Collatz's Problem)"
    ));

    qDebug() << "command entry 13";
    testCommandEntry(entry, 13, QString::fromUtf8(
        "collatz_tm_str = \"\"\"\n"
        "TM\n"
        "\n"
        "i_start      : 0; ., R -> i_start             !! erase this zero and try to find more\n"
        "i_start      : 1; 1, R -> goto_lsb            !! we have a proper number, go to the lsb\n"
        "i_start      : .; ., S -> error               !! error on no input or input == 0\n"
        "\n"
        "\n"
        "goto_lsb     : 0; 0,R | 1; 1,R -> goto_lsb    !! scan off the right edge of the number\n"
        "goto_lsb     : .; .,L -> branch               !! take a step back to be on the lsb and start branch\n"
        "\n"
        "\n"
        "branch       : 0; .,L -> branch               !! number is even, divide by two and re-branch\n"
        "branch       : 1; 1,L -> check_n_eq_1         !! number is odd, check if it is 1\n"
        "\n"
        "\n"
        "check_n_eq_1 : 0; 0,R | 1; 1,R -> 01_fma      !! number wasn't 1, goto 3n+1\n"
        "check_n_eq_1 : .; .,R -> f_halt               !! number was 1, halt\n"
        "\n"
        "\n"
        "!! carrying 0 we see a 0 so write 0 and carry 0 forward\n"
        "00_fma       : 0; 0,L -> 00_fma\n"
        "\n"
        "!! carrying 0 we see a 1 (times 3 is 11) so write 1 and carry 1 forward\n"
        "00_fma       : 1; 1,L -> 01_fma\n"
        "\n"
        "!! reached the end of the number, go back to the start\n"
        "00_fma       : .; .,R -> goto_lsb             \n"
        "\n"
        "\n"
        "!! carrying 1 we see a 0 so write 1 and carry 0 forward\n"
        "01_fma       : 0; 1,L -> 00_fma  \n"
        "\n"
        "!! carrying 1 we see a 1 (times 3 is 11, plus our carry is 100) so write 0 and carry 10 forward\n"
        "01_fma       : 1; 0,L -> 10_fma  \n"
        "\n"
        "!! reached the end of the number, write our 1 and go back to the start\n"
        "01_fma       : .; 1,R -> goto_lsb             \n"
        "\n"
        "\n"
        "!! carrying 10 we see a 0, so write 0 and carry 1 forward\n"
        "10_fma       : 0; 0,L -> 01_fma\n"
        "\n"
        "!! carrying 10 we see a 1 (times 3 is 11, plus our carry is 101), so write 1 and carry 10 forward\n"
        "10_fma       : 1; 1,L -> 10_fma\n"
        "\n"
        "!! reached the end of the number, write a 0 from our 10 and carry 1\n"
        "10_fma       : .; 0,L -> 01_fma\n"
        "\n"
        "!!\"\"\"\n"
        ""
    ));

    qDebug() << "command entry 14";
    testCommandEntry(entry, 14, 2, QString::fromUtf8(
        "# Now show the above TM graphically!\n"
        "collatz_tm = md2mc(collatz_tm_str)\n"
        "dotObj_tm(collatz_tm, FuseEdges=True)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Generating LALR tables"
    ));
    testImageResult(entry, 1);
    entry = entry->next();

    qDebug() << "command entry 15";
    testCommandEntry(entry, 15, 1, QString::fromUtf8(
        "from jove.Def_TM      import *"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "You may use any of these help commands:\n"
        "help(step_tm)\n"
        "help(run_tm)\n"
        "help(explore_tm)"
    ));
    entry = entry->next();

    qDebug() << "command entry 16";
    testCommandEntry(entry, 16, 1, QString::fromUtf8(
        "# Will loop if the Collatz (\"3x+1\") program will ever loop!\n"
        "explore_tm(collatz_tm, \"0110\", 100)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Allocating  8  tape cells to the RIGHT!\n"
        "Allocating  8  tape cells to the LEFT!\n"
        "Detailing the halted configs now.\n"
        "Accepted at  ('f_halt', 5, '.....1..............', 65)\n"
        " via .. \n"
        " ->('i_start', 0, '0110', 100)\n"
        " ->('i_start', 1, '.110', 99)\n"
        " ->('goto_lsb', 2, '.110', 98)\n"
        " ->('goto_lsb', 3, '.110', 97)\n"
        " ->('goto_lsb', 4, '.110', 96)\n"
        " ->('branch', 3, '.110........', 95)\n"
        " ->('branch', 2, '.11.........', 94)\n"
        " ->('check_n_eq_1', 1, '.11.........', 93)\n"
        " ->('01_fma', 2, '.11.........', 92)\n"
        " ->('10_fma', 1, '.10.........', 91)\n"
        " ->('10_fma', 0, '.10.........', 90)\n"
        " ->('01_fma', 7, '........010.........', 89)\n"
        " ->('goto_lsb', 8, '.......1010.........', 88)\n"
        " ->('goto_lsb', 9, '.......1010.........', 87)\n"
        " ->('goto_lsb', 10, '.......1010.........', 86)\n"
        " ->('goto_lsb', 11, '.......1010.........', 85)\n"
        " ->('branch', 10, '.......1010.........', 84)\n"
        " ->('branch', 9, '.......101..........', 83)\n"
        " ->('check_n_eq_1', 8, '.......101..........', 82)\n"
        " ->('01_fma', 9, '.......101..........', 81)\n"
        " ->('10_fma', 8, '.......100..........', 80)\n"
        " ->('01_fma', 7, '.......100..........', 79)\n"
        " ->('10_fma', 6, '.......000..........', 78)\n"
        " ->('01_fma', 5, '......0000..........', 77)\n"
        " ->('goto_lsb', 6, '.....10000..........', 76)\n"
        " ->('goto_lsb', 7, '.....10000..........', 75)\n"
        " ->('goto_lsb', 8, '.....10000..........', 74)\n"
        " ->('goto_lsb', 9, '.....10000..........', 73)\n"
        " ->('goto_lsb', 10, '.....10000..........', 72)\n"
        " ->('branch', 9, '.....10000..........', 71)\n"
        " ->('branch', 8, '.....1000...........', 70)\n"
        " ->('branch', 7, '.....100............', 69)\n"
        " ->('branch', 6, '.....10.............', 68)\n"
        " ->('branch', 5, '.....1..............', 67)\n"
        " ->('check_n_eq_1', 4, '.....1..............', 66)\n"
        " ->('f_halt', 5, '.....1..............', 65)"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "# END: You have a ton more waiting for your execution pleasure!"
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testJupyter6()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("Cue Combination with Neural Populations .ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testMarkdown(entry, QString::fromUtf8(
        "# Humans and animals integrate multisensory cues near-optimally\n"
        "## An intuition for how populations of neurons can perform Bayesian inference"
    ));

    qDebug() << "command entry 30";
    testCommandEntry(entry, 30, QString::fromUtf8(
        "from __future__ import division\n"
        "import numpy as np\n"
        "from scipy.special import factorial\n"
        "import scipy.stats as stats\n"
        "import pylab\n"
        "import matplotlib.pyplot as plt\n"
        "%matplotlib inline\n"
        "import seaborn as sns\n"
        "sns.set_style(\"darkgrid\")\n"
        "import ipywidgets\n"
        "from IPython.display import display\n"
        "from matplotlib.font_manager import FontProperties\n"
        "fontP = FontProperties()\n"
        "fontP.set_size('medium')\n"
        "%config InlineBackend.figure_format = 'svg'\n"
        "\n"
        "\n"
        "def mean_firing_rate(gain, stimulus, preferred_stimulus, std_tc, baseline):\n"
        "    # Gaussian tuning curve that determines the mean firing rate (Poisson rate parameter) for a given stimulus\n"
        "    return baseline + gain*stats.norm.pdf(preferred_stimulus, loc = stimulus, scale = std_tc)\n"
        "\n"
        "def get_spikes(gain, stimulus, preferred_stimuli, std_tc, baseline):\n"
        "    # produce a vector of spikes for some population given some stimulus\n"
        "    lambdas = mean_firing_rate(gain, stimulus, preferred_stimuli, std_tc, baseline)\n"
        "    return np.random.poisson(lambdas)\n"
        "                \n"
        "def likelihood(stimulus, r, gain, preferred_stimuli, std_tc, baseline):\n"
        "    # returns p(r|s)\n"
        "    lambdas = mean_firing_rate(gain, stimulus, preferred_stimuli, std_tc, baseline)\n"
        "    return np.prod(lambdas**r)\n"
        "\n"
        "def spikes_and_inference(r_V = True,\n"
        "                         r_A = True,\n"
        "                         show_tuning_curves = False,\n"
        "                         show_spike_count = False,\n"
        "                         show_likelihoods = True,\n"
        "                         true_stimulus = 10,\n"
        "                         number_of_neurons = 40,\n"
        "                         r_V_gain = 15,\n"
        "                         r_A_gain = 75,\n"
        "                         r_V_tuning_curve_sigma = 10,\n"
        "                         r_A_tuning_curve_sigma = 10,\n"
        "                         tuning_curve_baseline = 0,\n"
        "                         joint_likelihood = True,\n"
        "                         r_V_plus_r_A = True,\n"
        "                         cue = False):\n"
        "    np.random.seed(7)\n"
        "    max_s = 40\n"
        "    preferred_stimuli = np.linspace(-max_s*2, max_s*2, number_of_neurons)\n"
        "    n_hypothesized_s = 250\n"
        "    hypothesized_s = np.linspace(-max_s, max_s, n_hypothesized_s)\n"
        "    gains     = {'r1':    r_V_gain,\n"
        "                 'r2':    r_A_gain,\n"
        "                 'r1+r2': r_V_gain + r_A_gain}\n"
        "    sigma_TCs = {'r1':    r_V_tuning_curve_sigma,\n"
        "                 'r2':    r_A_tuning_curve_sigma,\n"
        "                 'r1+r2': (r_V_tuning_curve_sigma + r_A_tuning_curve_sigma)/2}\n"
        "    spikes    = {'r1':    get_spikes(gains['r1'], true_stimulus, preferred_stimuli, sigma_TCs['r1'], tuning_curve_baseline),\n"
        "                 'r2':    get_spikes(gains['r2'], true_stimulus, preferred_stimuli, sigma_TCs['r2'], tuning_curve_baseline)}\n"
        "    spikes['r1+r2'] = spikes['r1'] + spikes['r2']\n"
        "    active_pops = []\n"
        "    if r_V: active_pops.append('r1')\n"
        "    if r_A: active_pops.append('r2')\n"
        "    if r_V_plus_r_A: active_pops.append('r1+r2')\n"
        "\n"
        "    colors = {'r1':    sns.xkcd_rgb['light purple'],\n"
        "              'r2':    sns.xkcd_rgb['dark pink'],\n"
        "              'r1+r2': sns.xkcd_rgb['royal blue'],\n"
        "              'joint': sns.xkcd_rgb['gold']}\n"
        "    nSubplots = show_spike_count + show_tuning_curves + show_likelihoods\n"
        "    fig, axes = plt.subplots(nSubplots, figsize = (7, 1.5*nSubplots)) # number of subplots according to what's been requested\n"
        "    if not isinstance(axes, np.ndarray): axes = [axes] # makes axes into a list even if it's just one subplot\n"
        "    subplot_idx = 0\n"
        "    \n"
        "    def plot_true_stimulus_and_legend(subplot_idx):\n"
        "        axes[subplot_idx].plot(true_stimulus, 0, 'k^', markersize = 12, clip_on = False, label = 'true rattlesnake location')\n"
        "        axes[subplot_idx].legend(loc = 'center left', bbox_to_anchor = (1, 0.5), prop = fontP)\n"
        "    \n"
        "    if show_tuning_curves:\n"
        "        for neuron in range(number_of_neurons):\n"
        "            if r_V:\n"
        "                axes[subplot_idx].plot(hypothesized_s,\n"
        "                                       mean_firing_rate(gains['r1'],\n"
        "                                                        hypothesized_s,\n"
        "                                                        preferred_stimuli[neuron],\n"
        "                                                        sigma_TCs['r1'],\n"
        "                                                        tuning_curve_baseline),\n"
        "                                       color = colors['r1'])\n"
        "            if r_A:\n"
        "                axes[subplot_idx].plot(hypothesized_s,\n"
        "                                       mean_firing_rate(gains['r2'],\n"
        "                                                        hypothesized_s,\n"
        "                                                        preferred_stimuli[neuron],\n"
        "                                                        sigma_TCs['r2'],\n"
        "                                                        tuning_curve_baseline),\n"
        "                                       color = colors['r2'])\n"
        "        axes[subplot_idx].set_xlabel('location $s$')\n"
        "        axes[subplot_idx].set_ylabel('mean firing rate\\n(spikes/s)')\n"
        "        axes[subplot_idx].set_ylim((0, 4))\n"
        "        axes[subplot_idx].set_xlim((-40, 40))\n"
        "        axes[subplot_idx].set_yticks(np.linspace(0, 4, 5))\n"
        "        subplot_idx += 1\n"
        "\n"
        "    if show_spike_count:\n"
        "        idx = abs(preferred_stimuli) < max_s\n"
        "        if r_V:\n"
        "            axes[subplot_idx].plot(preferred_stimuli[idx], spikes['r1'][idx], 'o', color = colors['r1'],\n"
        "                                   clip_on = False,  label = '$\\mathbf{r}_\\mathrm{V}$',\n"
        "                                   markersize=4)\n"
        "        if r_A:\n"
        "            axes[subplot_idx].plot(preferred_stimuli[idx], spikes['r2'][idx], 'o', color = colors['r2'],\n"
        "                                   clip_on = False, label = '$\\mathbf{r}_\\mathrm{A}$',\n"
        "                                   markersize=4)\n"
        "        if r_V_plus_r_A:\n"
        "            axes[subplot_idx].plot(preferred_stimuli[idx], spikes['r1+r2'][idx], 'o', color = colors['r1+r2'],\n"
        "                                   clip_on = False, label = '$\\mathbf{r}_\\mathrm{V}+\\mathbf{r}_\\mathrm{A}$',\n"
        "                                   markersize=8, zorder=1)\n"
        "        axes[subplot_idx].set_xlabel('preferred location')\n"
        "        axes[subplot_idx].set_ylabel('spike count')\n"
        "        axes[subplot_idx].set_ylim((0, 10))\n"
        "        axes[subplot_idx].set_xlim((-40, 40))\n"
        "        plot_true_stimulus_and_legend(subplot_idx)\n"
        "        subplot_idx += 1\n"
        "\n"
        "    if show_likelihoods:\n"
        "        if cue:\n"
        "            var = 'c'\n"
        "        else:\n"
        "            var = '\\mathbf{r}'\n"
        "        likelihoods = {}\n"
        "            \n"
        "        for population in active_pops:\n"
        "            likelihoods[population] = np.zeros_like(hypothesized_s)\n"
        "            for idx, ort in enumerate(hypothesized_s):\n"
        "                likelihoods[population][idx] = likelihood(ort, spikes[population], gains[population],\n"
        "                                                          preferred_stimuli, sigma_TCs[population], tuning_curve_baseline)\n"
        "            likelihoods[population] /= np.sum(likelihoods[population]) # normalize\n"
        "\n"
        "        if r_V:\n"
        "            axes[subplot_idx].plot(hypothesized_s, likelihoods['r1'], color = colors['r1'],\n"
        "                                   linewidth = 2, label = '$p({}_\\mathrm{{V}}|s)$'.format(var))\n"
        "        if r_A:\n"
        "            axes[subplot_idx].plot(hypothesized_s, likelihoods['r2'], color = colors['r2'],\n"
        "                                   linewidth = 2, label = '$p({}_\\mathrm{{A}}|s)$'.format(var))\n"
        "        if r_V_plus_r_A:\n"
        "            axes[subplot_idx].plot(hypothesized_s, likelihoods['r1+r2'], color = colors['r1+r2'],\n"
        "                                   linewidth = 2, label = '$p({}_\\mathrm{{V}}+{}_\\mathrm{{A}}|s)$'.format(var, var))\n"
        "        if joint_likelihood:\n"
        "            product = likelihoods['r1']*likelihoods['r2']\n"
        "            product /= np.sum(product)\n"
        "            axes[subplot_idx].plot(hypothesized_s, product, color = colors['joint'],linewidth = 7,\n"
        "                                   label = '$p({}_\\mathrm{{V}}|s)\\ p({}_\\mathrm{{A}}|s)$'.format(var, var), zorder = 1)\n"
        "\n"
        "        axes[subplot_idx].set_xlabel('location $s$')\n"
        "        axes[subplot_idx].set_ylabel('probability')\n"
        "        axes[subplot_idx].set_xlim((-40, 40))\n"
        "        axes[subplot_idx].legend()\n"
        "        axes[subplot_idx].set_yticks([])\n"
        "        \n"
        "        plot_true_stimulus_and_legend(subplot_idx)\n"
        "        subplot_idx += 1"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "<p>We live in a complex environment and must constantly integrate sensory information to interact with the world around us. Inputs from different modalities might not always be congruent with each other, but dissociating the true nature of the stimulus may be a matter of life or death for an organism.</p>\n"
        "<img src=\"http://www.wtadler.com/picdrop/rattlesnake.jpg\" width=25% height=25% align=\"left\" style=\"margin: 10px 10px 10px 0px;\" >\n"
        "<p>You hear and see evidence of a rattlesnake in tall grass near you. You get an auditory and a visual cue of the snake's location $s$. Both cues are associated with a likelihood function indicating the probability of that cue for all possible locations of the snake. The likelihood function associated with the visual cue, $p(c_\\mathrm{V}|s)$, has high uncertainty, because of the tall grass. The auditory cue is easier to localize, so its associated likelihood function, $p(c_\\mathrm{A}|s)$, is sharper. In accordance with Bayes' Rule, and assuming a flat prior over the snake's location, an optimal estimate of the location of the snake can be computed by multiplying the two likelihoods. This joint likelihood will be between the two cues but closer to the less uncertain cue, and will have less uncertainty than both unimodal likelihood functions.</p>"
    ));

    qDebug() << "command entry 31";
    testCommandEntry(entry, 31, 1, QString::fromUtf8(
        "spikes_and_inference(show_likelihoods = True, r_V_plus_r_A = False, cue = True)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Behavioral experiments have demonstrated that humans perform near-optimal Bayesian inference on ambiguous sensory information (van Beers *et al.*, 1999; Ernst & Banks, 2002; Kording & Wolpert, 2004; Stocker & Simoncelli, 2006). This has been demonstrated in cue combination experiments in which subjects report a near-optimal estimate of the stimulus given two noisy measurements of that stimulus. However, the neural basis for how humans might perform these computations is unclear. \n"
        "\n"
        "Ma *et. al.* (2006) propose that variance in cortical activity, rather than impairing sensory systems, is an adaptive mechanism to encode uncertainty in sensory measurements. They provide theory showing how the brain might use probabilistic population codes to perform near-optimal cue combination. We will re-derive the theory in here, and demonstrate it by simulating and decoding neural populations.\n"
        "\n"
        "## Cues can be represented by neural populations\n"
        "\n"
        "To return to our deadly rattlesnake, let's now assume that $c_\\mathrm{V}$ and $c_\\mathrm{A}$ are represented by populations of neurons $\\mathbf{r}_\\mathrm{V}$ and $\\mathbf{r}_\\mathrm{A}$, respectively. For our math and simulations, we assume that $\\mathbf{r}_\\mathrm{V}$ and $\\mathbf{r}_\\mathrm{A}$ are each composed of $N$ neurons that:\n"
        "\n"
        "* have independent Poisson variability\n"
        "* have regularly spaced Gaussian tuning curves that are identical in mean and variance for neurons with the same index in both populations\n"
        "\n"
        "The populations may have different gains, $g_\\mathrm{V}$ and $g_\\mathrm{A}$.\n"
        "\n"
        "These are the tuning curves for the neurons in $\\mathbf{r}_\\mathrm{V}$ (purple) and $\\mathbf{r}_\\mathrm{A}$ (red). Each curve represents the mean firing rate of a single neuron given a location $s$. Each neuron thus has a preferred location, which is where its tuning curve peaks."
    ));

    qDebug() << "command entry 32";
    testCommandEntry(entry, 32, 1, QString::fromUtf8(
        "spikes_and_inference(show_tuning_curves = True, show_likelihoods = False)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "The tuning curves are dense enough that we can also assume that $\\sum_{i=0}^N f_i(s) = k$ (*i.e.*, the sum of the tuning curves in a population is constant.)"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "First, we will show how the brain can decode a likelihood over stimulus from neural activity. Then we will ask how the brain can compute joint likelihoods.\n"
        "### How can the brain decode $p(\\mathbf{r_\\mathrm{V}}|s)$?\n"
        "\n"
        "\\begin{align}\n"
        "L(s) &= p(\\mathbf{r_\\mathrm{V}}\\ |\\ s) \\tag{1} \\\\ \n"
        "&= \\prod_{i=0}^N \\frac{e^{-g_\\mathrm{V}\\ f_i(s)}\\ g_\\mathrm{V}\\ f_i(s)^{r_{\\mathrm{V}i}}}{r_{\\mathrm{V}i}!} \\tag{2} \\\\\n"
        "&\\propto \\prod_{i=0}^N e^{-g_\\mathrm{V}\\ f_i(s)}\\ f_i(s)^{r_{\\mathrm{V}i}} \\tag{3} \\\\\n"
        "&= e^{-g_\\mathrm{V}\\sum_{i=0}^N f_i(s)} \\prod_{i=0}^N f_i(s)^{r_{\\mathrm{V}i}}\\tag{4} \\\\ \n"
        "&= e^{-g_\\mathrm{V}k} \\prod_{i=0}^N f_i(s)^{r_{\\mathrm{V}i}} \\tag{5} \\\\\n"
        "&\\propto \\prod_{i=0}^N f_i(s)^{r_{\\mathrm{V}i}} \\tag{6} \\\\\n"
        "\\end{align}\n"
        "\n"
        "### Then what is the joint likelihood $p(\\mathbf{r_\\mathrm{V}}|s)\\ p(\\mathbf{r_\\mathrm{A}}|s)$?\n"
        "\n"
        "\\begin{align}\n"
        "L(s) &= p(\\mathbf{r_\\mathrm{V}}\\ |\\ s)\\ p(\\mathbf{r_\\mathrm{A}}|s) \\tag{7} \\\\\n"
        "&\\propto \\prod_{i=0}^N f_i(s)^{r_{\\mathrm{V}i}}\\ \\prod_{i=0}^N f_i(s)^{r_{\\mathrm{A}i}} \\tag{8} \\\\\n"
        "&= \\prod_{i=0}^N f_i(s)^{r_{\\mathrm{V}i}+r_{\\mathrm{A}i}} \\tag{9} \\\\\n"
        "\\end{align}\n"
        "\n"
        "## How can the brain compute the joint likelihood $p(\\mathbf{r}_\\mathrm{V}|s)\\ p(\\mathbf{r}_\\mathrm{A}|s)$?\n"
        "The fact that we see neurons from $\\mathbf{r}_\\mathrm{V}$ and $\\mathbf{r}_\\mathrm{A}$ being added on a neuron-by-neuron basis in the exponent above suggests that we could construct a third population vector, $\\mathbf{r}_\\mathrm{V}+\\mathbf{r}_\\mathrm{A}$, and decode that.\n"
        "\n"
        "### First, we must prove that the sum of two Poisson-distributed random variables $X+Y$ is again Poisson-distributed.\n"
        "\\begin{align}\n"
        "X &\\sim \\textrm{Poisson}(\\lambda_x) \\textrm{, so } p(X=k)=\\frac{\\lambda_x^k\\ e^{-\\lambda_x}}{k!} \\tag{10} \\\\\n"
        "Y &\\sim \\textrm{Poisson}(\\lambda_y) \\textrm{, so } p(X=k)=\\frac{\\lambda_y^k\\ e^{-\\lambda_y}}{k!} \\tag{11} \\\\\n"
        "X+Y &\\overset{?}{\\sim} \\textrm{Poisson}(\\lambda_{x+y}) \\textrm{ and, if so, } \\lambda_{x+y}=? \\tag{12} \\\\\n"
        "\\end{align}\n"
        "\n"
        "\\begin{align}\n"
        "p(X+Y=n) &= p(X=0)\\ p(Y=n) + p(X=1)\\ p(Y=n-1)\\ +...+\\ p(X=n-1)\\ p(Y = 1) + p(X=n)\\ p(Y=0) \\tag{13} \\\\\n"
        "&= \\sum_{k=0}^n p(X=k)\\ p(Y=n-k) \\tag{14} \\\\\n"
        "&= \\sum_{k=0}^n \\frac{\\lambda_x^k\\ e^{-\\lambda_x}\\ \\lambda_y^{n-k}\\ e^{-\\lambda_y}}{k!(n-k)!} \\tag{15} \\\\\n"
        "&= e^{-(\\lambda_x+\\lambda_y)} \\sum_{k=0}^n \\frac{1}{k!(n-k)!}\\ \\lambda_x^k\\ \\lambda_y^{n-k} \\tag{16} \\\\\n"
        "&= e^{-(\\lambda_x+\\lambda_y)} \\frac{1}{n!} \\sum_{k=0}^n \\frac{n!}{k!(n-k)!}\\ \\lambda_x^k\\ \\lambda_y^{n-k} \\tag{17} \\\\\n"
        "&= e^{-(\\lambda_x+\\lambda_y)} \\frac{1}{n!} \\sum_{k=0}^n \\binom{n}{k}\\ \\lambda_x^k\\ \\lambda_y^{n-k}\\ [ \\textrm{because} \\frac{n!}{k!(n-k)!}=\\binom{n}{k} ]\\tag{18} \\\\\n"
        "&=\\frac{e^{-(\\lambda_x + \\lambda_y)}(\\lambda_x+\\lambda_y)^n}{n!} [ \\textrm{because} \\sum_{k=0}^n \\binom{n}{k}\\ x^ky^{n-k} = (x+y)^n ]\\tag{19} \\\\\n"
        "\\end{align}\n"
        "\n"
        "Therefore, $X + Y \\sim \\mathrm{Poisson}(\\lambda_x + \\lambda_y)$.\n"
        "\n"
        "## What is $p(\\mathbf{r}_\\mathrm{V}+\\mathbf{r}_\\mathrm{A} | s)$?\n"
        "\n"
        "In our case:\n"
        "\n"
        "\\begin{align}\n"
        "r_{\\mathrm{V}i} &\\sim \\textrm{Poisson}(g_\\mathrm{V}\\ f_i(s)) \\tag{20} \\\\\n"
        "r_{\\mathrm{A}i} &\\sim \\textrm{Poisson}(g_\\mathrm{A}\\ f_i(s)) \\tag{21} \\\\\n"
        "r_{\\mathrm{V}i}+r_{\\mathrm{A}i} &\\sim \\textrm{Poisson}((g_\\mathrm{V}+g_\\mathrm{A})\\ f_i(s)) \\tag{22} \\\\\n"
        "\\end{align}\n"
        "\n"
        "\\begin{align}\n"
        "L(s)&=p(\\mathbf{r}_\\mathrm{V} + \\mathbf{r}_\\mathrm{A}\\ |\\ s)\n"
        "= \\prod_{i=0}^N \\frac{e^{-f_i(s)(g_\\mathrm{V}+g_\\mathrm{A})}\\ (g_\\mathrm{V}+g_\\mathrm{A})\\ f_i(s)^{r_{\\mathrm{V}i}+r_{\\mathrm{A}i}}}{(r_{\\mathrm{V}i}+r_{\\mathrm{A}i})!} \\tag{23} \\\\\n"
        "&\\propto \\prod_{i=0}^N e^{-f_i(s)(g_\\mathrm{V}+g_\\mathrm{A})}\\ f_i(s)^{r_{\\mathrm{V}i}+r_{\\mathrm{A}i}} \\tag{24} \\\\\n"
        "&= e^{-(g_\\mathrm{V}+g_\\mathrm{A})\\sum_{i=0}^Nf_i(s)} \\prod_{i=0}^N \\ f_i(s)^{r_{\\mathrm{V}i}+r_{\\mathrm{A}i}} \\tag{25} \\\\\n"
        "&= e^{-(g_\\mathrm{V}+g_\\mathrm{A})k} \\prod_{i=0}^N \\ f_i(s)^{r_{\\mathrm{V}i}+r_{\\mathrm{A}i}} \\tag{26} \\\\\n"
        "&\\propto \\prod_{i=0}^N f_i(s)^{r_{\\mathrm{V}i}+r_{\\mathrm{A}i}} \\tag{27} \\\\\n"
        "\\end{align}\n"
        "\n"
        "Since equations $(9)$ and $(27)$ are proportional, we have shown that optimal cue combination can be executed by decoding linear sums of populations."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "$$x = 2$$"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Simulation\n"
        "Here are the spike counts (during 1 s) from the two populations on one trial. Depicted in blue is a third population vector that is the sum of $\\mathbf{r}_\\mathrm{V}$ and $\\mathbf{r}_\\mathrm{A}$."
    ));

    qDebug() << "command entry 33";
    testCommandEntry(entry, 33, 1, QString::fromUtf8(
        "spikes_and_inference(show_spike_count = True, show_likelihoods = False)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Here are the decoded likelihoods for each population alone $(6)$, the joint likelihood $(9)$, and the likelihood for the summed population $(27)$. Note that the joint likelihood (gold) is less uncertain than either unimodal likelihood. Also note that it is identical to the likelihood for the summed population (blue)."
    ));

    qDebug() << "command entry 34";
    testCommandEntry(entry, 34, 1, QString::fromUtf8(
        "spikes_and_inference()"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Here, we break the assumption that the two populations have the same tuning curve width. Note that the joint likelihood (gold) is no longer identical to the likelihood for the summed population (blue)."
    ));

    qDebug() << "command entry 35";
    testCommandEntry(entry, 35, 1, QString::fromUtf8(
        "spikes_and_inference(r_V_tuning_curve_sigma = 7, r_A_tuning_curve_sigma = 10)"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Now you can play interactively with the parameters of the simulation using these sliders, and watch the decoded likelihoods shift around. Every time you change a parameter, new sets of spikes are generated and used to infer $s$.\n"
        "\n"
        "For the simulation to be interactive, you'll have to download this notebook."
    ));

    qDebug() << "command entry 36";
    testCommandEntry(entry, 36, 1, QString::fromUtf8(
        "i = ipywidgets.interactive(spikes_and_inference,\n"
        "              true_stimulus = (-40, 40, .1),\n"
        "              number_of_neurons = (2, 200, 1),\n"
        "              r_V_gain = (0, 100, 1),\n"
        "              r_A_gain = (0, 100, 1),\n"
        "              r_V_tuning_curve_sigma = (0.1, 50, .1),\n"
        "              r_A_tuning_curve_sigma = (0.1, 50, .1),\n"
        "              tuning_curve_baseline = (0, 20, .1));\n"
        "display(ipywidgets.VBox(i.children[2:-1]))"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "\n"
        "\n"
        ""
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Conclusion\n"
        "\n"
        "It has been shown behaviorally that humans perform near-optimal Bayesian inference on ambiguous sensory information. As suggested by Ma *et. al.* (2006) and shown here, it is possible that the brain does this operation by simply performing linear combinations of populations of Poisson neurons receiving various sensory input. Cortical neurons may be particularly well suited for this task because they have Poisson-like firing rates, displaying reliable variability from trial to trial (Tolhurst, Movshon & Dean, 1982; Softky & Koch, 1993).\n"
        "\n"
        "High levels of noise in these populations might at first be difficult to reconcile considering highly precise behavioral data. However, variability in neural populations might be direcly representative of uncertainty in environmental stimuli. Variability in cortical populations would then be critical for precise neural coding.\n"
        "\n"
        "## References\n"
        "\n"
        "* Ernst MO, Banks MS. (2002). Humans integrate visual and haptic information in a statistically optimal fashion. *Nature.*\n"
        "* Körding KP, Wolpert DM. (2004). Bayesian integration in sensorimotor learning. *Nature.*\n"
        "* Ma WJ, Beck JM, Latham PE, Pouget A. (2006). Bayesian inference with probabilistic population codes. *Nature Neuroscience.*\n"
        "* Softky WR, Koch C. (1993). The highly irregular firing of cortical cells is inconsistent with temporal integration of random EPSPs. *Journal of Neuroscience.*\n"
        "* Stocker AA, Simoncelli EP. (2006). Noise characteristics and prior expectations in human visual speed perception. *Nature Neuroscience.*\n"
        "* Tolhurst, DJ, Movshon JA, Dean AF. (1983). The statistical reliability of signals in single neurons in cat and monkey visual cortex. *Vision Research.*\n"
        "* van Beers RJ, Sittig AC, Gon JJ. (1999). Integration of proprioceptive and visual position-information: An experimentally supported model. *Journal of Neurophysiology.*"
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testJupyter7()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("Transformation2D.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testMarkdown(entry, QString::fromUtf8(
        "# Rigid-body transformations in a plane (2D)\n"
        "\n"
        "> Marcos Duarte  \n"
        "> Laboratory of Biomechanics and Motor Control ([http://demotu.org/](http://demotu.org/))  \n"
        "> Federal University of ABC, Brazil"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "The kinematics of a rigid body is completely described by its pose, i.e., its position and orientation in space (and the corresponding changes are translation and rotation). The translation and rotation of a rigid body are also known as rigid-body transformations (or simply, rigid transformations).\n"
        "\n"
        "Remember that in physics, a [rigid body](https://en.wikipedia.org/wiki/Rigid_body) is a model (an idealization) for a body in which deformation is neglected, i.e., the distance between every pair of points in the body is considered constant. Consequently, the position and orientation of a rigid body can be completely described by a corresponding coordinate system attached to it. For instance, two (or more) coordinate systems can be used to represent the same rigid body at two (or more) instants or two (or more) rigid bodies in space.\n"
        "\n"
        "Rigid-body transformations are used in motion analysis (e.g., of the human body) to describe the position and orientation of each segment (using a local (anatomical) coordinate system defined for each segment) in relation to a global coordinate system fixed at the laboratory. Furthermore, one can define an additional coordinate system called technical coordinate system also fixed at the rigid body but not based on anatomical landmarks. In this case, the position of the technical markers is first described in the laboratory coordinate system, and then the technical coordinate system is calculated to recreate the anatomical landmarks position in order to finally calculate the original anatomical coordinate system (and obtain its unknown position and orientation through time).\n"
        "\n"
        "In what follows, we will study rigid-body transformations by looking at the transformations between two coordinate systems. For simplicity, let's first analyze planar (two-dimensional) rigid-body transformations and later we will extend these concepts to three dimensions (where the study of rotations are more complicated)."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Affine transformations\n"
        "\n"
        "Translation and rotation are two examples of [affine transformations](https://en.wikipedia.org/wiki/Affine_transformation). Affine transformations preserve straight lines, but not necessarily the distance between points. Other examples of affine transformations are scaling, shear, and reflection. The figure below illustrates different affine transformations in a plane. Note that a 3x3 matrix is shown on top of each transformation; these matrices are known as the transformation matrices and are the mathematical representation of the physical transformations. Next, we will study how to use this approach to describe the translation and rotation of a rigid-body.  \n"
        "<br>\n"
        "<figure><img src='https://upload.wikimedia.org/wikipedia/commons/thumb/2/2c/2D_affine_transformation_matrix.svg/360px-2D_affine_transformation_matrix.svg.png' alt='Affine transformations'/> <figcaption><center><i>Figure. Examples of affine transformations in a plane applied to a square (with the letter <b>F</b> in it) and the corresponding transformation matrices (<a href=\"https://en.wikipedia.org/wiki/Affine_transformation\">image from Wikipedia</a>).</i></center></figcaption> </figure>"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Translation\n"
        "\n"
        "In a two-dimensional space, two coordinates and one angle are sufficient to describe the pose of the rigid body, totalizing three degrees of freedom for a rigid body. Let's see first the transformation for translation, then for rotation, and combine them at last.\n"
        "\n"
        "A pure two-dimensional translation of a coordinate system in relation to other coordinate system and the representation of a point in these two coordinate systems are illustrated in the figure below (remember that this is equivalent to describing a translation between two rigid bodies).  \n"
        "<br>\n"
        "<figure><img src='./../images/translation2D.png' alt='translation 2D'/> <figcaption><center><i>Figure. A point in two-dimensional space represented in two coordinate systems (Global and local), with one system translated.</i></center></figcaption> </figure>\n"
        "\n"
        "The position of point $\\mathbf{P}$ originally described in the local coordinate system but now described in the Global coordinate system in vector form is:\n"
        "\n"
        "$$ \\mathbf{P_G} = \\mathbf{L_G} + \\mathbf{P_l} $$\n"
        "\n"
        "Or for each component:\n"
        "\n"
        "$$ \\mathbf{P_X} = \\mathbf{L_X} + \\mathbf{P}_x $$\n"
        "\n"
        "$$ \\mathbf{P_Y} = \\mathbf{L_Y} + \\mathbf{P}_y $$\n"
        "\n"
        "And in matrix form is:\n"
        "\n"
        "$$\n"
        "\\begin{bmatrix}\n"
        "\\mathbf{P_X} \\\\\n"
        "\\mathbf{P_Y} \n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "\\mathbf{L_X} \\\\\n"
        "\\mathbf{L_Y} \n"
        "\\end{bmatrix} +\n"
        "\\begin{bmatrix}\n"
        "\\mathbf{P}_x \\\\\n"
        "\\mathbf{P}_y \n"
        "\\end{bmatrix}\n"
        "$$\n"
        "\n"
        "Because position and translation can be treated as vectors, the inverse operation, to describe the position at the local coordinate system in terms of the Global coordinate system, is simply:\n"
        "\n"
        "$$ \\mathbf{P_l} = \\mathbf{P_G} -\\mathbf{L_G} $$\n"
        "<br>\n"
        "$$ \\begin{bmatrix}\n"
        "\\mathbf{P}_x \\\\\n"
        "\\mathbf{P}_y \n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "\\mathbf{P_X} \\\\\n"
        "\\mathbf{P_Y} \n"
        "\\end{bmatrix} - \n"
        "\\begin{bmatrix}\n"
        "\\mathbf{L_X} \\\\\n"
        "\\mathbf{L_Y} \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "From classical mechanics, this transformation is an example of [Galilean transformation](http://en.wikipedia.org/wiki/Galilean_transformation).   \n"
        "\n"
        "For example, if the local coordinate system is translated by $\\mathbf{L_G}=[2, 3]$ in relation to the Global coordinate system, a point with coordinates $\\mathbf{P_l}=[4, 5]$ at the local coordinate system will have the position $\\mathbf{P_G}=[6, 8]$ at the Global coordinate system:"
    ));

    qDebug() << "command entry 1";
    testCommandEntry(entry, 1, QString::fromUtf8(
        "# Import the necessary libraries\n"
        "import numpy as np"
    ));

    qDebug() << "command entry 2";
    testCommandEntry(entry, 2, 1, QString::fromUtf8(
        "LG = np.array([2, 3])  # (Numpy 1D array with 2 elements)\n"
        "Pl = np.array([4, 5])\n"
        "PG = LG + Pl\n"
        "PG"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "array([6, 8])"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "This operation also works if we have more than one data point (NumPy knows how to handle vectors with different dimensions):"
    ));

    qDebug() << "command entry 3";
    testCommandEntry(entry, 3, 1, QString::fromUtf8(
        "Pl = np.array([[4, 5], [6, 7], [8, 9]])  # 2D array with 3 rows and two columns\n"
        "PG = LG + Pl\n"
        "PG"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "array([[ 6,  8],\n"
        "       [ 8, 10],\n"
        "       [10, 12]])"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "## Rotation\n"
        "\n"
        "A pure two-dimensional rotation of a coordinate system in relation to other coordinate system and the representation of a point in these two coordinate systems are illustrated in the figure below (remember that this is equivalent to describing a rotation between two rigid bodies). The rotation is around an axis orthogonal to this page, not shown in the figure (for a three-dimensional coordinate system the rotation would be around the $\\mathbf{Z}$ axis).  \n"
        "<br>\n"
        "<figure><img src='./../images/rotation2D.png' alt='rotation 2D'/> <figcaption><center><i>Figure. A point in the two-dimensional space represented in two coordinate systems (Global and local), with one system rotated in relation to the other around an axis orthogonal to both coordinate systems.</i></center></figcaption> </figure>\n"
        "\n"
        "Consider we want to express the position of point $\\mathbf{P}$ in the Global coordinate system in terms of the local coordinate system knowing only the coordinates at the local coordinate system and the angle of rotation between the two coordinate systems.   \n"
        "\n"
        "There are different ways of deducing that, we will see three of these methods next.     "
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### 1. Using trigonometry\n"
        "\n"
        "From figure below, the coordinates of point $\\mathbf{P}$ in the Global coordinate system can be determined finding the sides of the triangles marked in red.   \n"
        "<br>\n"
        "<figure><img src='./../images/rotation2Db.png' alt='rotation 2D'/> <figcaption><center><i>Figure. The coordinates of a point at the Global coordinate system in terms of the coordinates of this point at the local coordinate system.</i></center></figcaption> </figure>\n"
        "\n"
        "Then:   \n"
        "\n"
        "$$ \\mathbf{P_X} = \\mathbf{P}_x \\cos \\alpha - \\mathbf{P}_y \\sin \\alpha $$\n"
        "\n"
        "$$ \\mathbf{P_Y} = \\mathbf{P}_x \\sin \\alpha + \\mathbf{P}_y \\cos \\alpha  $$  \n"
        "\n"
        "The equations above can be expressed in matrix form:\n"
        "\n"
        "$$\n"
        "\\begin{bmatrix} \n"
        "\\mathbf{P_X} \\\\\n"
        "\\mathbf{P_Y} \n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha \\\\\n"
        "\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} \\begin{bmatrix}\n"
        "\\mathbf{P}_x \\\\\n"
        "\\mathbf{P}_y \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "Or simply:\n"
        "\n"
        "$$ \\mathbf{P_G} = \\mathbf{R_{Gl}}\\mathbf{P_l} $$\n"
        "\n"
        "Where $\\mathbf{R_{Gl}}$ is the rotation matrix that rotates the coordinates from the local to the Global coordinate system:\n"
        "\n"
        "$$ \\mathbf{R_{Gl}} = \\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha \\\\\n"
        "\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "So, given any position at the local coordinate system, with the rotation matrix above we are able to determine the position at the Global coordinate system. Let's check that before looking at other methods to obtain this matrix.  \n"
        "\n"
        "For instance, consider a local coordinate system rotated by $45^o$ in relation to the Global coordinate system, a point in the local coordinate system with position $\\mathbf{P_l}=[1, 1]$ will have the following position at the Global coordinate system:"
    ));

    qDebug() << "command entry 4";
    testCommandEntry(entry, 4, 1, QString::fromUtf8(
        "RGl = np.array([[np.cos(np.pi/4), -np.sin(np.pi/4)], [np.sin(np.pi/4), np.cos(np.pi/4)]])\n"
        "Pl  = np.array([[1, 1]]).T  # transpose the array for correct matrix multiplication\n"
        "PG  = np.dot(RGl, Pl)       # the function dot() is used for matrix multiplication of arrays\n"
        "np.around(PG, 4)            # round the number due to floating-point arithmetic errors"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "array([[0.    ],\n"
        "       [1.4142]])"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "We have rounded the number to 4 decimal places due to [floating-point arithmetic errors in the computation](http://floating-point-gui.de).   \n"
        "\n"
        "And if we have the points [1,1], [0,1], [1,0] at the local coordinate system, their positions at the Global coordinate system are:"
    ));

    qDebug() << "command entry 5";
    testCommandEntry(entry, 5, 1, QString::fromUtf8(
        "Pl = np.array([[1, 1], [0, 1], [1, 0]]).T  # transpose array for matrix multiplication\n"
        "PG = np.dot(RGl, Pl)  # the function dot() is used for matrix multiplication with arrays\n"
        "np.around(PG, 4)      # round the number due to floating point arithmetic errors"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "array([[ 0.    , -0.7071,  0.7071],\n"
        "       [ 1.4142,  0.7071,  0.7071]])"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "We have done all the calculations using the array function in NumPy. A [NumPy array is different than a matrix](http://www.scipy.org/NumPy_for_Matlab_Users), if we want to use explicit matrices in NumPy, the calculation above will be:"
    ));

    qDebug() << "command entry 6";
    testCommandEntry(entry, 6, 1, QString::fromUtf8(
        "RGl = np.mat([[np.cos(np.pi/4), -np.sin(np.pi/4)], [np.sin(np.pi/4), np.cos(np.pi/4)]])\n"
        "Pl  = np.mat([[1, 1], [0,1], [1, 0]]).T  # 2x3 matrix\n"
        "PG  = RGl*Pl       # matrix multiplication in NumPy\n"
        "np.around(PG, 4)   # round the number due to floating point arithmetic errors"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "array([[ 0.    , -0.7071,  0.7071],\n"
        "       [ 1.4142,  0.7071,  0.7071]])"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Both array and matrix types work in NumPy, but you should choose only one type and not mix them; the array is preferred because it is [the standard vector/matrix/tensor type of NumPy](http://www.scipy.org/NumPy_for_Matlab_Users)."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### 2. Using direction cosines\n"
        "\n"
        "Another way to determine the rotation matrix is to use the concept of direction cosine.   \n"
        "\n"
        "> Direction cosines are the cosines of the angles between any two vectors.   \n"
        "\n"
        "For the present case with two coordinate systems, they are  the cosines of the angles between each axis of one coordinate system and each axis of the other coordinate system. The figure below illustrates the directions angles between the two coordinate systems, expressing the local coordinate system in terms of the Global coordinate system.  \n"
        "<br>\n"
        "<figure><img src='./../images/directioncosine2D.png' alt='direction angles 2D'/> <figcaption><center><i>Figure. Definition of direction angles at the two-dimensional space.</i></center></figcaption> </figure>  \n"
        "<br>\n"
        "$$ \\mathbf{R_{Gl}} = \\begin{bmatrix}\n"
        "\\cos\\mathbf{X}x & \\cos\\mathbf{X}y \\\\\n"
        "\\cos\\mathbf{Y}x & \\cos\\mathbf{Y}y \n"
        "\\end{bmatrix} = \n"
        "\\begin{bmatrix}\n"
        "\\cos(\\alpha) & \\cos(90^o+\\alpha) \\\\\n"
        "\\cos(90^o-\\alpha) & \\cos(\\alpha)\n"
        "\\end{bmatrix} = \n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha \\\\\n"
        "\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} $$  \n"
        "\n"
        "The same rotation matrix as obtained before.\n"
        "\n"
        "Note that the order of the direction cosines is because in our convention, the first row is for the $\\mathbf{X}$ coordinate and the second row for the $\\mathbf{Y}$ coordinate (the outputs). For the inputs, we followed the same order, first column for the $\\mathbf{x}$ coordinate, second column for the $\\mathbf{y}$ coordinate."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### 3. Using a basis\n"
        "\n"
        "Another way to deduce the rotation matrix is to view the axes of the rotated coordinate system as unit vectors, versors, of a <a href=\"http://en.wikipedia.org/wiki/Basis_(linear_algebra)\">basis</a> as illustrated in the figure below.\n"
        "\n"
        "> A basis is a set of linearly independent vectors that can represent every vector in a given vector space, i.e., a basis defines a coordinate system.\n"
        "\n"
        "<figure><img src='./../images/basis2D2.png' alt='basis 2D'/> <figcaption><center><i>Figure. Definition of the rotation matrix using a basis at the two-dimensional space.</i></center></figcaption> </figure>\n"
        "\n"
        "The coordinates of these two versors at the local coordinate system in terms of the Global coordinate system are:\n"
        "\n"
        "$$ \\begin{array}{l l}\n"
        "\\mathbf{e}_x = \\cos\\alpha\\:\\mathbf{e_X} + \\sin\\alpha\\:\\mathbf{e_Y} \\\\\n"
        "\\mathbf{e}_y = -\\sin\\alpha\\:\\mathbf{e_X} + \\cos\\alpha\\:\\mathbf{e_Y}\n"
        "\\end{array}$$\n"
        "\n"
        "Note that as unit vectors, each of the versors above should have norm (length) equals to one, which indeed is the case.\n"
        "\n"
        "If we express each versor above as different columns of a matrix, we obtain the rotation matrix again:  \n"
        "\n"
        "$$ \\mathbf{R_{Gl}} = \\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha \\\\\\\n"
        "\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "This means that the rotation matrix can be viewed as the basis of the rotated coordinate system defined by its versors.   \n"
        "\n"
        "This third way to derive the rotation matrix is in fact the method most commonly used in motion analysis because the coordinates of markers (in the Global/laboratory coordinate system) are what we measure with cameras.   "
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### 4. Using the inner (dot or scalar) product between versors\n"
        "\n"
        "Yet another way to deduce the rotation matrix is to define it as the dot product between the versors of the bases related to the two coordinate systems:\n"
        "\n"
        "$$\n"
        "\\mathbf{R_{Gl}} = \\begin{bmatrix}\n"
        "\\mathbf{\\hat{e}_X}\\! \\cdot \\mathbf{\\hat{e}_x} & \\mathbf{\\hat{e}_X}\\! \\cdot \\mathbf{\\hat{e}_y} \\\\\n"
        "\\mathbf{\\hat{e}_Y}\\! \\cdot \\mathbf{\\hat{e}_x} & \\mathbf{\\hat{e}_Y}\\! \\cdot \\mathbf{\\hat{e}_y} \n"
        "\\end{bmatrix}\n"
        "$$  \n"
        "\n"
        "By definition:\n"
        "\n"
        "$$ \\hat{\\mathbf{e}}_1\\! \\cdot \\hat{\\mathbf{e}}_2 = ||\\hat{\\mathbf{e}}_1|| \\times ||\\hat{\\mathbf{e}}_2||\\cos(e_1,e_2)=\\cos(e_1,e_2)$$\n"
        "\n"
        "And the rotation matrix will be equal to the matrix deduced based on the direction cosines."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Local-to-Global and Global-to-local coordinate systems' rotations"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "If we want the inverse operation, to express the position of point $\\mathbf{P}$ in the local coordinate system in terms of the Global coordinate system, the figure below illustrates that using trigonometry.  \n"
        "<br>\n"
        "<figure><img src='./../images/rotation2Dc.png' alt='rotation 2D'/> <figcaption><center><i>Figure. The coordinates of a point at the local coordinate system in terms of the coordinates at the Global coordinate system.</i></center></figcaption> </figure>\n"
        "\n"
        "Then:\n"
        "\n"
        "$$ \\mathbf{P}_x = \\;\\;\\mathbf{P_X} \\cos \\alpha + \\mathbf{P_Y} \\sin \\alpha $$\n"
        "\n"
        "$$ \\mathbf{P}_y = -\\mathbf{P_X} \\sin \\alpha + \\mathbf{P_Y} \\cos \\alpha  $$\n"
        "\n"
        "And in matrix form:\n"
        "\n"
        "$$\n"
        "\\begin{bmatrix} \n"
        "\\mathbf{P}_x \\\\\n"
        "\\mathbf{P}_y \n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & \\sin\\alpha \\\\\n"
        "-\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} \\begin{bmatrix}\n"
        "\\mathbf{P_X} \\\\\n"
        "\\mathbf{P_Y} \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "$$ \\mathbf{P_l} = \\mathbf{R_{lG}}\\mathbf{P_G} $$\n"
        "\n"
        "Where $\\mathbf{R_{lG}}$ is the rotation matrix that rotates the coordinates from the Global to the local coordinate system (note the inverse order of the subscripts):\n"
        "\n"
        "$$ \\mathbf{R_{lG}} = \\begin{bmatrix}\n"
        "\\cos\\alpha & \\sin\\alpha \\\\\n"
        "-\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "If we use the direction cosines to calculate the rotation matrix, because the axes didn't change, the cosines are the same, only the order changes, now $\\mathbf{x, y}$ are the rows (outputs) and $\\mathbf{X, Y}$ are the columns (inputs):\n"
        "\n"
        "$$ \\mathbf{R_{lG}} = \\begin{bmatrix}\n"
        "\\cos\\mathbf{X}x & \\cos\\mathbf{Y}x \\\\\n"
        "\\cos\\mathbf{X}y & \\cos\\mathbf{Y}y \n"
        "\\end{bmatrix} = \n"
        "\\begin{bmatrix}\n"
        "\\cos(\\alpha) & \\cos(90^o-\\alpha) \\\\\n"
        "\\cos(90^o+\\alpha) & \\cos(\\alpha)\n"
        "\\end{bmatrix} = \n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & \\sin\\alpha \\\\\n"
        "-\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "And defining the versors of the axes in the Global coordinate system for a basis in terms of the local coordinate system would also produce this latter rotation matrix.\n"
        "\n"
        "The two sets of equations and matrices for the rotations from Global-to-local and local-to-Global coordinate systems are very similar, this is no coincidence. Each of the rotation matrices we deduced, $\\mathbf{R_{Gl}}$ and $\\mathbf{R_{lG}}$, perform the inverse operation in relation to the other. Each matrix is the inverse of the other.   \n"
        "\n"
        "In other words, the relation between the two rotation matrices means it is equivalent to instead of rotating the local coordinate system by $\\alpha$ in relation to the Global coordinate system, to rotate the Global coordinate system by $-\\alpha$ in relation to the local coordinate system; remember that $\\cos(-\\alpha)=\\cos(\\alpha)$ and $\\sin(-\\alpha)=-\\sin(\\alpha)$."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Rotation of a Vector\n"
        "\n"
        "We can also use the rotation matrix to rotate a vector by a given angle around an axis of the coordinate system as shown in the figure below.   \n"
        "<br>\n"
        "<figure><img src='./../images/rotation2Dvector.png' alt='rotation 2D of a vector'/> <figcaption><center><i>Figure. Rotation of a position vector $\\mathbf{P}$ by an angle $\\alpha$ in the two-dimensional space.</i></center></figcaption> </figure>\n"
        "\n"
        "We will not prove that we use the same rotation matrix, but think that in this case the vector position rotates by the same angle instead of the coordinate system. The new coordinates of the vector position $\\mathbf{P'}$ rotated by an angle $\\alpha$ is simply the rotation matrix (for the angle $\\alpha$) multiplied by the coordinates of the vector position $\\mathbf{P}$:\n"
        "\n"
        "$$ \\mathbf{P'} = \\mathbf{R}_\\alpha\\mathbf{P} $$\n"
        "\n"
        "Consider for example that $\\mathbf{P}=[2,1]$ and $\\alpha=30^o$; the coordinates of $\\mathbf{P'}$ are:"
    ));

    qDebug() << "command entry 7";
    testCommandEntry(entry, 7, 1, QString::fromUtf8(
        "a  = np.pi/6\n"
        "R  = np.array([[np.cos(a), -np.sin(a)], [np.sin(a), np.cos(a)]])\n"
        "P  = np.array([[2, 1]]).T\n"
        "Pl = np.dot(R, P)\n"
        "print(\"P':\\n\", Pl)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "P':\n"
        " [[1.23205081]\n"
        " [1.8660254 ]]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "### The rotation matrix\n"
        "\n"
        "**[See here for a review about matrix and its main properties](http://nbviewer.ipython.org/github/demotu/BMC/blob/master/notebooks/Matrix.ipynb)**.\n"
        "\n"
        "A nice property of the rotation matrix is that its inverse is the transpose of the matrix (because the columns/rows are mutually orthogonal and have norm equal to one).   \n"
        "This property can be shown with the rotation matrices we deduced:\n"
        "\n"
        "$$ \\begin{array}{l l}\n"
        "\\mathbf{R}\\:\\mathbf{R^T} & = \n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha \\\\\n"
        "\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} \n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & \\sin\\alpha \\\\\n"
        "-\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} \\\\\n"
        "& = \\begin{bmatrix}\n"
        "\\cos^2\\alpha+\\sin^2\\alpha & \\cos\\alpha \\sin\\alpha-\\sin\\alpha \\cos\\alpha\\;\\; \\\\\n"
        "\\sin\\alpha \\cos\\alpha-\\cos\\alpha \\sin\\alpha & \\sin^2\\alpha+\\cos^2\\alpha\\;\\;\n"
        "\\end{bmatrix} \\\\\n"
        "& = \\begin{bmatrix}\n"
        "1 & 0 \\\\\n"
        "0 & 1 \n"
        "\\end{bmatrix} \\\\\n"
        "& = \\mathbf{I} \\\\\n"
        "\\mathbf{R^{-1}} = \\mathbf{R^T}\n"
        "\\end{array} $$\n"
        "\n"
        "This means that if we have a rotation matrix, we know its inverse.   \n"
        "\n"
        "The transpose and inverse operators in NumPy are methods of the array:"
    ));

    qDebug() << "command entry 8";
    testCommandEntry(entry, 8, 1, QString::fromUtf8(
        "RGl = np.mat([[np.cos(np.pi/4), -np.sin(np.pi/4)], [np.sin(np.pi/4), np.cos(np.pi/4)]])\n"
        "\n"
        "print('Orthogonal matrix (RGl):\\n', np.around(RGl, 4))\n"
        "print('Transpose (RGl.T):\\n', np.around(RGl.T, 4))\n"
        "print('Inverse (RGl.I):\\n', np.around(RGl.I, 4))"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Orthogonal matrix (RGl):\n"
        " [[ 0.7071 -0.7071]\n"
        " [ 0.7071  0.7071]]\n"
        "Transpose (RGl.T):\n"
        " [[ 0.7071  0.7071]\n"
        " [-0.7071  0.7071]]\n"
        "Inverse (RGl.I):\n"
        " [[ 0.7071  0.7071]\n"
        " [-0.7071  0.7071]]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Using the inverse and the transpose mathematical operations, the coordinates at the local coordinate system given the coordinates at the Global coordinate system and the rotation matrix can be obtained by:   \n"
        "\n"
        "$$ \\begin{array}{l l}\n"
        "\\mathbf{P_G} = \\mathbf{R_{Gl}}\\mathbf{P_l} \\implies \\\\\n"
        "\\\\\n"
        "\\mathbf{R_{Gl}^{-1}}\\mathbf{P_G} = \\mathbf{R_{Gl}^{-1}}\\mathbf{R_{Gl}}\\mathbf{P_l} \\implies \\\\\n"
        "\\\\\n"
        "\\mathbf{R_{Gl}^{-1}}\\mathbf{P_G} = \\mathbf{I}\\:\\mathbf{P_l} \\implies \\\\\n"
        "\\\\\n"
        "\\mathbf{P_l} = \\mathbf{R_{Gl}^{-1}}\\mathbf{P_G} = \\mathbf{R_{Gl}^T}\\mathbf{P_G} \\quad \\text{or}\n"
        "\\quad \\mathbf{P_l} = \\mathbf{R_{lG}}\\mathbf{P_G}\n"
        "\\end{array} $$\n"
        "\n"
        "Where we referred the inverse of $\\mathbf{R_{Gl}}\\;(\\:\\mathbf{R_{Gl}^{-1}})$ as $\\mathbf{R_{lG}}$ (note the different order of the subscripts).  \n"
        "\n"
        "Let's show this calculation in NumPy:"
    ));

    qDebug() << "command entry 9";
    testCommandEntry(entry, 9, 1, QString::fromUtf8(
        "RGl = np.array([[np.cos(np.pi/4), -np.sin(np.pi/4)], [np.sin(np.pi/4), np.cos(np.pi/4)]])\n"
        "print('Rotation matrix (RGl):\\n', np.around(RGl, 4))\n"
        "\n"
        "Pl  = np.array([[1, 1]]).T # transpose the array for correct matrix multiplication\n"
        "print('Position at the local coordinate system (Pl):\\n', Pl)\n"
        "\n"
        "PG = np.dot(RGl, Pl) # the function dot() is used for matrix multiplication with arrays\n"
        "print('Position at the Global coordinate system (PG=RGl*Pl):\\n', np.around(PG,2))\n"
        "\n"
        "Pl = np.dot(RGl.T, PG)\n"
        "print('Position at the local coordinate system using the inverse of RGl (Pl=RlG*PG):\\n', Pl)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Rotation matrix (RGl):\n"
        " [[ 0.7071 -0.7071]\n"
        " [ 0.7071  0.7071]]\n"
        "Position at the local coordinate system (Pl):\n"
        " [[1]\n"
        " [1]]\n"
        "Position at the Global coordinate system (PG=RGl*Pl):\n"
        " [[0.  ]\n"
        " [1.41]]\n"
        "Position at the local coordinate system using the inverse of RGl (Pl=RlG*PG):\n"
        " [[1.]\n"
        " [1.]]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "**In summary, some of the properties of the rotation matrix are:**  \n"
        "1. The columns of the rotation matrix form a basis of (independent) unit vectors (versors) and the rows are also independent versors since the transpose of the rotation matrix is another rotation matrix. \n"
        "2. The rotation matrix is orthogonal. There is no linear combination of one of the lines or columns of the matrix that would lead to the other row or column, i.e., the lines and columns of the rotation matrix are independent, orthogonal, to each other (this is property 1 rewritten). Because each row and column have norm equal to one, this matrix is also sometimes said to be orthonormal. \n"
        "3. The determinant of the rotation matrix is equal to one (or equal to -1 if a left-hand coordinate system was used, but you should rarely use that). For instance, the determinant of the rotation matrix we deduced is $cos\\alpha cos\\alpha - sin\\alpha(-sin\\alpha)=1$.\n"
        "4. The inverse of the rotation matrix is equals to its transpose.\n"
        "\n"
        "**On the different meanings of the rotation matrix:**  \n"
        "- It represents the coordinate transformation between the coordinates of a point expressed in two different coordinate systems.  \n"
        "- It describes the rotation between two coordinate systems. The columns are the direction cosines (versors) of the axes of the rotated coordinate system in relation to the other coordinate system and the rows are also direction cosines (versors) for the inverse rotation.  \n"
        "- It is an operator for the calculation of the rotation of a vector in a coordinate system.\n"
        "- Rotation matrices provide a means of numerically representing rotations without appealing to angular specification.\n"
        "\n"
        "**Which matrix to use, from local to Global or Global to local?**  \n"
        "- A typical use of the transformation is in movement analysis, where there are the fixed Global (laboratory) coordinate system and the local (moving, e.g. anatomical) coordinate system attached to each body segment. Because the movement of the body segment is measured in the Global coordinate system, using cameras for example, and we want to reconstruct the coordinates of the markers at the anatomical coordinate system, we want the transformation leading from the Global coordinate system to the local coordinate system.\n"
        "- Of course, if you have one matrix, it is simple to get the other; you just have to pay attention to use the right one."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Translation and rotation\n"
        "\n"
        "Consider now the case where the local coordinate system is translated and rotated in relation to the Global coordinate system and a point is described in both coordinate systems as illustrated in the figure below (once again, remember that this is equivalent to describing a translation and a rotation between two rigid bodies).  \n"
        "<br>\n"
        "<figure><img src='./../images/transrot2D.png' alt='translation and rotation 2D'/> <figcaption><center><i>Figure. A point in two-dimensional space represented in two coordinate systems, with one system translated and rotated.</i></center></figcaption> </figure>\n"
        "\n"
        "The position of point $\\mathbf{P}$ originally described in the local coordinate system, but now described in the Global coordinate system in vector form is:\n"
        "\n"
        "$$ \\mathbf{P_G} = \\mathbf{L_G} + \\mathbf{R_{Gl}}\\mathbf{P_l} $$\n"
        "\n"
        "And in matrix form:\n"
        "\n"
        "$$ \\begin{bmatrix}\n"
        "\\mathbf{P_X} \\\\\n"
        "\\mathbf{P_Y} \n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix} \\mathbf{L_{X}} \\\\\\ \\mathbf{L_{Y}} \\end{bmatrix} + \n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha \\\\\n"
        "\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} \\begin{bmatrix}\n"
        "\\mathbf{P}_x \\\\\n"
        "\\mathbf{P}_y \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "This means that we first *disrotate* the local coordinate system and then correct for the translation between the two coordinate systems. Note that we can't invert this order: the point position is expressed in the local coordinate system and we can't add this vector to another vector expressed in the Global coordinate system, first we have to convert the vectors to the same coordinate system.\n"
        "\n"
        "If now we want to find the position of a point at the local coordinate system given its position in the Global coordinate system, the rotation matrix and the translation vector, we have to invert the expression above:\n"
        "\n"
        "$$ \\begin{array}{l l}\n"
        "\\mathbf{P_G} = \\mathbf{L_G} + \\mathbf{R_{Gl}}\\mathbf{P_l} \\implies \\\\\n"
        "\\\\\n"
        "\\mathbf{R_{Gl}^{-1}}(\\mathbf{P_G} - \\mathbf{L_G}) = \\mathbf{R_{Gl}^{-1}}\\mathbf{R_{Gl}}\\mathbf{P_l} \\implies \\\\\n"
        "\\\\\n"
        "\\mathbf{P_l} = \\mathbf{R_{Gl}^{-1}}\\left(\\mathbf{P_G}-\\mathbf{L_G}\\right) = \\mathbf{R_{Gl}^T}\\left(\\mathbf{P_G}-\\mathbf{L_G}\\right) \\quad \\text{or} \\quad \\mathbf{P_l} = \\mathbf{R_{lG}}\\left(\\mathbf{P_G}-\\mathbf{L_G}\\right) \n"
        "\\end{array} $$\n"
        "\n"
        "The expression above indicates that to perform the inverse operation, to go from the Global to the local coordinate system, we first translate and then rotate the coordinate system."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Transformation matrix\n"
        "\n"
        "It is possible to combine the translation and rotation operations in only one matrix, called the transformation matrix (also referred as homogeneous transformation matrix):\n"
        "\n"
        "$$ \\begin{bmatrix}\n"
        "\\mathbf{P_X} \\\\\n"
        "\\mathbf{P_Y} \\\\\n"
        "1\n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha & \\mathbf{L_{X}} \\\\\n"
        "\\sin\\alpha & \\cos\\alpha  & \\mathbf{L_{Y}} \\\\\n"
        "0 & 0 & 1\n"
        "\\end{bmatrix} \\begin{bmatrix}\n"
        "\\mathbf{P}_x \\\\\n"
        "\\mathbf{P}_y \\\\\n"
        "1\n"
        "\\end{bmatrix} $$\n"
        "\n"
        "Or simply:\n"
        "\n"
        "$$ \\mathbf{P_G} = \\mathbf{T_{Gl}}\\mathbf{P_l} $$\n"
        "\n"
        "The inverse operation, to express the position at the local coordinate system in terms of the Global coordinate system, is:\n"
        "\n"
        "$$ \\mathbf{P_l} = \\mathbf{T_{Gl}^{-1}}\\mathbf{P_G} $$\n"
        "\n"
        "However, because $\\mathbf{T_{Gl}}$ is not orthonormal when there is a translation, its inverse is not its transpose. Its inverse in matrix form is given by:\n"
        "\n"
        "$$ \\begin{bmatrix}\n"
        "\\mathbf{P}_x \\\\\n"
        "\\mathbf{P}_y \\\\\n"
        "1\n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "\\mathbf{R^{-1}_{Gl}} & \\cdot & - \\mathbf{R^{-1}_{Gl}}\\mathbf{L_{G}} \\\\\n"
        "\\cdot & \\cdot  & \\cdot \\\\\n"
        "0 & 0 & 1\n"
        "\\end{bmatrix} \\begin{bmatrix}\n"
        "\\mathbf{P_X} \\\\\n"
        "\\mathbf{P_Y} \\\\\n"
        "1\n"
        "\\end{bmatrix} $$"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Calculation of a basis\n"
        "\n"
        "A typical scenario in motion analysis is to calculate the rotation matrix using the position of markers placed on the moving rigid body. With the markers' positions, we create a local basis, which by definition is the rotation matrix for the rigid body with respect to the Global (laboratory) coordinate system. To define a coordinate system using a basís, we also will need to define an origin. \n"
        "\n"
        "Let's see how to calculate a basis given the markers' positions.   \n"
        "Consider the markers at m1=[1,1]`, m2=[1,2] and m3=[-1,1] measured in the Global coordinate system as illustrated in the figure below:  \n"
        "<br>\n"
        "<figure><img src='./../images/transrot2Db.png' alt='translation and rotation 2D'/> <figcaption><center><i>Figure. Three points in the two-dimensional space, two possible vectors given these points, and the corresponding basis.</i></center></figcaption> </figure>\n"
        "\n"
        "A possible local coordinate system with origin at the position of m1 is also illustrated in the figure above. Intentionally, the three markers were chosen to form orthogonal vectors.   \n"
        "The translation vector between the two coordinate system is:\n"
        "\n"
        "$$\\mathbf{L_{Gl}} = m_1 - [0,0] = [1,1]$$\n"
        "\n"
        "The vectors expressing the axes of the local coordinate system are:\n"
        "\n"
        "$$ x = m_2 - m_1 = [1,2] - [1,1] = [0,1] $$\n"
        "\n"
        "$$ y = m_3 - m_1 = [-1,1] - [1,1] = [-2,0] $$\n"
        "\n"
        "Note that these two vectors do not form a basis yet because they are not unit vectors (in fact, only *y* is not a unit vector). Let's normalize these vectors:\n"
        "\n"
        "$$ \\begin{array}{}\n"
        "e_x = \\frac{x}{||x||} = \\frac{[0,1]}{\\sqrt{0^2+1^2}} = [0,1] \\\\\n"
        "\\\\\n"
        "e_y = \\frac{y}{||y||} = \\frac{[-2,0]}{\\sqrt{2^2+0^2}} = [-1,0] \n"
        "\\end{array} $$\n"
        "\n"
        "Beware that the versors above are not exactly the same as the ones shown in the right plot of the last figure, the versors above if plotted will start at the origin of the coordinate system, not at [1,1] as shown in the figure.\n"
        "\n"
        "We could have done this calculation in NumPy (we will need to do that when dealing with real data later):"
    ));

    qDebug() << "command entry 10";
    testCommandEntry(entry, 10, 1, QString::fromUtf8(
        "m1 = np.array([1.,1.])    # marker 1\n"
        "m2 = np.array([1.,2.])    # marker 2\n"
        "m3 = np.array([-1.,1.])   # marker 3\n"
        "\n"
        "x = m2 - m1               # vector x\n"
        "y = m3 - m1               # vector y\n"
        "\n"
        "vx = x/np.linalg.norm(x)  # versor x\n"
        "vy = y/np.linalg.norm(y)  # verson y\n"
        "\n"
        "print(\"x =\", x, \", y =\", y, \"\\nex=\", vx, \", ey=\", vy)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "x = [0. 1.] , y = [-2.  0.] \n"
        "ex= [0. 1.] , ey= [-1.  0.]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Now, both $\\mathbf{e}_x$ and $\\mathbf{e}_y$ are unit vectors (versors) and they are orthogonal, a basis can be formed with these two versors, and we can represent the rotation matrix using this basis (just place the versors of this basis as columns of the rotation matrix):\n"
        "\n"
        "$$ \\mathbf{R_{Gl}} = \\begin{bmatrix}\n"
        "0 & -1 \\\\\n"
        "1 & 0 \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "This rotation matrix makes sense because from the figure above we see that the local coordinate system we defined is rotated by 90$^o$ in relation to the Global coordinate system and if we use the general form for the rotation matrix:\n"
        "\n"
        "$$ \\mathbf{R} = \\begin{bmatrix}\n"
        "\\cos\\alpha & -\\sin\\alpha \\\\\n"
        "\\sin\\alpha & \\cos\\alpha \n"
        "\\end{bmatrix} = \n"
        "\\begin{bmatrix}\n"
        "\\cos90^o & -\\sin90^o \\\\\n"
        "\\sin90^o & \\cos90^o \n"
        "\\end{bmatrix} =\n"
        "\\begin{bmatrix}\n"
        "0 & -1 \\\\\n"
        "1 & 0 \n"
        "\\end{bmatrix} $$\n"
        "\n"
        "So, the position of any point in the local coordinate system can be represented in the Global coordinate system by:\n"
        "\n"
        "$$ \\begin{array}{l l}\n"
        "\\mathbf{P_G} =& \\mathbf{L_{Gl}} + \\mathbf{R_{Gl}}\\mathbf{P_l} \\\\\n"
        "\\\\\n"
        "\\mathbf{P_G} =& \\begin{bmatrix} 1 \\\\ 1 \\end{bmatrix} + \\begin{bmatrix} 0 & -1 \\\\ 1 & 0 \\end{bmatrix} \\mathbf{P_l} \n"
        "\\end{array} $$\n"
        "\n"
        "For example, the point $\\mathbf{P_l}=[1,1]$ has the following position at the Global coordinate system:"
    ));

    qDebug() << "command entry 11";
    testCommandEntry(entry, 11, 1, QString::fromUtf8(
        "LGl = np.array([[1, 1]]).T\n"
        "print('Translation vector:\\n', LGl)\n"
        "\n"
        "RGl = np.array([[0, -1], [1, 0]])\n"
        "print('Rotation matrix:\\n', RGl)\n"
        "\n"
        "Pl  = np.array([[1, 1]]).T\n"
        "print('Position at the local coordinate system:\\n', Pl)\n"
        "\n"
        "PG = LGl + np.dot(RGl, Pl)\n"
        "print('Position at the Global coordinate system, PG = LGl + RGl*Pl:\\n', PG)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Translation vector:\n"
        " [[1]\n"
        " [1]]\n"
        "Rotation matrix:\n"
        " [[ 0 -1]\n"
        " [ 1  0]]\n"
        "Position at the local coordinate system:\n"
        " [[1]\n"
        " [1]]\n"
        "Position at the Global coordinate system, PG = LGl + RGl*Pl:\n"
        " [[0]\n"
        " [2]]"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "### Determination of the unknown angle of rotation\n"
        "\n"
        "If we didn't know the angle of rotation between the two coordinate systems, which is the typical situation in motion analysis, we simply would equate one of the terms of the two-dimensional rotation matrix in its algebraic form to its correspondent value in the numerical rotation matrix we calculated.\n"
        "\n"
        "For instance, taking the first term of the rotation matrices above: $\\cos\\alpha = 0$ implies that $\\theta$ is 90$^o$ or 270$^o$, but combining with another matrix term, $\\sin\\alpha = 1$, implies that $\\alpha=90^o$. We can solve this problem in one step using the tangent $(\\sin\\alpha/\\cos\\alpha)$ function with two terms of the rotation matrix and calculating the angle with the `arctan2(y, x)` function:"
    ));

    qDebug() << "command entry 12";
    testCommandEntry(entry, 12, 1, QString::fromUtf8(
        "ang = np.arctan2(RGl[1, 0], RGl[0, 0])*180/np.pi\n"
        "print('The angle is:', ang)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "The angle is: 90.0"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "And this procedure would be repeated for each segment and for each instant of the analyzed movement to find the rotation of each segment."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "#### Joint angle as a sequence of rotations of adjacent segments\n"
        "\n"
        "In the notebook about [two-dimensional angular kinematics](http://nbviewer.ipython.org/github/demotu/BMC/blob/master/notebooks/AngularKinematics2D.ipynb), we calculated segment and joint angles using simple trigonometric relations. We can also calculate these two-dimensional angles using what we learned here about the rotation matrix.\n"
        "\n"
        "The segment angle will be given by the matrix representing the rotation from the laboratory coordinate system (G) to a coordinate system attached to the segment and the joint angle will be given by the matrix representing the rotation from one segment coordinate system (l1) to the other segment coordinate system (l2). So, we have to calculate two basis now, one for each segment and the joint angle will be given by the product between the two rotation matrices.  \n"
        "\n"
        "To define a two-dimensional basis, we need to calculate vectors perpendicular to each of these lines. Here is a way of doing that. First, let's find three non-collinear points for each basis:"
    ));

    qDebug() << "command entry 13";
    testCommandEntry(entry, 13, QString::fromUtf8(
        "x1, y1, x2, y2 = 0, 0, 1, 1      # points at segment 1\n"
        "x3, y3, x4, y4 = 1.1, 1, 2.1, 0  # points at segment 2\n"
        "\n"
        "#The slope of the perpendicular line is minus the inverse of the slope of the line\n"
        "xl1 = x1 - (y2-y1); yl1 = y1 + (x2-x1)  # point at the perpendicular line 1\n"
        "xl2 = x4 - (y3-y4); yl2 = y4 + (x3-x4)  # point at the perpendicular line 2"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "With these three points, we can create a basis and the corresponding rotation matrix:"
    ));

    qDebug() << "command entry 14";
    testCommandEntry(entry, 14, QString::fromUtf8(
        "b1x = np.array([x2-x1, y2-y1])\n"
        "b1x = b1x/np.linalg.norm(b1x)    # versor x of basis 1\n"
        "b1y = np.array([xl1-x1, yl1-y1])\n"
        "b1y = b1y/np.linalg.norm(b1y)    # versor y of basis 1\n"
        "b2x = np.array([x3-x4, y3-y4])\n"
        "b2x = b2x/np.linalg.norm(b2x)    # versor x of basis 2\n"
        "b2y = np.array([xl2-x4, yl2-y4])\n"
        "b2y = b2y/np.linalg.norm(b2y)    # versor y of basis 2\n"
        "\n"
        "RGl1 = np.array([b1x, b1y]).T    # rotation matrix from segment 1 to the laboratory\n"
        "RGl2 = np.array([b2x, b2y]).T    # rotation matrix from segment 2 to the laboratory"
    ));

    testMarkdown(entry, QString::fromUtf8(
        "Now, the segment and joint angles are simply matrix operations:"
    ));

    qDebug() << "command entry 15";
    testCommandEntry(entry, 15, 1, QString::fromUtf8(
        "print('Rotation matrix for segment 1:\\n', np.around(RGl1, 4))\n"
        "print('\\nRotation angle of segment 1:', np.arctan2(RGl1[1,0], RGl1[0,0])*180/np.pi)\n"
        "print('\\nRotation matrix for segment 2:\\n', np.around(RGl2, 4))\n"
        "print('\\nRotation angle of segment 2:', np.arctan2(RGl1[1,0], RGl2[0,0])*180/np.pi)\n"
        "\n"
        "Rl1l2 = np.dot(RGl1.T, RGl2)  # Rl1l2 = Rl1G*RGl2\n"
        "\n"
        "print('\\nJoint rotation matrix (Rl1l2 = Rl1G*RGl2):\\n', np.around(Rl1l2, 4))\n"
        "print('\\nJoint angle:', np.arctan2(Rl1l2[1,0], Rl1l2[0,0])*180/np.pi)"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Rotation matrix for segment 1:\n"
        " [[ 0.7071 -0.7071]\n"
        " [ 0.7071  0.7071]]\n"
        "\n"
        "Rotation angle of segment 1: 45.0\n"
        "\n"
        "Rotation matrix for segment 2:\n"
        " [[-0.7071 -0.7071]\n"
        " [ 0.7071 -0.7071]]\n"
        "\n"
        "Rotation angle of segment 2: 135.0\n"
        "\n"
        "Joint rotation matrix (Rl1l2 = Rl1G*RGl2):\n"
        " [[ 0. -1.]\n"
        " [ 1. -0.]]\n"
        "\n"
        "Joint angle: 90.0"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Same result as obtained in [Angular kinematics in a plane (2D)](http://nbviewer.ipython.org/github/demotu/BMC/blob/master/notebooks/AngularKinematics2D.ipynb). "
    ));

    testMarkdown(entry, QString::fromUtf8(
        "### Kinematic chain in a plain (2D)\n"
        "\n"
        "The fact that we simply multiplied the rotation matrices to calculate the rotation matrix of one segment in relation to the other is powerful and can be generalized for any number of segments: given a serial kinematic chain with links 1, 2, ..., n and 0 is the base/laboratory, the rotation matrix between the base and last link is: $\\mathbf{R_{n,n-1}R_{n-1,n-2} \\dots R_{2,1}R_{1,0}}$, where each matrix in this product (calculated from right to left) is the rotation of one link with respect to the next one.  \n"
        "\n"
        "For instance, consider a kinematic chain with two links, the link 1 is rotated by $\\alpha_1$ with respect to the base (0) and the link 2 is rotated by $\\alpha_2$ with respect to the link 1.  \n"
        "Using Sympy, the rotation matrices for link 2 w.r.t. link 1 $(R_{12})$ and for link 1 w.r.t. base 0 $(R_{01})$ are: "
    ));

    qDebug() << "command entry 16";
    testCommandEntry(entry, 16, QString::fromUtf8(
        "from IPython.display import display, Math\n"
        "from sympy import sin, cos, Matrix, simplify, latex, symbols\n"
        "from sympy.interactive import printing\n"
        "printing.init_printing()"
    ));

    qDebug() << "command entry 17";
    testCommandEntry(entry, 17, 2, QString::fromUtf8(
        "a1, a2 = symbols('alpha1 alpha2')\n"
        "\n"
        "R12 = Matrix([[cos(a2), -sin(a2)], [sin(a2), cos(a2)]])\n"
        "display(Math(latex(r'\\mathbf{R_{12}}=') + latex(R12)))\n"
        "R01 = Matrix([[cos(a1), -sin(a1)], [sin(a1), cos(a1)]])\n"
        "display(Math(latex(r'\\mathbf{R_{01}}=') + latex(R01)))"
    ));
    {
    QCOMPARE(expression(entry)->results()[0]->type(), (int)Cantor::LatexResult::Type);
    Cantor::LatexResult* result = static_cast<Cantor::LatexResult*>(expression(entry)->results()[0]);
    QCOMPARE(result->code(), QLatin1String(
        "$$\\mathbf{R_{12}}=\\left[\\begin{matrix}\\cos{\\left(\\alpha_{2} \\right)} & - \\sin{\\left(\\alpha_{2} \\right)}\\\\\\sin{\\left(\\alpha_{2} \\right)} & \\cos{\\left(\\alpha_{2} \\right)}\\end{matrix}\\right]$$"
    ));
    QCOMPARE(result->plain(), QLatin1String(
        "<IPython.core.display.Math object>"
    ));
    QCOMPARE(result->mimeType(), QStringLiteral("image/x-eps"));
    }
    {
    QCOMPARE(expression(entry)->results()[1]->type(), (int)Cantor::LatexResult::Type);
    Cantor::LatexResult* result = static_cast<Cantor::LatexResult*>(expression(entry)->results()[1]);
    QCOMPARE(result->code(), QLatin1String(
        "$$\\mathbf{R_{01}}=\\left[\\begin{matrix}\\cos{\\left(\\alpha_{1} \\right)} & - \\sin{\\left(\\alpha_{1} \\right)}\\\\\\sin{\\left(\\alpha_{1} \\right)} & \\cos{\\left(\\alpha_{1} \\right)}\\end{matrix}\\right]$$"
    ));
    QCOMPARE(result->plain(), QLatin1String(
        "<IPython.core.display.Math object>"
    ));
    QCOMPARE(result->mimeType(), QStringLiteral("image/x-eps"));
    }
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "The rotation matrix of link 2 w.r.t. the base $(R_{02})$ is given simply by $R_{01}*R_{12}$:"
    ));

    qDebug() << "command entry 18";
    testCommandEntry(entry, 18, 1, QString::fromUtf8(
        "R02 = R01*R12\n"
        "display(Math(latex(r'\\mathbf{R_{02}}=') + latex(R02)))"
    ));
    {
    QCOMPARE(expression(entry)->results()[0]->type(), (int)Cantor::LatexResult::Type);
    Cantor::LatexResult* result = static_cast<Cantor::LatexResult*>(expression(entry)->results()[0]);
    QCOMPARE(result->code(), QLatin1String(
        "$$\\mathbf{R_{02}}=\\left[\\begin{matrix}- \\sin{\\left(\\alpha_{1} \\right)} \\sin{\\left(\\alpha_{2} \\right)} + \\cos{\\left(\\alpha_{1} \\right)} \\cos{\\left(\\alpha_{2} \\right)} & - \\sin{\\left(\\alpha_{1} \\right)} \\cos{\\left(\\alpha_{2} \\right)} - \\sin{\\left(\\alpha_{2} \\right)} \\cos{\\left(\\alpha_{1} \\right)}\\\\\\sin{\\left(\\alpha_{1} \\right)} \\cos{\\left(\\alpha_{2} \\right)} + \\sin{\\left(\\alpha_{2} \\right)} \\cos{\\left(\\alpha_{1} \\right)} & - \\sin{\\left(\\alpha_{1} \\right)} \\sin{\\left(\\alpha_{2} \\right)} + \\cos{\\left(\\alpha_{1} \\right)} \\cos{\\left(\\alpha_{2} \\right)}\\end{matrix}\\right]$$"
    ));
    QCOMPARE(result->plain(), QLatin1String(
        "<IPython.core.display.Math object>"
    ));
    QCOMPARE(result->mimeType(), QStringLiteral("image/x-eps"));
    }
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "Which simplifies to:"
    ));

    qDebug() << "command entry 19";
    testCommandEntry(entry, 19, 1, QString::fromUtf8(
        "display(Math(latex(r'\\mathbf{R_{02}}=') + latex(simplify(R02))))"
    ));
    {
    QCOMPARE(expression(entry)->results()[0]->type(), (int)Cantor::LatexResult::Type);
    Cantor::LatexResult* result = static_cast<Cantor::LatexResult*>(expression(entry)->results()[0]);
    QCOMPARE(result->code(), QLatin1String(
        "$$\\mathbf{R_{02}}=\\left[\\begin{matrix}\\cos{\\left(\\alpha_{1} + \\alpha_{2} \\right)} & - \\sin{\\left(\\alpha_{1} + \\alpha_{2} \\right)}\\\\\\sin{\\left(\\alpha_{1} + \\alpha_{2} \\right)} & \\cos{\\left(\\alpha_{1} + \\alpha_{2} \\right)}\\end{matrix}\\right]$$"
    ));
    QCOMPARE(result->plain(), QLatin1String(
        "<IPython.core.display.Math object>"
    ));
    QCOMPARE(result->mimeType(), QStringLiteral("image/x-eps"));
    }
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        "As expected.\n"
        "\n"
        "The typical use of all these concepts is in the three-dimensional motion analysis where we will have to deal with angles in different planes, which needs a special manipulation as we will see next."
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## Problems\n"
        "\n"
        "1. A local coordinate system is rotated 30$^o$ clockwise in relation to the Global reference system.   \n"
        "  A. Determine the matrices for rotating one coordinate system to another (two-dimensional).   \n"
        "  B. What are the coordinates of the point [1, 1] (local coordinate system) at the global coordinate system?   \n"
        "  C. And if this point is at the Global coordinate system and we want the coordinates at the local coordinate system?   \n"
        "  D. Consider that the local coordinate system, besides the rotation is also translated by [2, 2]. What are the matrices for rotation, translation, and transformation from one coordinate system to another (two-dimensional)?   \n"
        "  E. Repeat B and C considering this translation.\n"
        "  \n"
        "2. Consider a local coordinate system U rotated 45$^o$ clockwise in relation to the Global reference system and another local coordinate system V rotated 45$^o$ clockwise in relation to the local reference system U.  \n"
        "  A. Determine the rotation matrices of all possible transformations between the coordinate systems.   \n"
        "  B. For the point [1, 1] in the coordinate system U, what are its coordinates in coordinate system V and in the Global coordinate system?   \n"
        "  \n"
        "3. Using the rotation matrix, deduce the new coordinates of a square figure with coordinates [0, 0], [1, 0], [1, 1], and [0, 1] when rotated by 0$^o$, 45$^o$, 90$^o$, 135$^o$, and 180$^o$ (always clockwise).\n"
        "  \n"
        "4. Solve the problem 2 of [Angular kinematics in a plane (2D)](http://nbviewer.ipython.org/github/demotu/BMC/blob/master/notebooks/AngularKinematics2D.ipynb) but now using the concept of two-dimensional transformations.  "
    ));

    testMarkdown(entry, QString::fromUtf8(
        "## References\n"
        "\n"
        "- Robertson G, Caldwell G, Hamill J, Kamen G (2013) [Research Methods in Biomechanics](http://books.google.com.br/books?id=gRn8AAAAQBAJ). 2nd Edition. Human Kinetics.      \n"
        "- Ruina A, Rudra P (2013) [Introduction to Statics and Dynamics](http://ruina.tam.cornell.edu/Book/index.html). Oxford University Press.  \n"
        "- Winter DA (2009) [Biomechanics and motor control of human movement](http://books.google.com.br/books?id=_bFHL08IWfwC). 4 ed. Hoboken, EUA: Wiley.  \n"
        "- Zatsiorsky VM (1997) [Kinematics of Human Motion](http://books.google.com.br/books/about/Kinematics_of_Human_Motion.html?id=Pql_xXdbrMcC&redir_esc=y). Champaign, Human Kinetics."
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testMarkdownAttachment()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("TestMarkdownAttachment.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testCommandEntry(entry, 1, 1, QString::fromUtf8(
        "2+2"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "4"
    ));
    entry = entry->next();

    // Tests attachments via toJupyterJson: ugly, but works
    QVERIFY(entry);
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QString::fromLatin1(
        "![CantorLogo.png](attachment:CantorLogo.png)\n"
        "![CantorLogo.png](attachment:CantorLogo.png)"
    ));
    QJsonValue value = entry->toJupyterJson();
    QVERIFY(value.isObject());
    QVERIFY(value.toObject().contains(QLatin1String("attachments")));
    entry = entry->next();

    QVERIFY(entry);
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QString::fromLatin1(
        "![CantorLogo.png](attachment:CantorLogo.png)"
    ));
    value = entry->toJupyterJson();
    QVERIFY(value.isObject());
    QVERIFY(value.toObject().contains(QLatin1String("attachments")));
    entry = entry->next();

    testCommandEntry(entry, -1, QString::fromUtf8(
        ""
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testEntryLoad1()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("TestEntryLoad1.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testCommandEntry(entry, 2, 1, QString::fromUtf8(
        "2+2"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "4"
    ));
    entry = entry->next();

    testLatexEntry(entry, QString::fromLatin1(
       "$$\\Gamma$$"
    ));

    testMarkdown(entry, QString::fromLatin1(
        "### Test Entry"
    ));

    testMarkdown(entry, QString::fromLatin1(
        "Text"
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testEntryLoad2()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("TestEntryLoad2.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("octave"));

    WorksheetEntry* entry = w->firstEntry();

    testCommandEntry(entry, 0, 1, QString::fromUtf8(
        "2+2"
    ));
    testTextResult(entry, 0, QString::fromLatin1(
        "ans =  4"
    ));
    entry = entry->next();

    testTextEntry(entry, QString::fromLatin1(
        "Text entry"
    ));

    testMarkdown(entry, QString::fromLatin1(
        "#### Markdown entry"
    ));

    testLatexEntry(entry, QString::fromLatin1(
       "\\LaTeX\\ entry"
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testResultsLoad()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("TestResultsLoad.ipynb")));

    QCOMPARE(w->isReadOnly(), false);
    QCOMPARE(w->session()->backend()->id(), QLatin1String("sage"));

    WorksheetEntry* entry = w->firstEntry();

    testCommandEntry(entry, 9, QString::fromUtf8(
        "from IPython.display import Latex"
    ));

    testCommandEntry(entry, 16, 1, QString::fromUtf8(
        "print(\"Hello world\")"
    ));
    testTextResult(entry, 0, QString::fromUtf8(
        "Hello world"
    ));
    entry = entry->next();

    testCommandEntry(entry, 17, 1, QString::fromUtf8(
        "plot(x^2, (x,0,5))"
    ));
    testImageResult(entry, 0);
    entry = entry->next();

    testCommandEntry(entry, 6, 1, QString::fromUtf8(
        "sines = [plot(c*sin(x), (-2*pi,2*pi), color=Color(c,0,0), ymin=-1, ymax=1) for c in sxrange(0,1,.05)]\n"
        "a = animate(sines)\n"
        "a.show()"
    ));
    QVERIFY(expression(entry));
    QCOMPARE(expression(entry)->results().at(0)->type(), (int)Cantor::AnimationResult::Type);
    QVERIFY(static_cast<Cantor::AnimationResult*>(expression(entry)->results().at(0))->url().isValid());
    entry = entry->next();

    testCommandEntry(entry, 15, 1, QString::fromUtf8(
        "Latex(\"$$\\Gamma$$\")"
    ));
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::LatexResult::Type);
    {
    Cantor::LatexResult* result = static_cast<Cantor::LatexResult*>(expression(entry)->result());
    QCOMPARE(result->code(), QLatin1String(
        "$$\\Gamma$$"
    ));
    QCOMPARE(result->plain(), QLatin1String(
        "<IPython.core.display.Latex object>"
    ));
    QCOMPARE(result->mimeType(), QStringLiteral("image/x-eps"));
    }
    entry = entry->next();

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testMimeResult()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("TestNotebookWithJson.ipynb")));

    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testCommandEntry(entry, 6, QString::fromUtf8(
        "import json\n"
        "import uuid\n"
        "from IPython.display import display_javascript, display_html, display\n"
        "\n"
        "class RenderJSON(object):\n"
        "    def __init__(self, json_data):\n"
        "        if isinstance(json_data, dict) or isinstance(json_data, list):\n"
        "            self.json_str = json.dumps(json_data)\n"
        "        else:\n"
        "            self.json_str = json_data\n"
        "        self.uuid = str(uuid.uuid4())\n"
        "\n"
        "    def _ipython_display_(self):\n"
        "        display_html('<div id=\"{}\" style=\"height: 600px; width:100%;font: 12px/18px monospace !important;\"></div>'.format(self.uuid), raw=True)\n"
        "        display_javascript(\"\"\"\n"
        "        require([\"https://rawgit.com/caldwell/renderjson/master/renderjson.js\"], function() {\n"
        "            renderjson.set_show_to_level(2);\n"
        "            document.getElementById('%s').appendChild(renderjson(%s))\n"
        "        });\n"
        "      \"\"\" % (self.uuid, self.json_str), raw=True)"
    ));

    testCommandEntry(entry, 7, 2, QString::fromUtf8(
        "RenderJSON([\n"
        "    {\n"
        "        \"a\": 1\n"
        "    }, \n"
        "    {\n"
        "        \"b\": 2,\n"
        "        \"in1\": {\n"
        "            \"key\": \"value\"\n"
        "        }\n"
        "    }\n"
        "])"
    ));
    testHtmlResult(entry, 0, QString::fromLatin1(
        ""
    ), QString::fromLatin1(
        "<div id=\"bb6d9031-c990-4aee-849e-6d697430777c\" style=\"height: 600px; width:100%;font: 12px/18px monospace !important;\"></div>"
    ));
    {
    QVERIFY(expression(entry)->results().size() > 1);
    QCOMPARE(expression(entry)->results().at(1)->type(), (int)Cantor::MimeResult::Type);
    Cantor::MimeResult* result = static_cast<Cantor::MimeResult*>(expression(entry)->results().at(1));
    QCOMPARE(result->mimeKey(), QLatin1String("application/javascript"));
    QJsonArray value = QJsonArray::fromStringList(QStringList{
        QLatin1String("\n"),
        QLatin1String("        require([\"https://rawgit.com/caldwell/renderjson/master/renderjson.js\"], function() {\n"),
        QLatin1String("            renderjson.set_show_to_level(2);\n"),
        QLatin1String("            document.getElementById('bb6d9031-c990-4aee-849e-6d697430777c').appendChild(renderjson([{\"a\": 1}, {\"b\": 2, \"in1\": {\"key\": \"value\"}}]))\n"),
        QLatin1String("        });\n"),
        QLatin1String("      ")
    });
    QCOMPARE(result->data().value<QJsonValue>(), QJsonValue(value));
    }
    entry = entry->next();

    testCommandEntry(entry, -1, QString::fromUtf8(
        ""
    ));

    QCOMPARE(entry, nullptr);
}

void WorksheetTest::testMimeResultWithPlain()
{
    QScopedPointer<Worksheet> w(loadWorksheet(QLatin1String("TestNotebookWithModJson.ipynb")));

    QCOMPARE(w->session()->backend()->id(), QLatin1String("python3"));

    WorksheetEntry* entry = w->firstEntry();

    testCommandEntry(entry, 6, QString::fromUtf8(
        "import json\n"
        "import uuid\n"
        "from IPython.display import display_javascript, display_html, display\n"
        "\n"
        "class RenderJSON(object):\n"
        "    def __init__(self, json_data):\n"
        "        if isinstance(json_data, dict) or isinstance(json_data, list):\n"
        "            self.json_str = json.dumps(json_data)\n"
        "        else:\n"
        "            self.json_str = json_data\n"
        "        self.uuid = str(uuid.uuid4())\n"
        "\n"
        "    def _ipython_display_(self):\n"
        "        display_html('<div id=\"{}\" style=\"height: 600px; width:100%;font: 12px/18px monospace !important;\"></div>'.format(self.uuid), raw=True)\n"
        "        display_javascript(\"\"\"\n"
        "        require([\"https://rawgit.com/caldwell/renderjson/master/renderjson.js\"], function() {\n"
        "            renderjson.set_show_to_level(2);\n"
        "            document.getElementById('%s').appendChild(renderjson(%s))\n"
        "        });\n"
        "      \"\"\" % (self.uuid, self.json_str), raw=True)"
    ));

    testCommandEntry(entry, 7, 2, QString::fromUtf8(
        "RenderJSON([\n"
        "    {\n"
        "        \"a\": 1\n"
        "    }, \n"
        "    {\n"
        "        \"b\": 2,\n"
        "        \"in1\": {\n"
        "            \"key\": \"value\"\n"
        "        }\n"
        "    }\n"
        "])"
    ));
    testHtmlResult(entry, 0, QString::fromLatin1(
        ""
    ), QString::fromLatin1(
        "<div id=\"bb6d9031-c990-4aee-849e-6d697430777c\" style=\"height: 600px; width:100%;font: 12px/18px monospace !important;\"></div>"
    ));
    {
    QVERIFY(expression(entry)->results().size() > 1);
    QCOMPARE(expression(entry)->results().at(1)->type(), (int)Cantor::MimeResult::Type);
    Cantor::MimeResult* result = static_cast<Cantor::MimeResult*>(expression(entry)->results().at(1));
    QCOMPARE(result->mimeKey(), QLatin1String("application/javascript"));
    QCOMPARE(result->plain(), QLatin1String("<__main__.RenderJSON at 0x7fa1599c6828>"));
    QJsonArray value = QJsonArray::fromStringList(QStringList{
        QLatin1String("\n"),
        QLatin1String("        require([\"https://rawgit.com/caldwell/renderjson/master/renderjson.js\"], function() {\n"),
        QLatin1String("            renderjson.set_show_to_level(2);\n"),
        QLatin1String("            document.getElementById('bb6d9031-c990-4aee-849e-6d697430777c').appendChild(renderjson([{\"a\": 1}, {\"b\": 2, \"in1\": {\"key\": \"value\"}}]))\n"),
        QLatin1String("        });\n"),
        QLatin1String("      ")
    });
    QCOMPARE(result->data().value<QJsonValue>(), QJsonValue(value));
    }
    entry = entry->next();

    testCommandEntry(entry, -1, QString::fromUtf8(
        ""
    ));

    QCOMPARE(entry, nullptr);
}

QTEST_MAIN( WorksheetTest )
