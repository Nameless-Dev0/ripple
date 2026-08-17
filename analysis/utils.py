import time
import numpy as np
import matplotlib.pyplot as plt
import librosa
import soundfile as sf
from nava import play, stop

def playtest(path):
    sound= play(path, async_mode=True, loop=False)
    time.sleep(7)
    stop(sound)


def plot_stereo_audio(in_path, out_path):
    # sample rate is the same for input and output anyways
    in_waveform, sample_rate = librosa.load(in_path, sr=None, mono=False)
    out_waveform, sample_rate = librosa.load(out_path, sr=None, mono=False)

    fig, ax = plt.subplots(nrows=2, ncols=2, sharex=False, sharey=False, figsize=(8, 6))

    librosa.display.waveshow(in_waveform[0], sr=sample_rate, ax=ax[0, 0], color="cyan")
    librosa.display.waveshow(in_waveform[1], sr=sample_rate, ax=ax[0, 1], color="magenta")
    librosa.display.waveshow(out_waveform[0], sr=sample_rate, ax=ax[1, 0], color="cyan")
    librosa.display.waveshow(out_waveform[1], sr=sample_rate, ax=ax[1, 1], color="magenta")

    ax[0, 0].set_title("Input - Left Channel")
    ax[0, 1].set_title("Input - Right Channel")
    ax[1, 0].set_title("Output - Left Channel")
    ax[1, 1].set_title("Output - Right Channel")

    for a in ax.flat:
        a.grid(True, alpha=0.3)
        a.set_axisbelow(True)
        a.set_xlim(0, 60)
        a.set_ylim(-1, 1)
        a.set_xlabel("Time (s)")
        a.set_ylabel("Amplitude")

    fig.tight_layout()
    plt.show()

def plot_stereo_spectrogram(in_path, out_path, n_fft=2048, hop_length=1024):
    in_waveform, sample_rate = sf.read(in_path, always_2d=True)
    in_waveform = in_waveform.T
    out_waveform, _ = sf.read(out_path, always_2d=True)
    out_waveform = out_waveform.T

    in_stft = librosa.stft(in_waveform, n_fft=n_fft, hop_length=hop_length)
    out_stft = librosa.stft(out_waveform, n_fft=n_fft, hop_length=hop_length)

    in_db = librosa.amplitude_to_db(np.abs(in_stft), ref=np.max)
    out_db = librosa.amplitude_to_db(np.abs(out_stft), ref=np.max)

    fig, ax = plt.subplots(nrows=2, ncols=2, sharex=False, sharey=False, figsize=(10, 8))

    data = [in_db[0], in_db[1], out_db[0], out_db[1]]
    titles = [
        "Input - Left Channel",
        "Input - Right Channel",
        "Output - Left Channel",
        "Output - Right Channel",
    ]

    for a, d, title in zip(ax.flat, data, titles):
        img = librosa.display.specshow(
            d, sr=sample_rate, hop_length=hop_length,
            x_axis="time", y_axis="log", ax=a
        )
        img.set_rasterized(True)
        a.set_title(title)
        a.set_xlabel("Time (s)")
        a.set_ylabel("Frequency (Hz)")
        fig.colorbar(img, ax=a, format="%+2.0f dB")

    fig.tight_layout()
    plt.show()