#include <stdio.h>
#include "wav.h"
#include "engine.h"

int main(){
    const char* input_path = "demo/audio/input.wav";
    const char* output_path = "demo/audio/output.wav";

    wav_t* wav_in = load_wav(input_path, NULL);
    export_wav(wav_in, output_path);

    wav_t* wav_out = load_wav(output_path, NULL);
    gain(wav_out, 2);
    export_wav(wav_out, output_path);
    wav_info(wav_out);
    
    release_wav(wav_in);
    release_wav(wav_out);
    return 0;

}
