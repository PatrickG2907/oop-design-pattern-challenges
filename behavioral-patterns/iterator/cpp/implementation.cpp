// ============================================================================
// ITERATOR PATTERN: MUSIC PLAYLIST
// ============================================================================
//
// ARCHITECTURE OVERVIEW
// ---------------------
// Goal:
//   Traverse the same playlist in different ways without exposing its internal
//   std::vector<Song> or changing the client code for each traversal strategy.
//
// Main components:
//   1. Song
//      A simple data object containing a title and an explicit-content flag.
//
//   2. Playlist (collection)
//      Owns the songs and provides read-only indexed access.
//
//   3. collectSongIndices() (shared helper)
//      Validates a half-open range [first, last) and selects eligible song
//      indices, optionally excluding explicit tracks.
//
//   4. IPlaylistIterator (iterator interface)
//      Defines hasNext() and next() for all traversal strategies.
//
//   5. IndexedPlaylistIterator (shared iterator implementation)
//      Stores an ordered sequence of indices and implements cursor-based
//      traversal once for NormalIterator and ShuffleIterator.
//
//   6. NormalIterator (concrete iterator)
//      Uses the eligible indices in their original playlist order.
//
//   7. ShuffleIterator (concrete iterator)
//      Shuffles the eligible indices using a seeded random-number generator.
//      The actual song collection is never reordered.
//
//   8. RepeatIterator (concrete iterator)
//      Traverses a selected subset multiple times. It stores each eligible
//      index only once and tracks repetition state using counters.
//
//   9. printSongs() and main() (client)
//      Use IPlaylistIterator polymorphically, without knowing which concrete
//      iterator supplies the next song.
//
// Relationships:
//
//   Playlist <----------------- all iterators (non-owning const reference)
//       ^
//       | reads songs by index
//   collectSongIndices() ------> builds eligible index sequences
//
//   IPlaylistIterator
//      |-- IndexedPlaylistIterator
//      |      |-- NormalIterator
//      |      `-- ShuffleIterator
//      `-- RepeatIterator
//
// Usage contract:
//   The Playlist must outlive its iterators. Do not modify the playlist while
//   using an iterator: traversal indices are computed at construction time,
//   and appending songs may invalidate previously returned Song references.
//   This educational implementation does not enforce that contract at runtime.
//
// ============================================================================
// SECTION 1 — REQUIRED HEADERS
// ============================================================================

#include <algorithm>  // std::shuffle
#include <cstddef>    // std::size_t
#include <iostream>   // std::cout
#include <random>     // std::mt19937
#include <stdexcept>  // std::out_of_range
#include <string>     // std::string
#include <utility>    // std::move
#include <vector>     // std::vector


// ============================================================================
// SECTION 2 — SONG: THE ELEMENT STORED IN THE COLLECTION
// ============================================================================

// A simple aggregate: clients can construct a Song with Song{"Title", false}.
struct Song {
    std::string title; 
    bool isExplicit;
};


// ============================================================================
// SECTION 3 — PLAYLIST: THE COLLECTION
// ============================================================================

class Playlist {
private:
    // Only Playlist owns the actual songs. Iterators store song indices,
    // rather than their own copies of Song objects.
    std::vector<Song> songs_;
public:
    // Accept by value, then move into storage. This supports both temporary
    // Song objects and values supplied by callers.
    void addSong(Song song) {
        songs_.push_back(std::move(song));
    }
    
    // Number of songs currently stored in the playlist.
    std::size_t size() const {
        return songs_.size();
    }
    
    // Give iterators read-only access to a song at a particular index.
    // Validate the index before accessing the underlying vector.
    const Song& getSong(std::size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("Index exceeds vector size");
        }
        return songs_[index];
    } 
};


// ============================================================================
// SECTION 4 — SHARED HELPER: RANGE VALIDATION AND EXPLICIT-TRACK FILTERING
// ============================================================================

// Collect eligible indices from the half-open range [first, last).
//
// Example:
//   Playlist indices: [0, 1, 2, 3, 4]
//   Song at index 2 is explicit.
//   collectSongIndices(playlist, 1, 4, true) returns {1, 3}.
//
// Notes:
//  - The helper does NOT copy songs or determine traversal behavior. 
//  - It only decides which indices belong in the traversal sequence.
std::vector<std::size_t> collectSongIndices(
    const Playlist& playlist,
    std::size_t first,
    std::size_t last,
    bool skipExplicit
) {
    // first == last is valid and produces an empty sequence.
    // last == playlist.size() is valid because last is exclusive.
    if (first > last || last > playlist.size()) {
        throw std::out_of_range("Invalid range selected.");   
    }
    
    std::vector<std::size_t> indices;
    
    for (std::size_t i = first; i < last; ++i) {
        // Include every song if filtering is disabled; otherwise include
        // only songs whose isExplicit flag is false.
        if (!skipExplicit || !playlist.getSong(i).isExplicit) {
            indices.push_back(i);
        }
    }
    
    return indices;
}


