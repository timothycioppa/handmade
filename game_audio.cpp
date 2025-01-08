#include "game_audio.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "include/miniaudio.h"
#include <stdio.h>

ma_engine engine;
ma_sound_group groups[2]; // music -> effects
#define MUSIC_GROUP 0
#define EFFECTS_GROUP 1

#define INIT_SOUND(src, entry, grp) entry.group = grp; ma_sound_init_from_file ( \
  &engine,\
  src,\
  MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION, \
  entry.group, \
  NULL,  \
  &entry.sound \
); \

struct SoundEntry
{
    bool initialized;
    ma_sound sound;
    ma_sound_group *group;        
};

#define MAX_SOUNDS 10
SoundEntry soundEntries[MAX_SOUNDS];

struct SoundQueueEntry 
{ 
  SoundCode code;
  bool loop;
};

// THIS DETERMINES HOW MANY SOUNDS CAN BE PLAYED PER FRAME
// probably needs tweeking
#define QUEUE_SIZE 10
SoundQueueEntry soundQueue[QUEUE_SIZE];
int q_head = 0;
int q_tail = 0;


void Audio_PlaySound(SoundCode code, bool loop) 
{
    SoundQueueEntry &entry = soundQueue[q_head];
    entry.code = code;
    entry.loop = loop;
    q_head = (q_head + 1) % QUEUE_SIZE; 
}

void Audio_StopSound(SoundCode code) 
{
    SoundEntry entry = soundEntries[int(code)];
    ma_sound_set_looping(&entry.sound, false);
    ma_sound_stop(&entry.sound);
}

void Audio_PlaySound(SoundCode code)
{
    Audio_PlaySound(code, false);
}

void Audio_LoopSound(SoundCode code) 
{ 
    Audio_PlaySound(code, true);
}

void Audio_PlayAllSounds() 
{ 
  while (q_tail != q_head)
  {
      SoundQueueEntry & queueEntry = soundQueue[q_tail];
      SoundEntry & entry = soundEntries[int(queueEntry.code)];
      ma_sound_set_looping(&entry.sound, queueEntry.loop);
      ma_sound_start(&entry.sound);
      q_tail = (q_tail + 1) % QUEUE_SIZE;
  }
}

void Audio_Init()
{    
    ma_result result =  ma_engine_init(NULL, &engine);

    ma_sound_group_init(&engine, 0, NULL, &groups[MUSIC_GROUP]);    
    ma_sound_group_set_volume(&groups[MUSIC_GROUP], .1f);
    
    ma_sound_group_init(&engine, 0, NULL, &groups[EFFECTS_GROUP]);
    ma_sound_group_set_volume(&groups[EFFECTS_GROUP], 0.2f);

    #define AUDIO_ENTRY(index, groupIndex, flag, src) \
      INIT_SOUND(src, soundEntries[int(flag)], &groups[groupIndex]);\
      soundEntries[int(flag)].initialized = true; \

    #include "audio_data.hpp"
    #undef AUDIO_ENTRY
}

void Audio_Shutdown() 
{ 
    ma_sound_group_stop(&groups[0]);
    ma_sound_group_stop(&groups[1]);
    ma_engine_uninit(&engine);
}