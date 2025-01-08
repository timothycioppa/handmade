#ifndef _GAME_AUDIO_HPP
#define _GAME_AUDIO_HPP

#define AUDIO_ENTRY(index, groupIndex, flag, source) flag = index,
enum SoundCode
{
    #include "audio_data.hpp"
};
#undef AUDIO_ENTRY

void Audio_Init();
void Audio_Shutdown();
void Audio_PlayAllSounds();
void Audio_PlaySound(SoundCode code);
void Audio_PlaySound(SoundCode code, bool loop);
void Audio_StopSound(SoundCode code);
void Audio_LoopSound(SoundCode code);

#endif