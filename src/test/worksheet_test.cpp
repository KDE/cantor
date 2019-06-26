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
}

QTEST_MAIN( WorksheetTest )
