from abc import ABC, abstractmethod
import random

# State Interface
class MediaPlayerState(ABC):
    @abstractmethod
    def play(self, player) -> None:
        pass

    @abstractmethod
    def pause(self, player) -> None:
        pass

    @abstractmethod
    def stop(self, player) -> None:
        pass

# Concrete States
class StoppedState(MediaPlayerState):
    def play(self, player) -> None:
        if player.shuffle:
            player.current_track = random.choice(player.playlist)
        else:
            player.current_track = player.playlist[0]
        print(f"Playing track: {player.current_track}")
        player.state = PlayingState()

    def pause(self, player) -> None:
        print("Cannot pause. MediaPlayer is stopped.")

    def stop(self, player) -> None:
        print("MediaPlayer already stopped.")

class PlayingState(MediaPlayerState):
    def play(self, player) -> None:
        if player.shuffle:
            player.current_track = random.choice(player.playlist)
        else:
            # Move to next track
            idx = player.playlist.index(player.current_track)
            player.current_track = player.playlist[(idx + 1) % len(player.playlist)]
        print(f"Playing track: {player.current_track}")

    def pause(self, player) -> None:
        print("Pausing playback.")
        player.state = PausedState()

    def stop(self, player) -> None:
        print("Stopping playback.")
        player.state = StoppedState()
        player.current_track = None

class PausedState(MediaPlayerState):
    def play(self, player) -> None:
        print(f"Resuming track: {player.current_track}")
        player.state = PlayingState()

    def pause(self, player) -> None:
        print("MediaPlayer already paused.")

    def stop(self, player) -> None:
        print("Stopping playback from paused state.")
        player.state = StoppedState()
        player.current_track = None

# MediaPlayer Context
class MediaPlayer:
    def __init__(self, playlist) -> None:
        self.playlist = playlist
        self.current_track = None
        self.state: MediaPlayerState = StoppedState()
        self.shuffle = False

    def toggle_shuffle(self) -> None:
        self.shuffle = not self.shuffle
        print(f"Shuffle mode {'ON' if self.shuffle else 'OFF'}.")

    # Delegate actions to state
    def play(self) -> None:
        self.state.play(self)

    def pause(self) -> None:
        self.state.pause(self)

    def stop(self) -> None:
        self.state.stop(self)

# Example Usage
if __name__ == "__main__":
    playlist = ["Song A", "Song B", "Song C"]
    player = MediaPlayer(playlist)

    player.play()           # Starts playing first track
    player.toggle_shuffle() # Turn shuffle on
    player.play()           # Plays random next track
    player.pause()          # Pauses
    player.play()           # Resumes
    player.stop()           # Stops playback
