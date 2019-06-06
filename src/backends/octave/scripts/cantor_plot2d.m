%{
    Copyright (C) 2010 Miha Čančula <miha.cancula@gmail.com>

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
%}

function cantor_plot2d(f_string,var,min,max)
   f_mod_string = f_string;
  for op = ['*' '/' '^']
    f_mod_string = strrep(f_mod_string, op, strcat('.',op));
  endfor

  f = inline(f_mod_string, var);
  x = linspace(min,max);
  plot (x, f(x));

  xlabel(var);
  ylabel(f_string);
endfunction
