class Character {
private:
    static constexpr int minimum_health_ = 0;
    static constexpr int maximum_health_ = 100;
    static constexpr int minimum_attack_power_ = 0;
    static constexpr int maximum_attack_power_ = 1000;

    std::string name_;
    int health_;
    int attack_power_;
    std::vector<std::string> skills_;
    
    static void validate_name(std::string_view name) {
        if (name.empty()) {
            throw std::invalid_argument("Name must not be empty!");
        }
    }
    
    static void validate_health(int health) {
        if (health < minimum_health_ || health > maximum_health_) {
            throw std::invalid_argument(
                "Health must be in the valid range: [" + 
                std::to_string(minimum_health_) + 
                ',' +
                std::to_string(maximum_health_) + 
                ']'
            );
        }
    }
    
    static void validate_attack_power(int attack_power) {
        if (
            attack_power < minimum_attack_power_ || 
            attack_power > maximum_attack_power_
        ) {
            throw std::invalid_argument(
                "Attack power must be in the valid range: [" + 
                std::to_string(minimum_attack_power_) + 
                ',' +
                std::to_string(maximum_attack_power_) + 
                ']'
            );
        }
    }
    
    static void validate_skill(std::string_view skill) {
        if (skill.empty()) {
            throw std::invalid_argument("Skill must not be empty!");
        }
    }
    
    static void validate_skills(const std::vector<std::string>& skills) {
        for (const std::string& skill : skills) {
            validate_skill(skill);
        }
    }

public:
    Character(
        std::string name, 
        int health, 
        int attack_power, 
        std::vector<std::string> skills
    ) : name_(name),
        health_(health),
        attack_power_(attack_power),
        skills_(skills)
    {
        validate_name(name_),
        validate_health(health_),
        validate_attack_power(attack_power_),
        validate_skills(skills_)
    }
};
