class Flyweight {
public:
    virtual ~Flyweight() = default;
    
    virtual void render(char value, bool highlighted) const = 0;
};


class CharacterStyle : public Flyweight {
private:
    const bool bold_;
    const bool italic_;
    const std::string color_;
    
    static void validate_color(std::string_view color) {
        if (color.empty()) {
            throw std::invalid_argument("Color must not be empty!");
        }
    }
    
public:
    CharacterStyle(bool bold, bool italic, std::string color)
        : bold_(bold), italic_(italic), color_(std::move(color))
    {
        validate_color(color_);
    }
    
    void render(char value, bool highlighted) const override {
        std::cout
            << "Rendering character: "
            << value
            << ". Formatting options: "
            << "bold: "
            << ((bold_) ? "true" : "false")
            << ", italic: "
            << ((italic_) ? "true" : "false")
            << ", color: "
            << color_
            << ". Highlighted: "
            << ((highlighted) ? "true" : "false")
            << '\n';
    }
};


struct StyleKey{
    bool bold;
    bool italic;
    std::string color;
    
    auto operator<=>(const StyleKey& other) const = default;
};



class StyleFactory {
private:
    std::map<StyleKey, std::unique_ptr<const Flyweight>> styles_;

public:
    const Flyweight* getStyle(
        bool bold,
        bool italic,
        std::string_view color
    ) {
        StyleKey key{bold, italic, std::string{color}};

        // Check whether the style already exists
        auto it = styles_.find(key);

        if (it != styles_.end()) {
            return it->second.get();
        }

        // Create a new Flyweight
        auto style = std::make_unique<CharacterStyle>(
            bold, italic, key.color
        );

        // Save its address before transferring ownership
        const Flyweight* ptr = style.get();

        // Transfer ownership to the factory
        styles_.emplace(std::move(key), std::move(style));

        return ptr;
    }
};


class Character {
private:
    char value_;
    const Flyweight* style_;
    
    static void validate_style(const Flyweight* style) {
        if (!style) {
            throw std::invalid_argument("Flyweight pointer must not be null!");
        }
    }

public:
    Character(char value, const Flyweight* style)
        : value_(value), style_(style)
    {
        validate_style(style_);
    }
    
    void display(bool highlighted) const {
        style_->render(value_, highlighted);
    }
};


class TextEditor {
private:
    StyleFactory styleFactory_;
    std::vector<Character> characters_;
    std::set<std::size_t> highlightedPositions_;
    
public:
    void addCharacter(char value, bool bold, bool italic, std::string_view color) {
        auto ptr = styleFactory_.getStyle(bold, italic, color);
        characters_.emplace_back(value, ptr);
    }
    
    void highlight(std::size_t position) {
        if (position >= characters_.size()) {
            throw std::out_of_range("Invalid position!");
        }
        
        highlightedPositions_.insert(position); 
    }
    
    void remove_highlight(std::size_t position) {
        if (position >= characters_.size()) {
            throw std::out_of_range("Invalid position!");
        }
        
        highlightedPositions_.erase(position); 
    }
    
    void display() const {
        for (std::size_t i = 0; i < characters_.size(); i++) {
            bool highlight = highlightedPositions_.contains(i);
            characters_[i].display(highlight);
        }
    }
};
