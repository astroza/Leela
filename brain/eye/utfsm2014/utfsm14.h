#ifndef __UTFSM14__
#define __UTFSM14__

#include <automata.h>
#include <navig.h>

class Aligner : public Automata {
        private:
		Navig *navig;
		int old_v;
		bool is_increasing(int);
        public:          
                int do_step();
                Aligner(Navig *);
}; 


#endif
