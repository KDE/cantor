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

#include "worksheet_test.h"
#include "../worksheet.h"
#include "../session.h"
#include "../worksheetentry.h"
#include "../textentry.h"
#include "../markdownentry.h"
#include "../commandentry.h"
#include "../lib/backend.h"
#include "../lib/expression.h"
#include "../lib/result.h"
#include "../lib/textresult.h"
#include "../lib/imageresult.h"
#include "../lib/latexresult.h"

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

void WorksheetTest::testHTMLTextResult(WorksheetEntry* entry, int index, const QString& content)
{
    QVERIFY(expression(entry));
    QVERIFY(expression(entry)->results().size() > index);
    QCOMPARE(expression(entry)->results().at(index)->type(), (int)Cantor::TextResult::Type);
    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(expression(entry)->results().at(index));
    QVERIFY(result->format() == Cantor::TextResult::HTMLFormat);
    QCOMPARE(result->plain(), content);
}

void WorksheetTest::testHTMLTextResult(WorksheetEntry* entry, int index, const QString& plain, const QString& html)
{
    QVERIFY(expression(entry));
    QVERIFY(expression(entry)->results().size() > index);
    QCOMPARE(expression(entry)->results().at(index)->type(), (int)Cantor::TextResult::Type);
    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(expression(entry)->results().at(index));
    QVERIFY(result->format() == Cantor::TextResult::HTMLFormat);
    QCOMPARE(result->plain(), plain);
    QCOMPARE(result->toHtml(), html);
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
        "Consider a single atom coupled to a single cavity mode, as illustrated in the figure below. If there atom excitation rate $$\\Gamma$$ exceeds the relaxation rate, a population inversion can occur in the atom, and if coupled to the cavity the atom can then act as a photon pump on the cavity."
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
        "$$H = \\hbar \\omega_0 a^\\dagger a + \\frac{1}{2}\\hbar\\omega_a\\sigma_z + \\hbar g\\sigma_x(a^\\dagger + a)$$\n"
        "\n"
        "where $$\\omega_0$$ is the cavity energy splitting, $$\\omega_a$$ is the atom energy splitting and $$g$$ is the atom-cavity interaction strength.\n"
        "\n"
        "In addition to the coherent dynamics the following incoherent processes are also present: \n"
        "\n"
        "1. $$\\kappa$$ relaxation and thermal excitations of the cavity, \n"
        "2. $$\\Gamma$$ atomic excitation rate (pumping process).\n"
        "\n"
        "The Lindblad master equation for the model is:\n"
        "\n"
        "$$\\frac{d}{dt}\\rho = -i[H, \\rho] + \\Gamma\\left(\\sigma_+\\rho\\sigma_- - \\frac{1}{2}\\sigma_-\\sigma_+\\rho - \\frac{1}{2}\\rho\\sigma_-\\sigma_+\\right)\n"
        "+ \\kappa (1 + n_{\\rm th}) \\left(a\\rho a^\\dagger - \\frac{1}{2}a^\\dagger a\\rho - \\frac{1}{2}\\rho a^\\dagger a\\right)\n"
        "+ \\kappa n_{\\rm th} \\left(a^\\dagger\\rho a - \\frac{1}{2}a a^\\dagger \\rho - \\frac{1}{2}\\rho a a^\\dagger\\right)$$\n"
        "\n"
        "in units where $$\\hbar = 1$$.\n"
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
        "Here we evolve the system with the Lindblad master equation solver, and we request that the expectation values of the operators $$a^\\dagger a$$ and $$\\sigma_+\\sigma_-$$ are returned by the solver by passing the list `[a.dag()*a, sm.dag()*sm]` as the fifth argument to the solver."
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
        "Here we see that lasing is suppressed for $$\\Gamma\\kappa/(4g^2) > 1$$. \n"
        "\n"
        "\n"
        "Let's look at the fock-state distribution at $$\\Gamma\\kappa/(4g^2) = 0.5$$  (lasing regime) and $$\\Gamma\\kappa/(4g^2) = 1.5$$ (suppressed regime):"
    ));

    entry = entry->next();
    QCOMPARE(entry->type(), (int)MarkdownEntry::Type);
    QCOMPARE(plainMarkdown(entry), QLatin1String(
        "### Case 1: $$\\Gamma\\kappa/(4g^2) = 0.5$$"
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
        "### Case 2: $$\\Gamma\\kappa/(4g^2) = 1.5$$"
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
        "Too large pumping rate $$\\Gamma$$ kills the lasing process: reversed threshold."
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
    QCOMPARE(expression(entry)->result()->type(), (int)Cantor::TextResult::Type);
    {
    Cantor::TextResult* result = static_cast<Cantor::TextResult*>(expression(entry)->result());
    QVERIFY(result->format() == Cantor::TextResult::HTMLFormat);
    QCOMPARE(result->plain(), QLatin1String("<IPython.core.display.HTML at 0x7f2f2d5a0048>"));
    QCOMPARE(result->data().toString(), QLatin1String(
        "<table><tr><th>Software</th><th>Version</th></tr><tr><td>IPython</td><td>2.0.0</td></tr><tr><td>OS</td><td>posix [linux]</td></tr><tr><td>Python</td><td>3.4.1 (default, Jun  9 2014, 17:34:49) \n"
        "[GCC 4.8.3]</td></tr><tr><td>QuTiP</td><td>3.0.0.dev-5a88aa8</td></tr><tr><td>Numpy</td><td>1.8.1</td></tr><tr><td>matplotlib</td><td>1.3.1</td></tr><tr><td>Cython</td><td>0.20.1post0</td></tr><tr><td>SciPy</td><td>0.13.3</td></tr><tr><td colspan='2'>Thu Jun 26 14:28:35 2014 JST</td></tr></table>"
    ));
    }

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
        "    * population mean: $$\\mu = \\operatorname{E}[X]$$.\n"
        "    * estimation of sample mean: $$\\bar{x}$$.\n"
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
        "> Optimum in $$\\mathbf{x}=\\mathbf{0}$$, $$f(\\mathbf{x})=0$$."
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
        "## ($$\\mu$$,$$\\lambda$$) evolutionary strategy\n"
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
        "the individual is mutated by a normal distribution of mean 0 and standard deviation of $$\\boldsymbol{\\sigma}_{t}$$ (its current strategy). \n"
        "\n"
        "A recommended choice is $$c=1$$ when using a $$(10,100)$$ evolution strategy."
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
    testHTMLTextResult(entry, 1, QString::fromUtf8(
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
        "What happens if we increase $$\\mu$$ and $$\\lambda$$?"
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
        "* In an evolution strategy, new candidate solutions are sampled according to a multivariate normal distribution in the $$\\mathbb{R}^n$$. \n"
        "* Recombination amounts to selecting a new mean value for the distribution. \n"
        "* Mutation amounts to adding a random vector, a perturbation with zero mean. \n"
        "* Pairwise dependencies between the variables in the distribution are represented by a covariance matrix. \n"
        "\n"
        "### The covariance matrix adaptation (CMA) is a method to update the covariance matrix of this distribution. \n"
        "\n"
        "> This is particularly useful, if the objective function $$f()$$ is ill-conditioned."
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
        "We will place our start point by hand at $$(5,5)$$."
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
    testHTMLTextResult(entry, 1, QString::fromUtf8(
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
    testHTMLTextResult(entry, 0, QString::fromLatin1(
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
    testHTMLTextResult(entry, 0, QString::fromUtf8(
        "<IPython.core.display.HTML object>"
    ));
    entry = entry->next();

    testMarkdown(entry, QString::fromUtf8(
        " "
    ));

    QCOMPARE(entry, nullptr);
}

QTEST_MAIN( WorksheetTest )
