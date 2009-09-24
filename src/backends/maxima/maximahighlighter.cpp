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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximahighlighter.h"

#include <QTextEdit>

MaximaHighlighter::MaximaHighlighter(QTextEdit* edit) : Cantor::DefaultHighlighter(edit)
{
    HighlightingRule rule;

    //initialize the different formats used to highlight
    maximaKeywordFormat.setForeground(Qt::darkBlue);
    maximaKeywordFormat.setFontWeight(QFont::Bold);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);

    maximaFunctionFormat.setForeground(Qt::darkGreen);
    maximaFunctionFormat.setFontWeight(QFont::Bold);

    specialCommentFormat.setForeground(Qt::magenta);
    specialCommentFormat.setFontWeight(QFont::Bold);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);

    //Setup the highlighting rules
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    m_highlightingRules.append(rule);

    //Code highlighting the different keywords
    QStringList maximaKeywordPatterns;
    maximaKeywordPatterns
        <<"\\band\\b" <<"\\bdo\\b" <<"\\belse\\b" <<"\\belseif\\b"
        <<"\\bfalse\\b" <<"\\bfor\\b" <<"\\bif\\b" <<"\\bin\\b"
        <<"\\bnot\\b" <<"\\bor\\b" <<"\\bstep\\b" <<"\\bthen\\b"
        <<"\\bthru\\b" <<"\\btrue\\b" <<"\\bwhile\\b" ;

    foreach (const QString &pattern, maximaKeywordPatterns )
    {
        rule.pattern = QRegExp(pattern);
        rule.format = maximaKeywordFormat;
        m_highlightingRules.append(rule);
    }


    QStringList specialCommentPatterns;
    specialCommentPatterns
        <<"\\bFIXME\\b" <<"\\bTODO\\b" ;

    foreach (const QString &pattern, specialCommentPatterns )
    {
        rule.pattern = QRegExp(pattern);
        rule.format = specialCommentFormat;
        m_highlightingRules.append(rule);
    }


    QStringList maximaFunctionPatterns;
    maximaFunctionPatterns
        <<"\\babasep\\b" <<"\\babs\\b" <<"\\babsint\\b" <<"\\babsolute_real_time\\b"
        <<"\\bacos\\b" <<"\\bacosh\\b" <<"\\bacot\\b" <<"\\bacoth\\b"
        <<"\\bacsc\\b" <<"\\bacsch\\b" <<"\\bactivate\\b" <<"\\baddcol\\b"
        <<"\\badd_edge\\b" <<"\\badd_edges\\b" <<"\\baddmatrices\\b" <<"\\baddrow\\b"
        <<"\\badd_vertex\\b" <<"\\badd_vertices\\b" <<"\\badjacency_matrix\\b" <<"\\badjoin\\b"
        <<"\\badjoint\\b" <<"\\baf\\b" <<"\\bagd\\b" <<"\\bairy_ai\\b"
        <<"\\bairy_bi\\b" <<"\\bairy_dai\\b" <<"\\bairy_dbi\\b" <<"\\balgsys\\b"
        <<"\\balg_type\\b" <<"\\balias\\b" <<"\\ballroots\\b" <<"\\balphacharp\\b"
        <<"\\balphanumericp\\b" <<"\\bantid\\b" <<"\\bantidiff\\b" <<"\\bAntiDifference\\b"
        <<"\\bappend\\b" <<"\\bappendfile\\b" <<"\\bapply\\b" <<"\\bapply1\\b"
        <<"\\bapply2\\b" <<"\\bapplyb1\\b" <<"\\bapropos\\b" <<"\\bargs\\b"
        <<"\\barithmetic\\b" <<"\\barithsum\\b" <<"\\barray\\b" <<"\\barrayapply\\b"
        <<"\\barrayinfo\\b" <<"\\barraymake\\b" <<"\\bascii\\b" <<"\\basec\\b"
        <<"\\basech\\b" <<"\\basin\\b" <<"\\basinh\\b" <<"\\baskinteger\\b"
        <<"\\basksign\\b" <<"\\bassoc\\b" <<"\\bassoc_legendre_p\\b" <<"\\bassoc_legendre_q\\b"
        <<"\\bassume\\b" <<"\\basympa\\b" <<"\\bat\\b" <<"\\batan\\b"
        <<"\\batan2\\b" <<"\\batanh\\b" <<"\\batensimp\\b" <<"\\batom\\b"
        <<"\\batvalue\\b" <<"\\baugcoefmatrix\\b" <<"\\baugmented_lagrangian_method\\b" <<"\\bav\\b"
        <<"\\baverage_degree\\b" <<"\\bbacktrace\\b" <<"\\bbarsplot\\b" <<"\\bbashindices\\b"
        <<"\\bbatch\\b" <<"\\bbatchload\\b" <<"\\bbc2\\b" <<"\\bbdvac\\b"
        <<"\\bbelln\\b" <<"\\bbern\\b" <<"\\bbernpoly\\b" <<"\\bbessel\\b"
        <<"\\bbessel_i\\b" <<"\\bbessel_j\\b" <<"\\bbessel_k\\b" <<"\\bbessel_y\\b"
        <<"\\bbeta\\b" <<"\\bbezout\\b" <<"\\bbffac\\b" <<"\\bbfhzeta\\b"
        <<"\\bbfloat\\b" <<"\\bbfloatp\\b" <<"\\bbfpsi\\b" <<"\\bbfpsi0\\b"
        <<"\\bbfzeta\\b" <<"\\bbiconected_components\\b" <<"\\bbimetric\\b" <<"\\bbinomial\\b"
        <<"\\bbipartition\\b" <<"\\bblock\\b" <<"\\bblockmatrixp\\b" <<"\\bbode_gain\\b"
        <<"\\bbode_phase\\b" <<"\\bbothcoef\\b" <<"\\bbox\\b" <<"\\bboxplot\\b"
        <<"\\bbreak\\b" <<"\\bbug_report\\b" <<"\\bbuild_info\\b" <<"\\bbuildq\\b"
        <<"\\bburn\\b" <<"\\bcabs\\b" <<"\\bcanform\\b" <<"\\bcanten\\b"
        <<"\\bcardinality\\b" <<"\\bcarg\\b" <<"\\bcartan\\b" <<"\\bcartesian_product\\b"
        <<"\\bcatch\\b" <<"\\bcbffac\\b" <<"\\bcdf_bernoulli\\b" <<"\\bcdf_beta\\b"
        <<"\\bcdf_binomial\\b" <<"\\bcdf_cauchy\\b" <<"\\bcdf_chi2\\b" <<"\\bcdf_continuous_uniform\\b"
        <<"\\bcdf_discrete_uniform\\b" <<"\\bcdf_exp\\b" <<"\\bcdf_f\\b" <<"\\bcdf_gamma\\b"
        <<"\\bcdf_geometric\\b" <<"\\bcdf_gumbel\\b" <<"\\bcdf_hypergeometric\\b" <<"\\bcdf_laplace\\b"
        <<"\\bcdf_logistic\\b" <<"\\bcdf_lognormal\\b" <<"\\bcdf_negative_binomial\\b" <<"\\bcdf_normal\\b"
        <<"\\bcdf_pareto\\b" <<"\\bcdf_poisson\\b" <<"\\bcdf_rank_sum\\b" <<"\\bcdf_rayleigh\\b"
        <<"\\bcdf_signed_rank\\b" <<"\\bcdf_student_t\\b" <<"\\bcdf_weibull\\b" <<"\\bcdisplay\\b"
        <<"\\bceiling\\b" <<"\\bcentral_moment\\b" <<"\\bcequal\\b" <<"\\bcequalignore\\b"
        <<"\\bcf\\b" <<"\\bcfdisrep\\b" <<"\\bcfexpand\\b" <<"\\bcgeodesic\\b"
        <<"\\bcgreaterp\\b" <<"\\bcgreaterpignore\\b" <<"\\bchangename\\b" <<"\\bchangevar\\b"
        <<"\\bchaosgame\\b" <<"\\bcharat\\b" <<"\\bcharfun\\b" <<"\\bcharfun2\\b"
        <<"\\bcharlist\\b" <<"\\bcharp\\b" <<"\\bcharpoly\\b" <<"\\bchebyshev_t\\b"
        <<"\\bchebyshev_u\\b" <<"\\bcheckdiv\\b" <<"\\bcheck_overlaps\\b" <<"\\bcholesky\\b"
        <<"\\bchristof\\b" <<"\\bchromatic_index\\b" <<"\\bchromatic_number\\b" <<"\\bcint\\b"
        <<"\\bcirculant_graph\\b" <<"\\bclear_edge_weight\\b" <<"\\bclear_rules\\b" <<"\\bclear_vertex_label\\b"
        <<"\\bclebsch_graph\\b" <<"\\bclessp\\b" <<"\\bclesspignore\\b" <<"\\bclose\\b"
        <<"\\bclosefile\\b" <<"\\bcmetric\\b" <<"\\bcoeff\\b" <<"\\bcoefmatrix\\b"
        <<"\\bcograd\\b" <<"\\bcol\\b" <<"\\bcollapse\\b" <<"\\bcollectterms\\b"
        <<"\\bcolumnop\\b" <<"\\bcolumnspace\\b" <<"\\bcolumnswap\\b" <<"\\bcolumnvector\\b"
        <<"\\bcombination\\b" <<"\\bcombine\\b" <<"\\bcomp2pui\\b" <<"\\bcompare\\b"
        <<"\\bcompfile\\b" <<"\\bcompile\\b" <<"\\bcompile_file\\b" <<"\\bcomplement_graph\\b"
        <<"\\bcomplete_bipartite_graph\\b" <<"\\bcomplete_graph\\b" <<"\\bcomponents\\b" <<"\\bconcan\\b"
        <<"\\bconcat\\b" <<"\\bconjugate\\b" <<"\\bconmetderiv\\b" <<"\\bconnected_components\\b"
        <<"\\bconnect_vertices\\b" <<"\\bcons\\b" <<"\\bconstantp\\b" <<"\\bconstituent\\b"
        <<"\\bcont2part\\b" <<"\\bcontent\\b" <<"\\bcontinuous_freq\\b" <<"\\bcontortion\\b"
        <<"\\bcontour_plot\\b" <<"\\bcontract\\b" <<"\\bcontract_edge\\b" <<"\\bcontragrad\\b"
        <<"\\bcontrib_ode\\b" <<"\\bconvert\\b" <<"\\bcoord\\b" <<"\\bcopy\\b"
        <<"\\bcopy_graph\\b" <<"\\bcopylist\\b" <<"\\bcopymatrix\\b" <<"\\bcor\\b"
        <<"\\bcos\\b" <<"\\bcosh\\b" <<"\\bcot\\b" <<"\\bcoth\\b"
        <<"\\bcov\\b" <<"\\bcov1\\b" <<"\\bcovdiff\\b" <<"\\bcovect\\b"
        <<"\\bcovers\\b" <<"\\bcreate_graph\\b" <<"\\bcreate_list\\b" <<"\\bcsc\\b"
        <<"\\bcsch\\b" <<"\\bcsetup\\b" <<"\\bcspline\\b" <<"\\bctaylor\\b"
        <<"\\bct_coordsys\\b" <<"\\bctransform\\b" <<"\\bctranspose\\b" <<"\\bcube_graph\\b"
        <<"\\bcunlisp\\b" <<"\\bcv\\b" <<"\\bcycle_digraph\\b" <<"\\bcycle_graph\\b"
        <<"\\bdblint\\b" <<"\\bdeactivate\\b" <<"\\bdeclare\\b" <<"\\bdeclare_translated\\b"
        <<"\\bdeclare_weight\\b" <<"\\bdecsym\\b" <<"\\bdefcon\\b" <<"\\bdefine\\b"
        <<"\\bdefine_variable\\b" <<"\\bdefint\\b" <<"\\bdefmatch\\b" <<"\\bdefrule\\b"
        <<"\\bdeftaylor\\b" <<"\\bdegree_sequence\\b" <<"\\bdel\\b" <<"\\bdelete\\b"
        <<"\\bdeleten\\b" <<"\\bdelta\\b" <<"\\bdemo\\b" <<"\\bdemoivre\\b"
        <<"\\bdenom\\b" <<"\\bdepends\\b" <<"\\bderivdegree\\b" <<"\\bderivlist\\b"
        <<"\\bdescribe\\b" <<"\\bdesolve\\b" <<"\\bdeterminant\\b" <<"\\bdgauss_a\\b"
        <<"\\bdgauss_b\\b" <<"\\bdgeev\\b" <<"\\bdgesvd\\b" <<"\\bdiag\\b"
        <<"\\bdiagmatrix\\b" <<"\\bdiag_matrix\\b" <<"\\bdiagmatrixp\\b" <<"\\bdiameter\\b"
        <<"\\bdiff\\b" <<"\\bdigitcharp\\b" <<"\\bdimacs_export\\b" <<"\\bdimacs_import\\b"
        <<"\\bdimension\\b" <<"\\bdirect\\b" <<"\\bdiscrete_freq\\b" <<"\\bdisjoin\\b"
        <<"\\bdisjointp\\b" <<"\\bdisolate\\b" <<"\\bdisp\\b" <<"\\bdispcon\\b"
        <<"\\bdispform\\b" <<"\\bdispfun\\b" <<"\\bdispJordan\\b" <<"\\bdisplay\\b"
        <<"\\bdisprule\\b" <<"\\bdispterms\\b" <<"\\bdistrib\\b" <<"\\bdivide\\b"
        <<"\\bdivisors\\b" <<"\\bdivsum\\b" <<"\\bdkummer_m\\b" <<"\\bdkummer_u\\b"
        <<"\\bdlange\\b" <<"\\bdodecahedron_graph\\b" <<"\\bdotproduct\\b" <<"\\bdotsimp\\b"
        <<"\\bdpart\\b" <<"\\bdraw\\b" <<"\\bdraw2d\\b" <<"\\bdraw3d\\b"
        <<"\\bdraw_graph\\b" <<"\\bdscalar\\b" <<"\\bechelon\\b" <<"\\bedge_coloring\\b"
        <<"\\bedges\\b" <<"\\beigens_by_jacobi\\b" <<"\\beigenvalues\\b" <<"\\beigenvectors\\b"
        <<"\\beighth\\b" <<"\\beinstein\\b" <<"\\beivals\\b" <<"\\beivects\\b"
        <<"\\belapsed_real_time\\b" <<"\\belapsed_run_time\\b" <<"\\bele2comp\\b" <<"\\bele2polynome\\b"
        <<"\\bele2pui\\b" <<"\\belem\\b" <<"\\belementp\\b" <<"\\beliminate\\b"
        <<"\\belliptic_e\\b" <<"\\belliptic_ec\\b" <<"\\belliptic_eu\\b" <<"\\belliptic_f\\b"
        <<"\\belliptic_kc\\b" <<"\\belliptic_pi\\b" <<"\\bematrix\\b" <<"\\bempty_graph\\b"
        <<"\\bemptyp\\b" <<"\\bendcons\\b" <<"\\bentermatrix\\b" <<"\\bentertensor\\b"
        <<"\\bentier\\b" <<"\\bequal\\b" <<"\\bequalp\\b" <<"\\bequiv_classes\\b"
        <<"\\berf\\b" <<"\\berrcatch\\b" <<"\\berror\\b" <<"\\berrormsg\\b"
        <<"\\beuler\\b" <<"\\bev\\b" <<"\\beval_string\\b" <<"\\bevenp\\b"
        <<"\\bevery\\b" <<"\\bevolution\\b" <<"\\bevolution2d\\b" <<"\\bevundiff\\b"
        <<"\\bexample\\b" <<"\\bexp\\b" <<"\\bexpand\\b" <<"\\bexpandwrt\\b"
        <<"\\bexpandwrt_factored\\b" <<"\\bexplose\\b" <<"\\bexponentialize\\b" <<"\\bexpress\\b"
        <<"\\bexpt\\b" <<"\\bexsec\\b" <<"\\bextdiff\\b" <<"\\bextract_linear_equations\\b"
        <<"\\bextremal_subset\\b" <<"\\bezgcd\\b" <<"\\bf90\\b" <<"\\bfacsum\\b"
        <<"\\bfactcomb\\b" <<"\\bfactor\\b" <<"\\bfactorfacsum\\b" <<"\\bfactorial\\b"
        <<"\\bfactorout\\b" <<"\\bfactorsum\\b" <<"\\bfacts\\b" <<"\\bfast_central_elements\\b"
        <<"\\bfast_linsolve\\b" <<"\\bfasttimes\\b" <<"\\bfeaturep\\b" <<"\\bfft\\b"
        <<"\\bfib\\b" <<"\\bfibtophi\\b" <<"\\bfifth\\b" <<"\\bfilename_merge\\b"
        <<"\\bfile_search\\b" <<"\\bfile_type\\b" <<"\\bfillarray\\b" <<"\\bfindde\\b"
        <<"\\bfind_root\\b" <<"\\bfirst\\b" <<"\\bfix\\b" <<"\\bflatten\\b"
        <<"\\bflength\\b" <<"\\bfloat\\b" <<"\\bfloatnump\\b" <<"\\bfloor\\b"
        <<"\\bflower_snark\\b" <<"\\bflush\\b" <<"\\bflush1deriv\\b" <<"\\bflushd\\b"
        <<"\\bflushnd\\b" <<"\\bforget\\b" <<"\\bfortran\\b" <<"\\bfourcos\\b"
        <<"\\bfourexpand\\b" <<"\\bfourier\\b" <<"\\bfourint\\b" <<"\\bfourintcos\\b"
        <<"\\bfourintsin\\b" <<"\\bfoursimp\\b" <<"\\bfoursin\\b" <<"\\bfourth\\b"
        <<"\\bfposition\\b" <<"\\bframe_bracket\\b" <<"\\bfreeof\\b" <<"\\bfreshline\\b"
        <<"\\bfrom_adjacency_matrix\\b" <<"\\bfrucht_graph\\b" <<"\\bfull_listify\\b" <<"\\bfullmap\\b"
        <<"\\bfullmapl\\b" <<"\\bfullratsimp\\b" <<"\\bfullratsubst\\b" <<"\\bfullsetify\\b"
        <<"\\bfuncsolve\\b" <<"\\bfundef\\b" <<"\\bfunmake\\b" <<"\\bfunp\\b"
        <<"\\bgamma\\b" <<"\\bgauss_a\\b" <<"\\bgauss_b\\b" <<"\\bgaussprob\\b"
        <<"\\bgcd\\b" <<"\\bgcdex\\b" <<"\\bgcdivide\\b" <<"\\bgcfac\\b"
        <<"\\bgcfactor\\b" <<"\\bgd\\b" <<"\\bgenfact\\b" <<"\\bgen_laguerre\\b"
        <<"\\bgenmatrix\\b" <<"\\bgeometric\\b" <<"\\bgeometric_mean\\b" <<"\\bgeosum\\b"
        <<"\\bget\\b" <<"\\bget_edge_weight\\b" <<"\\bget_lu_factors\\b" <<"\\bget_pixel\\b"
        <<"\\bget_vertex_label\\b" <<"\\bgfactor\\b" <<"\\bgfactorsum\\b" <<"\\bggf\\b"
        <<"\\bgirth\\b" <<"\\bglobal_variances\\b" <<"\\bgnuplot_close\\b" <<"\\bgnuplot_replot\\b"
        <<"\\bgnuplot_reset\\b" <<"\\bgnuplot_restart\\b" <<"\\bgnuplot_start\\b" <<"\\bgo\\b"
        <<"\\bGosper\\b" <<"\\bGosperSum\\b" <<"\\bgradef\\b" <<"\\bgramschmidt\\b"
        <<"\\bgraph6_decode\\b" <<"\\bgraph6_encode\\b" <<"\\bgraph6_export\\b" <<"\\bgraph6_import\\b"
        <<"\\bgraph_center\\b" <<"\\bgraph_charpoly\\b" <<"\\bgraph_eigenvalues\\b" <<"\\bgraph_order\\b"
        <<"\\bgraph_periphery\\b" <<"\\bgraph_product\\b" <<"\\bgraph_size\\b" <<"\\bgraph_union\\b"
        <<"\\bgrid_graph\\b" <<"\\bgrind\\b" <<"\\bgrobner_basis\\b" <<"\\bgrotzch_graph\\b"
        <<"\\bhamilton_cycle\\b" <<"\\bhamilton_path\\b" <<"\\bhankel\\b" <<"\\bharmonic\\b"
        <<"\\bharmonic_mean\\b" <<"\\bhav\\b" <<"\\bheawood_graph\\b" <<"\\bhermite\\b"
        <<"\\bhessian\\b" <<"\\bhilbert_matrix\\b" <<"\\bhipow\\b" <<"\\bhistogram\\b"
        <<"\\bhodge\\b" <<"\\bhorner\\b" <<"\\bic1\\b" <<"\\bic2\\b"
        <<"\\bic_convert\\b" <<"\\bichr1\\b" <<"\\bichr2\\b" <<"\\bicosahedron_graph\\b"
        <<"\\bicurvature\\b" <<"\\bident\\b" <<"\\bidentfor\\b" <<"\\bidentity\\b"
        <<"\\bidiff\\b" <<"\\bidim\\b" <<"\\bidummy\\b" <<"\\bieqn\\b"
        <<"\\bifactors\\b" <<"\\biframes\\b" <<"\\bifs\\b" <<"\\bift\\b"
        <<"\\bigeodesic_coords\\b" <<"\\bilt\\b" <<"\\bimagpart\\b" <<"\\bimetric\\b"
        <<"\\bimplicit_derivative\\b" <<"\\bimplicit_plot\\b" <<"\\bindexed_tensor\\b" <<"\\bindices\\b"
        <<"\\binduced_subgraph\\b" <<"\\binferencep\\b" <<"\\binference_result\\b" <<"\\binfix\\b"
        <<"\\binit_atensor\\b" <<"\\binit_ctensor\\b" <<"\\bin_neighbors\\b" <<"\\binnerproduct\\b"
        <<"\\binpart\\b" <<"\\binprod\\b" <<"\\binrt\\b" <<"\\bintegerp\\b"
        <<"\\binteger_partitions\\b" <<"\\bintegrate\\b" <<"\\bintersect\\b" <<"\\bintersection\\b"
        <<"\\bintervalp\\b" <<"\\bintopois\\b" <<"\\bintosum\\b" <<"\\binvariant1\\b"
        <<"\\binvariant2\\b" <<"\\binverse_jacobi_cd\\b" <<"\\binverse_jacobi_cn\\b" <<"\\binverse_jacobi_cs\\b"
        <<"\\binverse_jacobi_dc\\b" <<"\\binverse_jacobi_dn\\b" <<"\\binverse_jacobi_ds\\b" <<"\\binverse_jacobi_nc\\b"
        <<"\\binverse_jacobi_nd\\b" <<"\\binverse_jacobi_ns\\b" <<"\\binverse_jacobi_sc\\b" <<"\\binverse_jacobi_sd\\b"
        <<"\\binverse_jacobi_sn\\b" <<"\\binvert\\b" <<"\\binvert_by_lu\\b" <<"\\binv_mod\\b"
        <<"\\bis\\b" <<"\\bis_biconnected\\b" <<"\\bis_bipartite\\b" <<"\\bis_connected\\b"
        <<"\\bis_digraph\\b" <<"\\bis_edge_in_graph\\b" <<"\\bis_graph\\b" <<"\\bis_graph_or_digraph\\b"
        <<"\\bishow\\b" <<"\\bis_isomorphic\\b" <<"\\bisolate\\b" <<"\\bisomorphism\\b"
        <<"\\bis_planar\\b" <<"\\bisqrt\\b" <<"\\bis_sconnected\\b" <<"\\bis_tree\\b"
        <<"\\bis_vertex_in_graph\\b" <<"\\bitems_inference\\b" <<"\\bjacobi\\b" <<"\\bjacobian\\b"
        <<"\\bjacobi_cd\\b" <<"\\bjacobi_cn\\b" <<"\\bjacobi_cs\\b" <<"\\bjacobi_dc\\b"
        <<"\\bjacobi_dn\\b" <<"\\bjacobi_ds\\b" <<"\\bjacobi_nc\\b" <<"\\bjacobi_nd\\b"
        <<"\\bjacobi_ns\\b" <<"\\bjacobi_p\\b" <<"\\bjacobi_sc\\b" <<"\\bjacobi_sd\\b"
        <<"\\bjacobi_sn\\b" <<"\\bJF\\b" <<"\\bjoin\\b" <<"\\bjordan\\b"
        <<"\\bjulia\\b" <<"\\bkdels\\b" <<"\\bkdelta\\b" <<"\\bkill\\b"
        <<"\\bkillcontext\\b" <<"\\bkostka\\b" <<"\\bkron_delta\\b" <<"\\bkronecker_product\\b"
        <<"\\bkummer_m\\b" <<"\\bkummer_u\\b" <<"\\bkurtosis\\b" <<"\\bkurtosis_bernoulli\\b"
        <<"\\bkurtosis_beta\\b" <<"\\bkurtosis_binomial\\b" <<"\\bkurtosis_chi2\\b" <<"\\bkurtosis_continuous_uniform\\b"
        <<"\\bkurtosis_discrete_uniform\\b" <<"\\bkurtosis_exp\\b" <<"\\bkurtosis_f\\b" <<"\\bkurtosis_gamma\\b"
        <<"\\bkurtosis_geometric\\b" <<"\\bkurtosis_gumbel\\b" <<"\\bkurtosis_hypergeometric\\b" <<"\\bkurtosis_laplace\\b"
        <<"\\bkurtosis_logistic\\b" <<"\\bkurtosis_lognormal\\b" <<"\\bkurtosis_negative_binomial\\b" <<"\\bkurtosis_normal\\b"
        <<"\\bkurtosis_pareto\\b" <<"\\bkurtosis_poisson\\b" <<"\\bkurtosis_rayleigh\\b" <<"\\bkurtosis_student_t\\b"
        <<"\\bkurtosis_weibull\\b" <<"\\blabels\\b" <<"\\blagrange\\b" <<"\\blaguerre\\b"
        <<"\\blambda\\b" <<"\\blaplace\\b" <<"\\blaplacian_matrix\\b" <<"\\blast\\b"
        <<"\\blbfgs\\b" <<"\\blc2kdt\\b" <<"\\blcharp\\b" <<"\\blc_l\\b"
        <<"\\blcm\\b" <<"\\blc_u\\b" <<"\\bldefint\\b" <<"\\bldisp\\b"
        <<"\\bldisplay\\b" <<"\\blegendre_p\\b" <<"\\blegendre_q\\b" <<"\\bleinstein\\b"
        <<"\\blength\\b" <<"\\blet\\b" <<"\\bletrules\\b" <<"\\bletsimp\\b"
        <<"\\blevi_civita\\b" <<"\\blfreeof\\b" <<"\\blgtreillis\\b" <<"\\blhs\\b"
        <<"\\bli\\b" <<"\\bliediff\\b" <<"\\blimit\\b" <<"\\bLindstedt\\b"
        <<"\\blinear\\b" <<"\\blinearinterpol\\b" <<"\\blinear_program\\b" <<"\\bline_graph\\b"
        <<"\\blinsolve\\b" <<"\\blistarray\\b" <<"\\blist_correlations\\b" <<"\\blistify\\b"
        <<"\\blist_nc_monomials\\b" <<"\\blistoftens\\b" <<"\\blistofvars\\b" <<"\\blistp\\b"
        <<"\\blmax\\b" <<"\\blmin\\b" <<"\\bload\\b" <<"\\bloadfile\\b"
        <<"\\blocal\\b" <<"\\blocate_matrix_entry\\b" <<"\\blog\\b" <<"\\blogand\\b"
        <<"\\blogarc\\b" <<"\\blogcontract\\b" <<"\\blogor\\b" <<"\\blogxor\\b"
        <<"\\blopow\\b" <<"\\blorentz_gauge\\b" <<"\\blowercasep\\b" <<"\\blpart\\b"
        <<"\\blratsubst\\b" <<"\\blreduce\\b" <<"\\blriemann\\b" <<"\\blsquares_estimates\\b"
        <<"\\blsquares_estimates_approximate\\b" <<"\\blsquares_estimates_exact\\b" <<"\\blsquares_mse\\b" <<"\\blsquares_residual_mse\\b"
        <<"\\blsquares_residuals\\b" <<"\\blsum\\b" <<"\\bltreillis\\b" <<"\\blu_backsub\\b"
        <<"\\blu_factor\\b" <<"\\bmacroexpand\\b" <<"\\bmacroexpand1\\b" <<"\\bmake_array\\b"
        <<"\\bmakebox\\b" <<"\\bmakefact\\b" <<"\\bmakegamma\\b" <<"\\bmake_level_picture\\b"
        <<"\\bmakelist\\b" <<"\\bmakeOrders\\b" <<"\\bmake_poly_continent\\b" <<"\\bmake_poly_country\\b"
        <<"\\bmake_polygon\\b" <<"\\bmake_random_state\\b" <<"\\bmake_rgb_picture\\b" <<"\\bmakeset\\b"
        <<"\\bmake_transform\\b" <<"\\bmandelbrot\\b" <<"\\bmap\\b" <<"\\bmapatom\\b"
        <<"\\bmaplist\\b" <<"\\bmatchdeclare\\b" <<"\\bmatchfix\\b" <<"\\bmat_cond\\b"
        <<"\\bmat_fullunblocker\\b" <<"\\bmat_function\\b" <<"\\bmat_norm\\b" <<"\\bmatrix\\b"
        <<"\\bmatrixmap\\b" <<"\\bmatrixp\\b" <<"\\bmatrix_size\\b" <<"\\bmattrace\\b"
        <<"\\bmat_trace\\b" <<"\\bmat_unblocker\\b" <<"\\bmax\\b" <<"\\bmax_clique\\b"
        <<"\\bmax_degree\\b" <<"\\bmax_flow\\b" <<"\\bmaxi\\b" <<"\\bmaximize_lp\\b"
        <<"\\bmax_independent_set\\b" <<"\\bmax_matching\\b" <<"\\bmaybe\\b" <<"\\bmean\\b"
        <<"\\bmean_bernoulli\\b" <<"\\bmean_beta\\b" <<"\\bmean_binomial\\b" <<"\\bmean_chi2\\b"
        <<"\\bmean_continuous_uniform\\b" <<"\\bmean_deviation\\b" <<"\\bmean_discrete_uniform\\b" <<"\\bmean_exp\\b"
        <<"\\bmean_f\\b" <<"\\bmean_gamma\\b" <<"\\bmean_geometric\\b" <<"\\bmean_gumbel\\b"
        <<"\\bmean_hypergeometric\\b" <<"\\bmean_laplace\\b" <<"\\bmean_logistic\\b" <<"\\bmean_lognormal\\b"
        <<"\\bmean_negative_binomial\\b" <<"\\bmean_normal\\b" <<"\\bmean_pareto\\b" <<"\\bmean_poisson\\b"
        <<"\\bmean_rayleigh\\b" <<"\\bmean_student_t\\b" <<"\\bmean_weibull\\b" <<"\\bmedian\\b"
        <<"\\bmedian_deviation\\b" <<"\\bmember\\b" <<"\\bmetricexpandall\\b" <<"\\bmin\\b"
        <<"\\bmin_degree\\b" <<"\\bminfactorial\\b" <<"\\bmini\\b" <<"\\bminimalPoly\\b"
        <<"\\bminimize_lp\\b" <<"\\bminimum_spanning_tree\\b" <<"\\bminor\\b" <<"\\bmnewton\\b"
        <<"\\bmod\\b" <<"\\bmode_declare\\b" <<"\\bmode_identity\\b" <<"\\bModeMatrix\\b"
        <<"\\bmoebius\\b" <<"\\bmon2schur\\b" <<"\\bmono\\b" <<"\\bmonomial_dimensions\\b"
        <<"\\bmulti_elem\\b" <<"\\bmultinomial\\b" <<"\\bmultinomial_coeff\\b" <<"\\bmulti_orbit\\b"
        <<"\\bmulti_pui\\b" <<"\\bmultsym\\b" <<"\\bmultthru\\b" <<"\\bmycielski_graph\\b"
        <<"\\bnary\\b" <<"\\bnc_degree\\b" <<"\\bncexpt\\b" <<"\\bncharpoly\\b"
        <<"\\bnegative_picture\\b" <<"\\bneighbors\\b" <<"\\bnewcontext\\b" <<"\\bnewdet\\b"
        <<"\\bnew_graph\\b" <<"\\bnewline\\b" <<"\\bnewton\\b" <<"\\bnext_prime\\b"
        <<"\\bniceindices\\b" <<"\\bninth\\b" <<"\\bnoncentral_moment\\b" <<"\\bnonmetricity\\b"
        <<"\\bnonnegintegerp\\b" <<"\\bnonscalarp\\b" <<"\\bnonzeroandfreeof\\b" <<"\\bnotequal\\b"
        <<"\\bnounify\\b" <<"\\bnptetrad\\b" <<"\\bnroots\\b" <<"\\bnterms\\b"
        <<"\\bntermst\\b" <<"\\bnthroot\\b" <<"\\bnullity\\b" <<"\\bnullspace\\b"
        <<"\\bnum\\b" <<"\\bnumbered_boundaries\\b" <<"\\bnumberp\\b" <<"\\bnum_distinct_partitions\\b"
        <<"\\bnumerval\\b" <<"\\bnumfactor\\b" <<"\\bnum_partitions\\b" <<"\\bnusum\\b"
        <<"\\bodd_girth\\b" <<"\\boddp\\b" <<"\\bode2\\b" <<"\\bode_check\\b"
        <<"\\bodelin\\b" <<"\\bop\\b" <<"\\bopena\\b" <<"\\bopenr\\b"
        <<"\\bopenw\\b" <<"\\boperatorp\\b" <<"\\bopsubst\\b" <<"\\boptimize\\b"
        <<"\\borbit\\b" <<"\\borbits\\b" <<"\\bordergreat\\b" <<"\\bordergreatp\\b"
        <<"\\borderless\\b" <<"\\borderlessp\\b" <<"\\borthogonal_complement\\b" <<"\\borthopoly_recur\\b"
        <<"\\borthopoly_weight\\b" <<"\\boutermap\\b" <<"\\bout_neighbors\\b" <<"\\boutofpois\\b"
        <<"\\bpade\\b" <<"\\bparGosper\\b" <<"\\bparse_string\\b" <<"\\bpart\\b"
        <<"\\bpart2cont\\b" <<"\\bpartfrac\\b" <<"\\bpartition\\b" <<"\\bpartition_set\\b"
        <<"\\bpartpol\\b" <<"\\bpath_digraph\\b" <<"\\bpath_graph\\b" <<"\\bpdf_bernoulli\\b"
        <<"\\bpdf_beta\\b" <<"\\bpdf_binomial\\b" <<"\\bpdf_cauchy\\b" <<"\\bpdf_chi2\\b"
        <<"\\bpdf_continuous_uniform\\b" <<"\\bpdf_discrete_uniform\\b" <<"\\bpdf_exp\\b" <<"\\bpdf_f\\b"
        <<"\\bpdf_gamma\\b" <<"\\bpdf_geometric\\b" <<"\\bpdf_gumbel\\b" <<"\\bpdf_hypergeometric\\b"
        <<"\\bpdf_laplace\\b" <<"\\bpdf_logistic\\b" <<"\\bpdf_lognormal\\b" <<"\\bpdf_negative_binomial\\b"
        <<"\\bpdf_normal\\b" <<"\\bpdf_pareto\\b" <<"\\bpdf_poisson\\b" <<"\\bpdf_rank_sum\\b"
        <<"\\bpdf_rayleigh\\b" <<"\\bpdf_signed_rank\\b" <<"\\bpdf_student_t\\b" <<"\\bpdf_weibull\\b"
        <<"\\bpearson_skewness\\b" <<"\\bpermanent\\b" <<"\\bpermut\\b" <<"\\bpermutation\\b"
        <<"\\bpermutations\\b" <<"\\bpetersen_graph\\b" <<"\\bpetrov\\b" <<"\\bpickapart\\b"
        <<"\\bpicture_equalp\\b" <<"\\bpicturep\\b" <<"\\bpiechart\\b" <<"\\bplanar_embedding\\b"
        <<"\\bplayback\\b" <<"\\bplog\\b" <<"\\bplot2d\\b" <<"\\bplot3d\\b"
        <<"\\bplotdf\\b" <<"\\bplsquares\\b" <<"\\bpochhammer\\b" <<"\\bpoisdiff\\b"
        <<"\\bpoisexpt\\b" <<"\\bpoisint\\b" <<"\\bpoismap\\b" <<"\\bpoisplus\\b"
        <<"\\bpoissimp\\b" <<"\\bpoissubst\\b" <<"\\bpoistimes\\b" <<"\\bpoistrim\\b"
        <<"\\bpolarform\\b" <<"\\bpolartorect\\b" <<"\\bpoly_add\\b" <<"\\bpoly_buchberger\\b"
        <<"\\bpoly_buchberger_criterion\\b" <<"\\bpoly_colon_ideal\\b" <<"\\bpoly_content\\b" <<"\\bpolydecomp\\b"
        <<"\\bpoly_depends_p\\b" <<"\\bpoly_elimination_ideal\\b" <<"\\bpoly_exact_divide\\b" <<"\\bpoly_expand\\b"
        <<"\\bpoly_expt\\b" <<"\\bpoly_gcd\\b" <<"\\bpoly_grobner\\b" <<"\\bpoly_grobner_equal\\b"
        <<"\\bpoly_grobner_member\\b" <<"\\bpoly_grobner_subsetp\\b" <<"\\bpoly_ideal_intersection\\b" <<"\\bpoly_ideal_polysaturation\\b"
        <<"\\bpoly_ideal_polysaturation1\\b" <<"\\bpoly_ideal_saturation\\b" <<"\\bpoly_ideal_saturation1\\b" <<"\\bpoly_lcm\\b"
        <<"\\bpoly_minimization\\b" <<"\\bpolymod\\b" <<"\\bpoly_multiply\\b" <<"\\bpolynome2ele\\b"
        <<"\\bpolynomialp\\b" <<"\\bpoly_normal_form\\b" <<"\\bpoly_normalize\\b" <<"\\bpoly_normalize_list\\b"
        <<"\\bpoly_polysaturation_extension\\b" <<"\\bpoly_primitive_part\\b" <<"\\bpoly_pseudo_divide\\b" <<"\\bpoly_reduced_grobner\\b"
        <<"\\bpoly_reduction\\b" <<"\\bpoly_saturation_extension\\b" <<"\\bpoly_s_polynomial\\b" <<"\\bpoly_subtract\\b"
        <<"\\bpolytocompanion\\b" <<"\\bpotential\\b" <<"\\bpower_mod\\b" <<"\\bpowers\\b"
        <<"\\bpowerseries\\b" <<"\\bpowerset\\b" <<"\\bprev_prime\\b" <<"\\bprimep\\b"
        <<"\\bprint\\b" <<"\\bprintf\\b" <<"\\bprint_graph\\b" <<"\\bprintpois\\b"
        <<"\\bprintprops\\b" <<"\\bprodrac\\b" <<"\\bproduct\\b" <<"\\bproperties\\b"
        <<"\\bpropvars\\b" <<"\\bpsi\\b" <<"\\bptriangularize\\b" <<"\\bpui\\b"
        <<"\\bpui2comp\\b" <<"\\bpui2ele\\b" <<"\\bpui2polynome\\b" <<"\\bpui_direct\\b"
        <<"\\bpuireduc\\b" <<"\\bput\\b" <<"\\bqput\\b" <<"\\bqrange\\b"
        <<"\\bquad_qag\\b" <<"\\bquad_qagi\\b" <<"\\bquad_qags\\b" <<"\\bquad_qawc\\b"
        <<"\\bquad_qawf\\b" <<"\\bquad_qawo\\b" <<"\\bquad_qaws\\b" <<"\\bquantile\\b"
        <<"\\bquantile_bernoulli\\b" <<"\\bquantile_beta\\b" <<"\\bquantile_binomial\\b" <<"\\bquantile_cauchy\\b"
        <<"\\bquantile_chi2\\b" <<"\\bquantile_continuous_uniform\\b" <<"\\bquantile_discrete_uniform\\b" <<"\\bquantile_exp\\b"
        <<"\\bquantile_f\\b" <<"\\bquantile_gamma\\b" <<"\\bquantile_geometric\\b" <<"\\bquantile_gumbel\\b"
        <<"\\bquantile_hypergeometric\\b" <<"\\bquantile_laplace\\b" <<"\\bquantile_logistic\\b" <<"\\bquantile_lognormal\\b"
        <<"\\bquantile_negative_binomial\\b" <<"\\bquantile_normal\\b" <<"\\bquantile_pareto\\b" <<"\\bquantile_poisson\\b"
        <<"\\bquantile_rayleigh\\b" <<"\\bquantile_student_t\\b" <<"\\bquantile_weibull\\b" <<"\\bquartile_skewness\\b"
        <<"\\bquit\\b" <<"\\bqunit\\b" <<"\\bquotient\\b" <<"\\bradcan\\b"
        <<"\\bradius\\b" <<"\\brandom\\b" <<"\\brandom_bernoulli\\b" <<"\\brandom_beta\\b"
        <<"\\brandom_binomial\\b" <<"\\brandom_cauchy\\b" <<"\\brandom_chi2\\b" <<"\\brandom_continuous_uniform\\b"
        <<"\\brandom_digraph\\b" <<"\\brandom_discrete_uniform\\b" <<"\\brandom_exp\\b" <<"\\brandom_f\\b"
        <<"\\brandom_gamma\\b" <<"\\brandom_geometric\\b" <<"\\brandom_graph\\b" <<"\\brandom_graph1\\b"
        <<"\\brandom_gumbel\\b" <<"\\brandom_hypergeometric\\b" <<"\\brandom_laplace\\b" <<"\\brandom_logistic\\b"
        <<"\\brandom_lognormal\\b" <<"\\brandom_negative_binomial\\b" <<"\\brandom_network\\b" <<"\\brandom_normal\\b"
        <<"\\brandom_pareto\\b" <<"\\brandom_permutation\\b" <<"\\brandom_poisson\\b" <<"\\brandom_rayleigh\\b"
        <<"\\brandom_regular_graph\\b" <<"\\brandom_student_t\\b" <<"\\brandom_tournament\\b" <<"\\brandom_tree\\b"
        <<"\\brandom_weibull\\b" <<"\\brange\\b" <<"\\brank\\b" <<"\\brat\\b"
        <<"\\bratcoef\\b" <<"\\bratdenom\\b" <<"\\bratdiff\\b" <<"\\bratdisrep\\b"
        <<"\\bratexpand\\b" <<"\\brational\\b" <<"\\brationalize\\b" <<"\\bratnumer\\b"
        <<"\\bratnump\\b" <<"\\bratp\\b" <<"\\bratsimp\\b" <<"\\bratsubst\\b"
        <<"\\bratvars\\b" <<"\\bratweight\\b" <<"\\bread\\b" <<"\\bread_hashed_array\\b"
        <<"\\breadline\\b" <<"\\bread_lisp_array\\b" <<"\\bread_list\\b" <<"\\bread_matrix\\b"
        <<"\\bread_maxima_array\\b" <<"\\bread_nested_list\\b" <<"\\breadonly\\b" <<"\\bread_xpm\\b"
        <<"\\brealpart\\b" <<"\\brealroots\\b" <<"\\brearray\\b" <<"\\brectform\\b"
        <<"\\brecttopolar\\b" <<"\\brediff\\b" <<"\\breduce_consts\\b" <<"\\breduce_order\\b"
        <<"\\bregion_boundaries\\b" <<"\\brem\\b" <<"\\bremainder\\b" <<"\\bremarray\\b"
        <<"\\brembox\\b" <<"\\bremcomps\\b" <<"\\bremcon\\b" <<"\\bremcoord\\b"
        <<"\\bremfun\\b" <<"\\bremfunction\\b" <<"\\bremlet\\b" <<"\\bremove\\b"
        <<"\\bremove_edge\\b" <<"\\bremove_vertex\\b" <<"\\brempart\\b" <<"\\bremrule\\b"
        <<"\\bremsym\\b" <<"\\bremvalue\\b" <<"\\brename\\b" <<"\\breset\\b"
        <<"\\bresidue\\b" <<"\\bresolvante\\b" <<"\\bresolvante_alternee1\\b" <<"\\bresolvante_bipartite\\b"
        <<"\\bresolvante_diedrale\\b" <<"\\bresolvante_klein\\b" <<"\\bresolvante_klein3\\b" <<"\\bresolvante_produit_sym\\b"
        <<"\\bresolvante_unitaire\\b" <<"\\bresolvante_vierer\\b" <<"\\brest\\b" <<"\\bresultant\\b"
        <<"\\breturn\\b" <<"\\breveal\\b" <<"\\breverse\\b" <<"\\brevert\\b"
        <<"\\brevert2\\b" <<"\\brgb2level\\b" <<"\\brhs\\b" <<"\\bricci\\b"
        <<"\\briemann\\b" <<"\\brinvariant\\b" <<"\\brisch\\b" <<"\\brk\\b"
        <<"\\brncombine\\b" <<"\\bromberg\\b" <<"\\broom\\b" <<"\\brootscontract\\b"
        <<"\\brow\\b" <<"\\browop\\b" <<"\\browswap\\b" <<"\\brreduce\\b"
        <<"\\brun_testsuite\\b" <<"\\bsave\\b" <<"\\bscalarp\\b" <<"\\bscaled_bessel_i\\b"
        <<"\\bscaled_bessel_i0\\b" <<"\\bscaled_bessel_i1\\b" <<"\\bscalefactors\\b" <<"\\bscanmap\\b"
        <<"\\bscatterplot\\b" <<"\\bschur2comp\\b" <<"\\bsconcat\\b" <<"\\bscopy\\b"
        <<"\\bscsimp\\b" <<"\\bscurvature\\b" <<"\\bsdowncase\\b" <<"\\bsec\\b"
        <<"\\bsech\\b" <<"\\bsecond\\b" <<"\\bsequal\\b" <<"\\bsequalignore\\b"
        <<"\\bsetdifference\\b" <<"\\bset_edge_weight\\b" <<"\\bsetelmx\\b" <<"\\bsetequalp\\b"
        <<"\\bsetify\\b" <<"\\bsetp\\b" <<"\\bset_partitions\\b" <<"\\bset_plot_option\\b"
        <<"\\bset_random_state\\b" <<"\\bsetunits\\b" <<"\\bsetup_autoload\\b" <<"\\bset_up_dot_simplifications\\b"
        <<"\\bset_vertex_label\\b" <<"\\bseventh\\b" <<"\\bsexplode\\b" <<"\\bsf\\b"
        <<"\\bshortest_path\\b" <<"\\bshow\\b" <<"\\bshowcomps\\b" <<"\\bshowratvars\\b"
        <<"\\bsign\\b" <<"\\bsignum\\b" <<"\\bsimilaritytransform\\b" <<"\\bsimple_linear_regression\\b"
        <<"\\bsimplify_sum\\b" <<"\\bsimplode\\b" <<"\\bsimpmetderiv\\b" <<"\\bsimtran\\b"
        <<"\\bsin\\b" <<"\\bsinh\\b" <<"\\bsinsert\\b" <<"\\bsinvertcase\\b"
        <<"\\bsixth\\b" <<"\\bskewness\\b" <<"\\bskewness_bernoulli\\b" <<"\\bskewness_beta\\b"
        <<"\\bskewness_binomial\\b" <<"\\bskewness_chi2\\b" <<"\\bskewness_continuous_uniform\\b" <<"\\bskewness_discrete_uniform\\b"
        <<"\\bskewness_exp\\b" <<"\\bskewness_f\\b" <<"\\bskewness_gamma\\b" <<"\\bskewness_geometric\\b"
        <<"\\bskewness_gumbel\\b" <<"\\bskewness_hypergeometric\\b" <<"\\bskewness_laplace\\b" <<"\\bskewness_logistic\\b"
        <<"\\bskewness_lognormal\\b" <<"\\bskewness_negative_binomial\\b" <<"\\bskewness_normal\\b" <<"\\bskewness_pareto\\b"
        <<"\\bskewness_poisson\\b" <<"\\bskewness_rayleigh\\b" <<"\\bskewness_student_t\\b" <<"\\bskewness_weibull\\b"
        <<"\\bslength\\b" <<"\\bsmake\\b" <<"\\bsmismatch\\b" <<"\\bsolve\\b"
        <<"\\bsolve_rec\\b" <<"\\bsolve_rec_rat\\b" <<"\\bsome\\b" <<"\\bsomrac\\b"
        <<"\\bsort\\b" <<"\\bsparse6_decode\\b" <<"\\bsparse6_encode\\b" <<"\\bsparse6_export\\b"
        <<"\\bsparse6_import\\b" <<"\\bspecint\\b" <<"\\bspherical_bessel_j\\b" <<"\\bspherical_bessel_y\\b"
        <<"\\bspherical_hankel1\\b" <<"\\bspherical_hankel2\\b" <<"\\bspherical_harmonic\\b" <<"\\bsplice\\b"
        <<"\\bsplit\\b" <<"\\bsposition\\b" <<"\\bsprint\\b" <<"\\bsqfr\\b"
        <<"\\bsqrt\\b" <<"\\bsqrtdenest\\b" <<"\\bsremove\\b" <<"\\bsremovefirst\\b"
        <<"\\bsreverse\\b" <<"\\bssearch\\b" <<"\\bssort\\b" <<"\\bsstatus\\b"
        <<"\\bssubst\\b" <<"\\bssubstfirst\\b" <<"\\bstaircase\\b" <<"\\bstatus\\b"
        <<"\\bstd\\b" <<"\\bstd1\\b" <<"\\bstd_bernoulli\\b" <<"\\bstd_beta\\b"
        <<"\\bstd_binomial\\b" <<"\\bstd_chi2\\b" <<"\\bstd_continuous_uniform\\b" <<"\\bstd_discrete_uniform\\b"
        <<"\\bstd_exp\\b" <<"\\bstd_f\\b" <<"\\bstd_gamma\\b" <<"\\bstd_geometric\\b"
        <<"\\bstd_gumbel\\b" <<"\\bstd_hypergeometric\\b" <<"\\bstd_laplace\\b" <<"\\bstd_logistic\\b"
        <<"\\bstd_lognormal\\b" <<"\\bstd_negative_binomial\\b" <<"\\bstd_normal\\b" <<"\\bstd_pareto\\b"
        <<"\\bstd_poisson\\b" <<"\\bstd_rayleigh\\b" <<"\\bstd_student_t\\b" <<"\\bstd_weibull\\b"
        <<"\\bstirling\\b" <<"\\bstirling1\\b" <<"\\bstirling2\\b" <<"\\bstrim\\b"
        <<"\\bstriml\\b" <<"\\bstrimr\\b" <<"\\bstring\\b" <<"\\bstringout\\b"
        <<"\\bstringp\\b" <<"\\bstrong_components\\b" <<"\\bsublis\\b" <<"\\bsublist\\b"
        <<"\\bsublist_indices\\b" <<"\\bsubmatrix\\b" <<"\\bsubsample\\b" <<"\\bsubset\\b"
        <<"\\bsubsetp\\b" <<"\\bsubst\\b" <<"\\bsubstinpart\\b" <<"\\bsubstpart\\b"
        <<"\\bsubstring\\b" <<"\\bsubvar\\b" <<"\\bsubvarp\\b" <<"\\bsum\\b"
        <<"\\bsumcontract\\b" <<"\\bsummand_to_rec\\b" <<"\\bsupcase\\b" <<"\\bsupcontext\\b"
        <<"\\bsymbolp\\b" <<"\\bsymmdifference\\b" <<"\\bsymmetricp\\b" <<"\\bsystem\\b"
        <<"\\btake_channel\\b" <<"\\btake_inference\\b" <<"\\btan\\b" <<"\\btanh\\b"
        <<"\\btaylor\\b" <<"\\btaylorinfo\\b" <<"\\btaylorp\\b" <<"\\btaylor_simplifier\\b"
        <<"\\btaytorat\\b" <<"\\btcl_output\\b" <<"\\btcontract\\b" <<"\\btellrat\\b"
        <<"\\btellsimp\\b" <<"\\btellsimpafter\\b" <<"\\btentex\\b" <<"\\btenth\\b"
        <<"\\btest_mean\\b" <<"\\btest_means_difference\\b" <<"\\btest_normality\\b" <<"\\btest_rank_sum\\b"
        <<"\\btest_sign\\b" <<"\\btest_signed_rank\\b" <<"\\btest_variance\\b" <<"\\btest_variance_ratio\\b"
        <<"\\btex\\b" <<"\\btexput\\b" <<"\\b%th\\b" <<"\\bthird\\b"
        <<"\\bthrow\\b" <<"\\btime\\b" <<"\\btimedate\\b" <<"\\btimer\\b"
        <<"\\btimer_info\\b" <<"\\btldefint\\b" <<"\\btlimit\\b" <<"\\btodd_coxeter\\b"
        <<"\\btoeplitz\\b" <<"\\btokens\\b" <<"\\bto_lisp\\b" <<"\\btopological_sort\\b"
        <<"\\btotaldisrep\\b" <<"\\btotalfourier\\b" <<"\\btotient\\b" <<"\\btpartpol\\b"
        <<"\\btrace\\b" <<"\\btracematrix\\b" <<"\\btrace_options\\b" <<"\\btranslate\\b"
        <<"\\btranslate_file\\b" <<"\\btranspose\\b" <<"\\btree_reduce\\b" <<"\\btreillis\\b"
        <<"\\btreinat\\b" <<"\\btriangularize\\b" <<"\\btrigexpand\\b" <<"\\btrigrat\\b"
        <<"\\btrigreduce\\b" <<"\\btrigsimp\\b" <<"\\btrunc\\b" <<"\\btr_warnings_get\\b"
        <<"\\bueivects\\b" <<"\\buforget\\b" <<"\\bultraspherical\\b" <<"\\bunderlying_graph\\b"
        <<"\\bundiff\\b" <<"\\bunion\\b" <<"\\bunique\\b" <<"\\buniteigenvectors\\b"
        <<"\\bunit_step\\b" <<"\\bunitvector\\b" <<"\\bunknown\\b" <<"\\bunorder\\b"
        <<"\\bunsum\\b" <<"\\buntellrat\\b" <<"\\buntimer\\b" <<"\\buntrace\\b"
        <<"\\buppercasep\\b" <<"\\buricci\\b" <<"\\buriemann\\b" <<"\\buvect\\b"
        <<"\\bvandermonde_matrix\\b" <<"\\bvar\\b" <<"\\bvar1\\b" <<"\\bvar_bernoulli\\b"
        <<"\\bvar_beta\\b" <<"\\bvar_binomial\\b" <<"\\bvar_chi2\\b" <<"\\bvar_continuous_uniform\\b"
        <<"\\bvar_discrete_uniform\\b" <<"\\bvar_exp\\b" <<"\\bvar_f\\b" <<"\\bvar_gamma\\b"
        <<"\\bvar_geometric\\b" <<"\\bvar_gumbel\\b" <<"\\bvar_hypergeometric\\b" <<"\\bvar_laplace\\b"
        <<"\\bvar_logistic\\b" <<"\\bvar_lognormal\\b" <<"\\bvar_negative_binomial\\b" <<"\\bvar_normal\\b"
        <<"\\bvar_pareto\\b" <<"\\bvar_poisson\\b" <<"\\bvar_rayleigh\\b" <<"\\bvar_student_t\\b"
        <<"\\bvar_weibull\\b" <<"\\bvectorpotential\\b" <<"\\bvectorsimp\\b" <<"\\bverbify\\b"
        <<"\\bvers\\b" <<"\\bvertex_coloring\\b" <<"\\bvertex_degree\\b" <<"\\bvertex_distance\\b"
        <<"\\bvertex_eccentricity\\b" <<"\\bvertex_in_degree\\b" <<"\\bvertex_out_degree\\b" <<"\\bvertices\\b"
        <<"\\bvertices_to_cycle\\b" <<"\\bvertices_to_path\\b" <<"\\bweyl\\b" <<"\\bwheel_graph\\b"
        <<"\\bwith_stdout\\b" <<"\\bwrite_data\\b" <<"\\bwritefile\\b" <<"\\bwronskian\\b"
        <<"\\bxgraph_curves\\b" <<"\\bxreduce\\b" <<"\\bxthru\\b" <<"\\bZeilberger\\b"
        <<"\\bzeroequiv\\b" <<"\\bzerofor\\b" <<"\\bzeromatrix\\b" <<"\\bzeromatrixp\\b"
        <<"\\bzeta\\b" <<"\\bzlange\\b" ;

    foreach (const QString &pattern, maximaFunctionPatterns )
    {
        rule.pattern = QRegExp(pattern);
        rule.format = maximaFunctionFormat;
        m_highlightingRules.append(rule);
    }


    QStringList maximaVariablePatterns;
    maximaVariablePatterns
        <<"\\b_\\b" <<"\\b__\\b" <<"\\b%\\b" <<"\\b%%\\b"
        <<"\\babsboxchar\\b" <<"\\bactivecontexts\\b" <<"\\badditive\\b" <<"\\balgebraic\\b"
        <<"\\balgepsilon\\b" <<"\\balgexact\\b" <<"\\baliases\\b" <<"\\ball_dotsimp_denoms\\b"
        <<"\\ballbut\\b" <<"\\ballsym\\b" <<"\\barrays\\b" <<"\\baskexp\\b"
        <<"\\bassume_pos\\b" <<"\\bassume_pos_pred\\b" <<"\\bassumescalar\\b" <<"\\batomgrad\\b"
        <<"\\bbacksubst\\b" <<"\\bberlefact\\b" <<"\\bbesselexpand\\b" <<"\\bbftorat\\b"
        <<"\\bbftrunc\\b" <<"\\bboxchar\\b" <<"\\bbreakup\\b" <<"\\bcauchysum\\b"
        <<"\\bcflength\\b" <<"\\bcframe_flag\\b" <<"\\bcnonmet_flag\\b" <<"\\bcontext\\b"
        <<"\\bcontexts\\b" <<"\\bcosnpiflag\\b" <<"\\bctaypov\\b" <<"\\bctaypt\\b"
        <<"\\bctayswitch\\b" <<"\\bctayvar\\b" <<"\\bct_coords\\b" <<"\\bctorsion_flag\\b"
        <<"\\bctrgsimp\\b" <<"\\bcurrent_let_rule_package\\b" <<"\\bdebugmode\\b" <<"\\bdefault_let_rule_package\\b"
        <<"\\bdemoivre\\b" <<"\\bdependencies\\b" <<"\\bderivabbrev\\b" <<"\\bderivsubst\\b"
        <<"\\bdetout\\b" <<"\\bdiagmetric\\b" <<"\\bdim\\b" <<"\\bdispflag\\b"
        <<"\\bdisplay2d\\b" <<"\\bdisplay_format_internal\\b" <<"\\bdoallmxops\\b" <<"\\bdomain\\b"
        <<"\\bdomxexpt\\b" <<"\\bdomxmxops\\b" <<"\\bdomxnctimes\\b" <<"\\bdontfactor\\b"
        <<"\\bdoscmxops\\b" <<"\\bdoscmxplus\\b" <<"\\bdot0nscsimp\\b" <<"\\bdot0simp\\b"
        <<"\\bdot1simp\\b" <<"\\bdotassoc\\b" <<"\\bdotconstrules\\b" <<"\\bdotdistrib\\b"
        <<"\\bdotexptsimp\\b" <<"\\bdotident\\b" <<"\\bdotscrules\\b" <<"\\bdraw_graph_program\\b"
        <<"\\b%edispflag\\b" <<"\\b%emode\\b" <<"\\b%enumer\\b" <<"\\bepsilon_lp\\b"
        <<"\\berfflag\\b" <<"\\berror\\b" <<"\\berror_size\\b" <<"\\berror_syms\\b"
        <<"\\b%e_to_numlog\\b" <<"\\bevflag\\b" <<"\\bevfun\\b" <<"\\bexpandwrt_denom\\b"
        <<"\\bexpon\\b" <<"\\bexponentialize\\b" <<"\\bexpop\\b" <<"\\bexptdispflag\\b"
        <<"\\bexptisolate\\b" <<"\\bexptsubst\\b" <<"\\bfacexpand\\b" <<"\\bfactlim\\b"
        <<"\\bfactorflag\\b" <<"\\bfile_output_append\\b" <<"\\bfile_search_demo\\b" <<"\\bfile_search_lisp\\b"
        <<"\\bfile_search_maxima\\b" <<"\\bfind_root_abs\\b" <<"\\bfind_root_error\\b" <<"\\bfind_root_rel\\b"
        <<"\\bflipflag\\b" <<"\\bfloat2bf\\b" <<"\\bfortindent\\b" <<"\\bfortspaces\\b"
        <<"\\bfpprec\\b" <<"\\bfpprintprec\\b" <<"\\bfunctions\\b" <<"\\bgammalim\\b"
        <<"\\bgdet\\b" <<"\\bgenindex\\b" <<"\\bgensumnum\\b" <<"\\bGGFCFMAX\\b"
        <<"\\bGGFINFINITY\\b" <<"\\bglobalsolve\\b" <<"\\bgradefs\\b" <<"\\bgrind\\b"
        <<"\\bhalfangles\\b" <<"\\b%iargs\\b" <<"\\bibase\\b" <<"\\bicounter\\b"
        <<"\\bidummyx\\b" <<"\\bieqnprint\\b" <<"\\biframe_bracket_form\\b" <<"\\bigeowedge_flag\\b"
        <<"\\bimetric\\b" <<"\\binchar\\b" <<"\\binfeval\\b" <<"\\binflag\\b"
        <<"\\binfolists\\b" <<"\\bin_netmath\\b" <<"\\bintegrate_use_rootsof\\b" <<"\\bintegration_constant\\b"
        <<"\\bintegration_constant_counter\\b" <<"\\bintfaclim\\b" <<"\\bisolate_wrt_times\\b" <<"\\bkeepfloat\\b"
        <<"\\blabels\\b" <<"\\bletrat\\b" <<"\\blet_rule_packages\\b" <<"\\blhospitallim\\b"
        <<"\\blimsubst\\b" <<"\\blinechar\\b" <<"\\blinel\\b" <<"\\blinenum\\b"
        <<"\\blinsolve_params\\b" <<"\\blinsolvewarn\\b" <<"\\blispdisp\\b" <<"\\blistarith\\b"
        <<"\\blistconstvars\\b" <<"\\blistdummyvars\\b" <<"\\blmxchar\\b" <<"\\bloadprint\\b"
        <<"\\blogabs\\b" <<"\\blogarc\\b" <<"\\blogconcoeffp\\b" <<"\\blogexpand\\b"
        <<"\\blognegint\\b" <<"\\blognumer\\b" <<"\\blogsimp\\b" <<"\\bm1pbranch\\b"
        <<"\\bmacroexpansion\\b" <<"\\bmaperror\\b" <<"\\bmapprint\\b" <<"\\bmatrix_element_add\\b"
        <<"\\bmatrix_element_mult\\b" <<"\\bmatrix_element_transpose\\b" <<"\\bmaxapplydepth\\b" <<"\\bmaxapplyheight\\b"
        <<"\\bmaxima_tempdir\\b" <<"\\bmaxima_userdir\\b" <<"\\bmaxnegex\\b" <<"\\bmaxposex\\b"
        <<"\\bmaxpsifracdenom\\b" <<"\\bmaxpsifracnum\\b" <<"\\bmaxpsinegint\\b" <<"\\bmaxpsiposint\\b"
        <<"\\bmaxtayorder\\b" <<"\\bmethod\\b" <<"\\bmode_check_errorp\\b" <<"\\bmode_checkp\\b"
        <<"\\bmode_check_warnp\\b" <<"\\bmodulus\\b" <<"\\bmultiplicities\\b" <<"\\bmyoptions\\b"
        <<"\\bnegdistrib\\b" <<"\\bnegsumdispflag\\b" <<"\\bnewtonepsilon\\b" <<"\\bnewtonmaxiter\\b"
        <<"\\bniceindicespref\\b" <<"\\bnolabels\\b" <<"\\bnonegative_lp\\b" <<"\\bnoundisp\\b"
        <<"\\bobase\\b" <<"\\bopproperties\\b" <<"\\bopsubst\\b" <<"\\boptimprefix\\b"
        <<"\\boptionset\\b" <<"\\boutchar\\b" <<"\\bpackagefile\\b" <<"\\bpartswitch\\b"
        <<"\\bpfeformat\\b" <<"\\b%piargs\\b" <<"\\bpiece\\b" <<"\\bplot_options\\b"
        <<"\\bpoislim\\b" <<"\\bpoly_coefficient_ring\\b" <<"\\bpoly_elimination_order\\b" <<"\\bpoly_grobner_algorithm\\b"
        <<"\\bpoly_grobner_debug\\b" <<"\\bpoly_monomial_order\\b" <<"\\bpoly_primary_elimination_order\\b" <<"\\bpoly_return_term_list\\b"
        <<"\\bpoly_secondary_elimination_order\\b" <<"\\bpoly_top_reduction_only\\b" <<"\\bpowerdisp\\b" <<"\\bprederror\\b"
        <<"\\bprimep_number_of_tests\\b" <<"\\bproduct_use_gamma\\b" <<"\\bprogrammode\\b" <<"\\bprompt\\b"
        <<"\\bpsexpand\\b" <<"\\bradexpand\\b" <<"\\bradsubstflag\\b" <<"\\brandom_beta_algorithm\\b"
        <<"\\brandom_binomial_algorithm\\b" <<"\\brandom_chi2_algorithm\\b" <<"\\brandom_exp_algorithm\\b" <<"\\brandom_f_algorithm\\b"
        <<"\\brandom_gamma_algorithm\\b" <<"\\brandom_geometric_algorithm\\b" <<"\\brandom_hypergeometric_algorithm\\b" <<"\\brandom_negative_binomial_algorithm\\b"
        <<"\\brandom_normal_algorithm\\b" <<"\\brandom_poisson_algorithm\\b" <<"\\brandom_student_t_algorithm\\b" <<"\\bratalgdenom\\b"
        <<"\\bratchristof\\b" <<"\\bratdenomdivide\\b" <<"\\brateinstein\\b" <<"\\bratepsilon\\b"
        <<"\\bratexpand\\b" <<"\\bratfac\\b" <<"\\bratmx\\b" <<"\\bratprint\\b"
        <<"\\bratriemann\\b" <<"\\bratsimpexpons\\b" <<"\\bratvars\\b" <<"\\bratweights\\b"
        <<"\\bratweyl\\b" <<"\\bratwtlvl\\b" <<"\\brealonly\\b" <<"\\brefcheck\\b"
        <<"\\brmxchar\\b" <<"\\b%rnum_list\\b" <<"\\brombergabs\\b" <<"\\brombergit\\b"
        <<"\\brombergmin\\b" <<"\\brombergtol\\b" <<"\\brootsconmode\\b" <<"\\brootsepsilon\\b"
        <<"\\bsavedef\\b" <<"\\bsavefactors\\b" <<"\\bscalarmatrixp\\b" <<"\\bsetcheck\\b"
        <<"\\bsetcheckbreak\\b" <<"\\bsetval\\b" <<"\\bshowtime\\b" <<"\\bsimplify_products\\b"
        <<"\\bsimpsum\\b" <<"\\bsinnpiflag\\b" <<"\\bsolvedecomposes\\b" <<"\\bsolveexplicit\\b"
        <<"\\bsolvefactors\\b" <<"\\bsolve_inconsistent_error\\b" <<"\\bsolvenullwarn\\b" <<"\\bsolveradcan\\b"
        <<"\\bsolvetrigwarn\\b" <<"\\bsparse\\b" <<"\\bsqrtdispflag\\b" <<"\\bstardisp\\b"
        <<"\\bstats_numer\\b" <<"\\bstringdisp\\b" <<"\\bsublis_apply_lambda\\b" <<"\\bsumexpand\\b"
        <<"\\bsumsplitfact\\b" <<"\\btaylordepth\\b" <<"\\btaylor_logexpand\\b" <<"\\btaylor_order_coefficients\\b"
        <<"\\btaylor_truncate_polynomials\\b" <<"\\btensorkill\\b" <<"\\btestsuite_files\\b" <<"\\btimer_devalue\\b"
        <<"\\btlimswitch\\b" <<"\\btranscompile\\b" <<"\\btransrun\\b" <<"\\btr_array_as_ref\\b"
        <<"\\btr_bound_function_applyp\\b" <<"\\btr_file_tty_messagesp\\b" <<"\\btr_float_can_branch_complex\\b" <<"\\btr_function_call_default\\b"
        <<"\\btrigexpandplus\\b" <<"\\btrigexpandtimes\\b" <<"\\btriginverses\\b" <<"\\btrigsign\\b"
        <<"\\btr_numer\\b" <<"\\btr_optimize_max_loop\\b" <<"\\btr_semicompile\\b" <<"\\btr_state_vars\\b"
        <<"\\btr_warn_bad_function_calls\\b" <<"\\btr_warn_fexpr\\b" <<"\\btr_warn_meval\\b" <<"\\btr_warn_mode\\b"
        <<"\\btr_warn_undeclared\\b" <<"\\btr_warn_undefined_variable\\b" <<"\\btr_windy\\b" <<"\\bttyoff\\b"
        <<"\\buse_fast_arrays\\b" <<"\\bvalues\\b" <<"\\bvect_cross\\b" <<"\\bverbose\\b"
        <<"\\bzerobern\\b" <<"\\bzeta%pi\\b" ;

    foreach (const QString &pattern, maximaVariablePatterns )
    {
        rule.pattern = QRegExp(pattern);
        rule.format = maximaVariableFormat;
        m_highlightingRules.append(rule);
    }


    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    m_highlightingRules.append(rule);

    rule.pattern= QRegExp("'.*'");
    rule.format = quotationFormat;
    m_highlightingRules.append(rule);

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}

MaximaHighlighter::~MaximaHighlighter()
{
}

void MaximaHighlighter::highlightBlock(const QString& text)
{
    if(text.isEmpty())
        return;

    //Do some backend independent highlighting (brackets etc.)
    DefaultHighlighter::highlightBlock(text);

    foreach (const HighlightingRule &rule,  m_highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index,  length,  rule.format);
            index = expression.indexIn(text,  index + length);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = commentStartExpression.indexIn(text);

    while (startIndex >= 0) {

        int endIndex = commentEndExpression.indexIn(text,  startIndex);
        int commentLength;
        if (endIndex == -1) {

            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                + commentEndExpression.matchedLength();
        }
        setFormat(startIndex,  commentLength,  multiLineCommentFormat);
        startIndex = commentStartExpression.indexIn(text,  startIndex + commentLength);
    }
}
