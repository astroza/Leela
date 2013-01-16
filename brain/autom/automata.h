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

#ifndef AUTOMATA_H
#define AUTOMATA_H


#define AUTOM_BEGIN switch(current_step) { case 0:
#define AUTOM_END }
#define WAIT_UNTIL(cond) case __LINE__: if(!(cond)) { current_step = __LINE__; break; }
#define WAIT_WHILE(cond) WAIT_UNTIL(!(cond))
#define JUMP(autom) get_control()->jump(autom); \
		    case __LINE__: if(current_step != __LINE__) { current_step = __LINE__; break; }

class Automata;
class AutomataControl
{
	private:
		Automata *current;
	public:
		AutomataControl();
		void jump(Automata *aut);
		int do_step();
};

class Automata
{
	private:
		Automata *parent;
		AutomataControl *control;
	public:
		Automata();
		virtual int do_step() = 0;
		void next_step();
		void set_parent(Automata *parent);
		Automata *get_parent();
		void reset();
		AutomataControl *get_control();
		void set_control(AutomataControl *ac);
	protected:
		unsigned int current_step;
};

#endif
