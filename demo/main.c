#include <stdio.h>
#include "wav.h"
#include "engine.h"

#define LOG_ERR \
do{             \
    get_err_msg(status, msg, sizeof(msg)); \
    fprintf(stderr, "%s\n", msg);          \
} while (0)

wav_status_t status;
char msg[128];

int main(){
    const char* input_path = "demo/audio/input/input_8.wav";
    const char* output_path = "demo/audio/output/output_8.wav";

    wav_t* wav_in = load_wav(input_path, &status);
    LOG_ERR;

    status = export_wav(wav_in, output_path);
    LOG_ERR;

    wav_t* wav_out = load_wav(output_path, &status);
    LOG_ERR;

    lpf_test(wav_out);

    status = export_wav(wav_out, output_path);
    LOG_ERR;
    
    status = release_wav(wav_in);
    LOG_ERR;

    status = release_wav(wav_out);
    LOG_ERR;
    
    { 
        int* p = malloc(sizeof(int));
    }
    

    return 0;
}
