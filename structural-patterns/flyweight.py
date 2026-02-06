from typing import Dict, Tuple

# ----------------------------
# Flyweight: CharacterStyle
# ----------------------------
class CharacterStyle:
    """
    Flyweight object representing a character's style.
    Intrinsic state: font style and base color.
    """
    def __init__(self, bold: bool, italic: bool, color: str):
        self.bold = bold
        self.italic = italic
        self.color = color

    def __repr__(self):
        return f"CharacterStyle(bold={self.bold}, italic={self.italic}, color='{self.color}')"

# ----------------------------
# Flyweight Factory
# ----------------------------
class StyleFactory:
    """
    Factory that reuses CharacterStyle instances.
    """
    _styles: Dict[Tuple[bool, bool, str], CharacterStyle] = {}

    @classmethod
    def get_style(cls, bold: bool, italic: bool, color: str) -> CharacterStyle:
        key = (bold, italic, color)
        if key not in cls._styles:
            cls._styles[key] = CharacterStyle(bold, italic, color)
        return cls._styles[key]

# ----------------------------
# Text character with flyweight
# ----------------------------
class Character:
    """
    Represents a single character.
    Extrinsic state: the actual character and optional dynamic highlight color.
    """
    def __init__(self, char: str, style: CharacterStyle):
        self.char = char
        self.style = style
        self.highlight_color = None  # Extrinsic state for temporary highlighting

    def display(self):
        """
        Display the character with its style.
        If highlight_color is set, it overrides the style's color temporarily.
        """
        color_to_use = self.highlight_color if self.highlight_color else self.style.color
        style_flags = []
        if self.style.bold:
            style_flags.append("bold")
        if self.style.italic:
            style_flags.append("italic")
        style_flags_str = "+".join(style_flags) if style_flags else "normal"
        print(f"{self.char} [{style_flags_str}, color={color_to_use}]", end=" ")

# ----------------------------
# Text document using characters
# ----------------------------
class TextDocument:
    """
    Manages a collection of characters and allows dynamic highlighting.
    """
    def __init__(self):
        self.characters = []

    def add_text(self, text: str, style: CharacterStyle):
        for c in text:
            self.characters.append(Character(c, style))

    def highlight_range(self, start: int, end: int, color: str):
        """
        Temporarily highlights characters from start to end (inclusive start, exclusive end).
        """
        for i in range(start, min(end, len(self.characters))):
            self.characters[i].highlight_color = color

    def remove_highlight(self, start: int, end: int):
        """
        Removes temporary highlight from a range.
        """
        for i in range(start, min(end, len(self.characters))):
            self.characters[i].highlight_color = None

    def display(self):
        for char in self.characters:
            char.display()
        print("\n")  # newline after displaying the document

# ----------------------------
# Example usage
# ----------------------------
if __name__ == "__main__":
    # Create styles via factory
    normal_style = StyleFactory.get_style(bold=False, italic=False, color="black")
    bold_style = StyleFactory.get_style(bold=True, italic=False, color="black")
    italic_red_style = StyleFactory.get_style(bold=False, italic=True, color="red")

    # Create document
    doc = TextDocument()
    doc.add_text("Hello ", normal_style)
    doc.add_text("World", bold_style)
    doc.add_text("!", italic_red_style)

    print("Original text:")
    doc.display()

    # Temporary highlight "World" in yellow
    doc.highlight_range(6, 11, "yellow")
    print("Text with dynamic highlighting:")
    doc.display()

    # Remove highlight
    doc.remove_highlight(6, 11)
    print("After removing highlight:")
    doc.display()

    # Memory check: all styles reused
    print("Total unique style objects:", len(StyleFactory._styles))
