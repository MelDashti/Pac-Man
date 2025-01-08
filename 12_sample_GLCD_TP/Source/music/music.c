#include "music.h"

#include "../timer/timer.h"

void playNote(NOTE note)
{
	if(note.freq != pause)
	{
		reset_timer(0);
		init_timer(0, note.freq);
		enable_timer(0);
	}
	reset_timer(1);
	init_timer(1, note.duration);
	enable_timer(1);
}

BOOL isNotePlaying(void)
{
	return ((LPC_TIM0->TCR != 0) || (LPC_TIM1->TCR != 0));
}

static NOTE* current_sound;
static int current_sound_length = 0;

void playSoundEffect(NOTE* sound, int length) {
    current_sound = sound;
    current_sound_length = length;
}

NOTE pacman_wakka[] = {
    {e4, time_semibiscroma},
    {pause, time_semibiscroma}
};

NOTE power_pill_sound[] = {
    {b4, time_semicroma},
    {e4, time_semicroma},
    {b4, time_semicroma}
};

NOTE death_sound[] = {
    {b4, time_semicroma},
    {b3, time_semicroma},
    {a3, time_semicroma},
    {g3, time_semicroma},
    {f3, time_semiminima},
    {pause, time_semicroma}
};

NOTE game_start[] = {
    {c4, time_semicroma},
    {e4, time_semicroma},
    {g4, time_semicroma},
    {c5, time_semiminima}
};

NOTE victory_sound[] = {
    {c4, time_semicroma},
    {e4, time_semicroma},
    {g4, time_semicroma},
    {c5, time_semicroma},
};