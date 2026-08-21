from utils import playtest, plot_stereo_audio, plot_stereo_spectrogram

in_path = "demo/audio/input/song_with_noise.wav"
out_path = "demo/audio/output/out.wav"

'''
playtest(in_path)
playtest(out_path)
'''

plot_stereo_audio(in_path,out_path)
plot_stereo_spectrogram(in_path, out_path)