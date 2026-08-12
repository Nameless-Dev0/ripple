#include <stdio.h>
#include "wav.h"

int main(){
    wav_t* wav_in = load_wav("audio/song.wav");

    if(is_valid_wav(wav_in))
        printf("VALID WAV!\n");
    else
        printf("INVALID WAV!\n");
    wav_info(wav_in);
    export_wav(wav_in, "audio/out.wav");
    release_wav(wav_in);

    wav_t* wav_out = load_wav("audio/exports/out.wav");
    if(is_valid_wav(wav_out))
        printf("VALID WAV!\n");
    else
        printf("INVALID WAV!\n");
    wav_info(wav_out);
    release_wav(wav_out);

    return 0;
}
