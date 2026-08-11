import time
from nava import play, stop

original_sound= play("audio/song.wav", async_mode=True, loop=False)
time.sleep(7)
stop(original_sound)

processed_sound= play("audio/output.wav", async_mode=True, loop=False)
time.sleep(7)
stop(processed_sound)
