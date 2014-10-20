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

bool forward_with_feedback(Navig *navig, int velocity, int cur_distance, int distance_to_keep);
bool obstacle_in_front();
bool obstacle_in_left_side();
bool obstacle_in_right_side();
float wall_sensor0_distance();
float wall_sensor1_distance();

#endif