// ============================================================================
// SECTION 5 — ITERATOR INTERFACE: THE CLIENT-FACING CONTRACT
// ============================================================================

class IPlaylistIterator {
public:
    // Virtual destructor permits safe destruction through a base pointer.
    virtual ~IPlaylistIterator() = default;
    
    // Report whether the next() operation can return another song.
    virtual bool hasNext() const = 0; 
    
    // Return the next song by const reference and advance the iterator.
    // Implementations throw std::out_of_range when exhausted.
    virtual const Song& next() = 0; 
};


// ============================================================================
// SECTION 6 — INDEXED PLAYLIST ITERATOR: SHARED TRAVERSAL IMPLEMENTATION
// ============================================================================

// Reuses the same cursor logic for both normal and shuffled traversal.
// Derived classes prepare the order of indices; this class performs traversal.
class IndexedPlaylistIterator : public IPlaylistIterator {
private:
    const Playlist& playlist_;  // Non-owning reference to the collection.
    std::size_t cursor_{0};     // Next position inside order_.
protected:
    // Protected so ShuffleIterator can rearrange the inherited index vector.
    std::vector<std::size_t> order_;
    
    // Only derived iterator classes should construct this shared base.
    // The prepared vector is moved into place rather than copied.
    IndexedPlaylistIterator(
        const Playlist& playlist,
        std::vector<std::size_t> order
    ) :
        playlist_(playlist),
        order_(std::move(order))
    {
    }
public:
    // Traversal is possible while the cursor points to a valid position
    // inside the prepared index vector.
    bool hasNext() const override {
        return cursor_ < order_.size();
    }
    
    const Song& next() override {
        // Prevent accessing the index vector beyond its valid range.
        if (!hasNext()) {
            throw std::out_of_range("Iterator exhausted. No more songs.");
        }
        
        // order_[cursor_] is the song's index in Playlist, not the song.
        const Song& song = playlist_.getSong(order_[cursor_]);
        
        // Advance the cursor for the next call.
        ++cursor_;
        
        // Return a reference instead of copying the Song object.
        return song;
    }
};


// ============================================================================
// SECTION 7 — NORMAL ITERATOR: ORIGINAL PLAYLIST ORDER
// ============================================================================

// No custom hasNext() or next() is needed; both are inherited.
//
// The constructor collects the eligible song indices in their original order.
// The base class then performs traversal using those prepared indices.
class NormalIterator : public IndexedPlaylistIterator {
public:
    NormalIterator(
       const Playlist& playlist, 
       bool skipExplicit
    ) : 
        IndexedPlaylistIterator(
            playlist,
            collectSongIndices(playlist, 0, playlist.size(), skipExplicit)
        )
    {
    }
};


// ============================================================================
// SECTION 8 — SHUFFLE ITERATOR: RANDOMIZED INDEX ORDER
// ============================================================================

// Shuffling order_ changes only the traversal order, NOT Playlist's storage.
//
// The constructor first collects eligible indices through the shared helper,
// then shuffles those indices before traversal begins.
class ShuffleIterator : public IndexedPlaylistIterator {
public:
    ShuffleIterator(
       const Playlist& playlist, 
       bool skipExplicit,
       unsigned int seed
    ) : 
        IndexedPlaylistIterator(
            playlist,
            collectSongIndices(playlist, 0, playlist.size(), skipExplicit)
        )
    {
        // A fixed seed makes runs repeatable with the same standard-library
        // implementation. Exact shuffle permutations are not portable.
        std::mt19937 engine{seed};

        // Randomize the index vector inherited from IndexedPlaylistIterator.
        // The original Playlist remains unchanged.
        std::shuffle(order_.begin(), order_.end(), engine);
    }
};


// ============================================================================
// SECTION 9 — REPEAT ITERATOR: MEMORY-EFFICIENT SUBSET REPETITION
// ============================================================================

