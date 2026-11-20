from dataclasses import dataclass
from abc import ABC, abstractmethod
from typing import List, Iterator as TypingIterator
import random

#----------------------------
# Song Model
#----------------------------
@dataclass
class Song:
    title: str
    artist: str
    duration_seconds: int
    explicit: bool = False

#----------------------------
# Filter Interface
#----------------------------
class SongFilter(ABC):
    @staticmethod
    @abstractmethod
    def apply(songs: List[Song], **criteria) -> List[Song]:
        """Return a filtered list of songs based on given criteria."""
        pass

#----------------------------
# Concrete Filter: Explicit Songs
#----------------------------
class ExplicitSongFilter(SongFilter):
    @staticmethod
    def apply(songs: List[Song], **criteria) -> List[Song]:
        skip_explicit = criteria.get("skip_explicit", False)
        if skip_explicit:
            return [song for song in songs if not song.explicit]
        return songs

#----------------------------
# Concrete Filter: Duration Filter
#----------------------------
class DurationSongFilter(SongFilter):
    @staticmethod
    def apply(songs: List[Song], **criteria) -> List[Song]:
        min_duration = criteria.get("min_duration", 0)
        max_duration = criteria.get("max_duration", float("inf"))
        return [song for song in songs if min_duration <= song.duration_seconds <= max_duration]

#----------------------------
# Iterator Interface
#----------------------------
class SongIterator(ABC, TypingIterator[Song]):
    @abstractmethod
    def __next__(self) -> Song:
        pass

    def __iter__(self):
        return self

#----------------------------
# Concrete Iterator: Sequential
#----------------------------
class SequentialSongIterator(SongIterator):
    def __init__(self, songs: List[Song]) -> None:
        self._songs = songs
        self._index = 0

    def __next__(self) -> Song:
        if self._index >= len(self._songs):
            raise StopIteration
        song = self._songs[self._index]
        self._index += 1
        return song

#----------------------------
# Concrete Iterator: Shuffled
#----------------------------
class ShuffledSongIterator(SongIterator):
    def __init__(self, songs: List[Song]) -> None:
        self._songs = random.sample(songs, len(songs))  # shuffled copy
        self._index = 0

    def __next__(self) -> Song:
        if self._index >= len(self._songs):
            raise StopIteration
        song = self._songs[self._index]
        self._index += 1
        return song

#----------------------------
# Concrete Iterator: Repeated
#----------------------------
class RepeatedSongIterator(SongIterator):
    def __init__(self, songs: List[Song], repeat: int = 1) -> None:
        """
        Repeats the given list of songs sequentially `repeat` times.
        """
        self._songs = songs * repeat  # sequential repetition
        self._index = 0

    def __next__(self) -> Song:
        if self._index >= len(self._songs):
            raise StopIteration
        song = self._songs[self._index]
        self._index += 1
        return song

#----------------------------
# Playlist Class
#----------------------------
class Playlist:
    def __init__(self, songs: List[Song]):
        self._songs = songs

    def sequential_iterator(self, song_filter: SongFilter = ExplicitSongFilter, **criteria) -> SongIterator:
        filtered_songs = song_filter.apply(self._songs, **criteria)
        return SequentialSongIterator(filtered_songs)

    def shuffled_iterator(self, song_filter: SongFilter = ExplicitSongFilter, **criteria) -> SongIterator:
        filtered_songs = song_filter.apply(self._songs, **criteria)
        return ShuffledSongIterator(filtered_songs)
        
    def repeated_iterator(self, song_filter: SongFilter = ExplicitSongFilter, repeat: int = 1, **criteria) -> SongIterator:
        filtered_songs = song_filter.apply(self._songs, **criteria)
        return RepeatedSongIterator(filtered_songs, repeat=repeat)

#----------------------------
# Usage Example
#----------------------------
if __name__ == "__main__":
    songs = [
        Song("Song A", "Artist 1", 210),
        Song("Song B", "Artist 2", 180, explicit=True),
        Song("Song C", "Artist 3", 200),
        Song("Song D", "Artist 4", 240, explicit=True),
    ]

    playlist = Playlist(songs)

    print("Sequential (skip explicit):")
    for song in playlist.sequential_iterator(skip_explicit=True):
        print(f"{song.title} by {song.artist} ({song.duration_seconds}s)")

    print("\nSequential (all tracks, duration >= 200s):")
    for song in playlist.sequential_iterator(song_filter=DurationSongFilter, min_duration=200):
        print(f"{song.title} by {song.artist} ({song.duration_seconds}s)")

    print("\nShuffled (skip explicit):")
    for song in playlist.shuffled_iterator(skip_explicit=True):
        print(f"{song.title} by {song.artist} ({song.duration_seconds}s)")

    print("\nRepeated (skip explicit, repeat 2x):")
    for song in playlist.repeated_iterator(skip_explicit=True, repeat=2):
        print(f"{song.title} by {song.artist} ({song.duration_seconds}s)")
