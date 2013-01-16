/* (c) 2012, 2013 - Felipe Astroza Araya
 *
 *  This file is part of Leela.
 *
 *  Leela is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Leela is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Leela. If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include "automata.h"

void AutomataControl::jump(Automata *aut)
{
	aut->set_parent(current);
	aut->set_control(this);
	aut->reset();
	current = aut;
}

AutomataControl::AutomataControl()
{
	current = NULL;
}

int AutomataControl::do_step()
{
	Automata *parent;
	if(current->do_step() != 0) {
		parent = current->get_parent();
		current = parent;
		if(parent != NULL) {
			return 0;
		} else
			return -1;
	}
	return 0;
}

Automata::Automata()
{
	parent = NULL;
	control = NULL;
}

void Automata::set_parent(Automata *parent)
{
	this->parent = parent;
}

Automata *Automata::get_parent()
{
	return parent;
}

void Automata::next_step()
{
	current_step++;
}

void Automata::reset()
{
	current_step = 0;
}

AutomataControl *Automata::get_control()
{
	return control;
}

void Automata::set_control(AutomataControl *ac)
{
	control = ac;
}