// Unlike IndexedPlaylistIterator, this iterator maintains a second level of
// traversal state: which full pass through the selected subset is in progress.
//
// Example:
//   order_ = {1, 3}, times_ = 3
//   next() returns indices 1, 3, 1, 3, 1, 3.
//   The vector contains only {1, 3}; repetitions are tracked with counters.
//
// If k songs pass the filter, traversal storage is O(k), independent of times_.
class RepeatIterator : public IPlaylistIterator {
private:
    const Playlist& playlist_;          // Non-owning reference to the collection.
    std::vector<std::size_t> order_;    // Eligible subset stored once.
    std::size_t cursor_{0};             // Next index within this subset.
    std::size_t completedPasses_{0};    // Number of finished full traversals.
    std::size_t times_;                 // Total traversals requested.
public:
    RepeatIterator(
        const Playlist& playlist,
        std::size_t first,
        std::size_t last,
        std::size_t times,
        bool skipExplicit
    ) : 
        playlist_(playlist),
        order_(collectSongIndices(playlist, first, last, skipExplicit)),
        times_(times)
    {
    }
    
    bool hasNext() const override {
        // An empty subset means there are no songs to traverse.
        //
        // completedPasses_ < times_ ensures that the iterator stops
        // after the requested number of full repetitions.
        //
        // When times_ == 0, this condition is false immediately.
        return !order_.empty() && completedPasses_ < times_;
    }
    
    const Song& next() override {
        // Reject calls when the selected subset is empty or when all
        // requested repetitions have already been completed.
        if (!hasNext()) {
            throw std::out_of_range("Iterator exhausted. No more songs.");
        }
    
        // Retrieve the current song from the original Playlist.
        const Song& song = playlist_.getSong(order_[cursor_]);
        
        // Move to the next eligible index in the selected subset
        ++cursor_;
        
        // Completing the subset advances the pass counter, then rewinds
        // the cursor so the same stored indices can be used again.
        if (cursor_ == order_.size()) {
            cursor_ = 0;
            ++completedPasses_;
        }
        
        // After the final pass, hasNext() becomes false because
        // completedPasses_ == times_, even though cursor_ has reset to 0.
        return song;
    }
};


// ============================================================================
// SECTION 10 — CLIENT HELPER: POLYMORPHIC ITERATOR CONSUMPTION
// ============================================================================

// Uses only the IPlaylistIterator interface, so this function works with
// NormalIterator, ShuffleIterator, RepeatIterator, or future implementations.
//
// Runtime polymorphism ensures that the appropriate hasNext() and next()
// implementations are invoked for the concrete iterator.
//
// Calling next() consumes an item; an exhausted iterator cannot be replayed.
void printSongs(IPlaylistIterator& iterator) {
    while (iterator.hasNext()) {
        std::cout << iterator.next().title << '\n';
    }
}


// ============================================================================
// SECTION 11 — DEMONSTRATION: THREE TRAVERSALS OF ONE PLAYLIST
// ============================================================================

int main() {
    Playlist playlist;
    
    // Create a sample playlist containing five songs.
    // Song C is the sole explicit song and should be filtered out below.
    playlist.addSong(Song { "Song A", false });
    playlist.addSong(Song { "Song B", false });
    playlist.addSong(Song { "Song C", true });
    playlist.addSong(Song { "Song D", false });
    playlist.addSong(Song { "Song E", false });
    
    // ------------------------------------------------------------------------
    // 11.1 — NORMAL TRAVERSAL
    // ------------------------------------------------------------------------

    // Visits A, B, D, E in their original order.
    // The second argument enables explicit-track filtering.
    NormalIterator normal(playlist, true);
    std::cout << "Normal:\n";
    printSongs(normal);
    
    // ------------------------------------------------------------------------
    // 11.2 — SHUFFLED TRAVERSAL
    // ------------------------------------------------------------------------

    // Visits the same four eligible songs exactly once, but in a randomized
    // order determined by the supplied seed and standard-library implementation.
    //
    // The original Playlist remains unchanged.
    ShuffleIterator shuffled(playlist, true, 42u);
    std::cout << "Shuffled:\n";
    printSongs(shuffled);
    
    // ------------------------------------------------------------------------
    // 11.3 — REPEATED SUBSET TRAVERSAL
    // ------------------------------------------------------------------------

    // Select the half-open range [1, 4):
    //
    //   Index 1: Song B (not explicit)
    //   Index 2: Song C (explicit)
    //   Index 3: Song D (not explicit)
    //
    // Skip Song C and repeat the remaining sequence three times.
    //
    // Expected:
    //   Song B
    //   Song D
    //   Song B
    //   Song D
    //   Song B
    //   Song D
    RepeatIterator repeated(playlist, 1, 4, 3, true);
    std::cout << "Repeated:\n";
    printSongs(repeated);
    
    return 0;
}
