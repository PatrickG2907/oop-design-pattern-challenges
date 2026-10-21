// ============================================================================
// Facade Pattern
//
// Facade provides a simplified, high-level interface to a group of lower-level
// subsystems. The client expresses what it wants to do, while the Facade hides
// which subsystem operations are required and how they must be coordinated.
//
// Pattern roles in this example:
//   - Facade:      SmartHome
//   - Subsystems:  Lights, AC, Music
//   - Configuration: VacationSettings
//   - Client:      main()
//
// SmartHome owns the subsystem objects and exposes high-level operations such
// as start_party(), end_party(), and vacation_mode(). The client therefore
// does not need to coordinate Lights, AC, and Music directly.
//
// The Facade hides orchestration, not necessarily configuration. The client
// may still provide meaningful high-level values such as a desired
// temperature, song, or vacation schedule.
//
// Facade does not require inheritance or a common interface with its
// subsystems. SmartHome coordinates the concrete subsystem objects through
// composition and presents a different, higher-level API to the client.
//
// The subsystem APIs are intentionally kept simple. Scheduling is represented
// by schedule() operations rather than real timers, threads, or persistence,
// keeping the example focused on the Facade pattern rather than building a
// complete home-automation system.
// ============================================================================


#include <iostream>
#include <stdexcept>
#include <string_view>


// ============================================================================
// Subsystems
// ============================================================================

// Lights exposes the lower-level operations available to the lighting
// subsystem. 
// Notes:
//  - SmartHome decides when these operations should be combined with 
//    operations from other subsystems.
//  - schedule() models a complete on/off schedule without implementing an
//    actual timer. 
//  - Validation remains inside the subsystem that owns the rule.
class Lights {
public:
    void off() const {
        std::cout << "\tLights: off\n";
    }
    
    void on() const {
        std::cout << "\tLights: on\n";
    }
    
    // This demo records the scheduling intent through console output.
    // Real scheduling infrastructure is deliberately outside the scope
    // of the Facade example.
    void schedule(
        std::string_view on_time,
        std::string_view off_time
    ) const {
        if (on_time.empty() || off_time.empty()) {
            throw std::invalid_argument("Schedule times must not be empty!");
        }

        std::cout
            << "\tLights: scheduled on at " << on_time
            << ", off at " << off_time
            << '\n';
    }
};


// AC provides independent climate-control operations.
// Notes:
//  - Both direct temperature changes and scheduled operation remain subsystem
//    responsibilities. 
//  - The Facade only coordinates when and why they are used.
class AC {
public:
    void off() const {
        std::cout << "\tAC: off\n";
    }
    
    void on() const {
        std::cout << "\tAC: on\n";
    }
    
    void set_temperature(int temperature) const {
        std::cout 
            << "\tAC: set temperature to " 
            << temperature
            << '\n';
    }
    
    void schedule(
        int temperature,
        std::string_view start_time,
        std::string_view end_time
    ) const {
        if (start_time.empty() || end_time.empty()) {
            throw std::invalid_argument("Schedule times must not be empty!");
        }
        
        std::cout
            << "\tAC: scheduled on at " << start_time
            << ", off at " << end_time
            << ", temperature set to " << temperature
            << '\n';
    }
};


// Music provides the lower-level music operations.
// Notes:
//  - Song validation belongs here rather than in SmartHome because Music owns
//    the rule that an empty song request is invalid.
class Music {
public:
    void off() const {
        std::cout << "\tMusic: off\n";
    }
    
    void on() const {
        std::cout << "\tMusic: on\n";
    }
    
    void play_song(std::string_view song) const {
        if (song.empty()) {
            throw std::invalid_argument("Song must not be empty!");
        }
        
        std::cout 
            << "\tMusic: play song " 
            << song 
            << '\n';
    }
};


// ============================================================================
// Facade Configuration
// ============================================================================

// Groups the related configuration needed by vacation_mode().
// Notes:
//  - The Facade hides subsystem orchestration, not user-configurable values.
//  - A configuration object is useful here because vacation mode needs several
//    closely related values that would otherwise become an awkward positional
//    parameter list.
//  - Simpler Facade operations keep their direct parameters.
//  - The string_view fields are non-owning; this demo initializes them from
//    string literals with static lifetime.
struct VacationSettings {
    std::string_view lights_on_time;
    std::string_view lights_off_time;
    int ac_temperature;
    std::string_view ac_start_time;
    std::string_view ac_end_time;
};


// ============================================================================
// Facade
// ============================================================================

// SmartHome is the Facade.
// Notes:
//  - It owns the concrete subsystems because they are simple implementation
//    details whose lifetime naturally belongs to the smart-home system.
//  - The client interacts with these high-level operations instead of knowing
//    which subsystem calls are required or in what order they should occur.
class SmartHome {
private:
    // Subsystems are hidden behind the Facade and owned directly by value.
    Lights lights_;
    AC ac_;
    Music music_;

public:
    // Coordinates several subsystem operations behind one high-level request.
    // Note:
    //  - The client chooses the desired temperature and song, while SmartHome 
    //    hides the individual steps required to activate party mode.
    void start_party(int temperature, std::string_view song) const {
        std::cout << "Start party mode.\n";
        lights_.on();
        ac_.on();
        ac_.set_temperature(temperature);
        music_.on();
        music_.play_song(song);
    }
        
    // Ends party mode and returns the home to a normal comfort configuration.
    void end_party(int temperature) const {
        std::cout << "End party mode.\n";
        lights_.off();
        ac_.on();
        ac_.set_temperature(temperature);
        music_.off();
    }
    
    // Converts one high-level vacation configuration into coordinated subsystem
    // operations. 
    // Notes:
    //  - The client does not need direct access to Lights, AC, or Music.
    //  - Configuration remains customizable, but the orchestration stays hidden
    //    behind the Facade.
    void vacation_mode(const VacationSettings& settings) const {
        std::cout << "Activate vacation mode.\n";
        lights_.schedule(
            settings.lights_on_time, 
            settings.lights_off_time
        );
        
        ac_.schedule(
            settings.ac_temperature, 
            settings.ac_start_time, 
            settings.ac_end_time
        );
        
        music_.off();
    }
};


// ============================================================================
// Client / Demo
// ============================================================================

// The client works only with SmartHome, the Facade.
// Note:
//  - It expresses high-level intentions such as starting a party or activating
//    vacation mode without coordinating the individual subsystems itself.
int main() {
    SmartHome smart_home;
    
    smart_home.start_party(23, "Let's have a party!");
    smart_home.end_party(21);
    
    // Designated initialization keeps the multi-field configuration readable
    // and avoids relying on the positional order of several similar values.
    VacationSettings settings{
        .lights_on_time = "6 a.m.",
        .lights_off_time = "8 p.m.",
        .ac_temperature = 20,
        .ac_start_time = "9 p.m.",
        .ac_end_time = "5 a.m."
    };
    
    smart_home.vacation_mode(settings);
    
    return 0;
}
