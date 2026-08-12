#include "wav.h"

int main(){
    wav_t* wav_in = load_wav("audio/song.wav");
    wav_info(wav_in);
    export_wav(wav_in);
    release_wav(wav_in);

    wav_t* wav_out = load_wav("audio/out.wav");
    wav_info(wav_out);
    release_wav(wav_out);

    return 0;
}
