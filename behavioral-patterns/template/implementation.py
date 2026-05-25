from abc import ABC, abstractmethod
from typing import Any, List

#-----------------------------------------
# Abstract Quiz class with Template Method
#-----------------------------------------
class Quiz(ABC):
    def take_quiz(self) -> None:
        """Template method defining the quiz flow."""
        self.display_questions()
        answers: List[Any] = self.collect_answers()
        score: int = self.score_answers(answers)
        self.show_feedback(score)
        if self.has_bonus() and self.is_eligible_for_bonus(score):
            bonus_score: int = self.score_bonus()
            self.show_bonus_feedback(bonus_score)

    # Steps of the template
    @abstractmethod
    def display_questions(self) -> None:
        """Display quiz questions."""
        pass

    @abstractmethod
    def collect_answers(self) -> List[Any]:
        """Collect answers from the student."""
        pass

    @abstractmethod
    def score_answers(self, answers: List[Any]) -> int:
        """Score the given answers."""
        pass

    @abstractmethod
    def show_feedback(self, score: int) -> None:
        """Display the feedback for the main quiz."""
        pass

    # Hooks for optional steps
    def has_bonus(self) -> bool:
        """Hook: override if quiz has bonus questions"""
        return False

    def score_bonus(self) -> int:
        """Hook: override to score bonus questions"""
        return 0

    def show_bonus_feedback(self, bonus_score: int) -> None:
        """Hook: override to show bonus feedback"""
        print(f"Bonus Score: {bonus_score}")

    def is_eligible_for_bonus(self, score: int) -> bool:
        """Hook: determine if student can attempt bonus"""
        return score > 0  # default: must get at least 1 point to attempt bonus

#-----------------------------------------
# Concrete Quiz implementations
#-----------------------------------------
class MathQuiz(Quiz):
    def __init__(self) -> None:
        self.questions: List[str] = ["2+2=?", "5*6=?"]
        self.correct_answers: List[int] = [4, 30]
        self.bonus_question: str = "Square root of 144?"

    def display_questions(self) -> None:
        print("Math Quiz Questions:")
        for q in self.questions:
            print(q)

    def collect_answers(self) -> List[int]:
        print("Collecting answers...")
        return [4, 30]  # simulated answers

    def score_answers(self, answers: List[int]) -> int:
        return sum(1 for a, c in zip(answers, self.correct_answers) if a == c)

    def show_feedback(self, score: int) -> None:
        print(f"Math Quiz Score: {score}/{len(self.questions)}")

    def has_bonus(self) -> bool:
        return True

    def is_eligible_for_bonus(self, score: int) -> bool:
        # Eligible if all main questions are correct
        return score == len(self.correct_answers)

    def score_bonus(self) -> int:
        answer: int = 12  # simulated
        correct: int = 12
        return 1 if answer == correct else 0

class HistoryQuiz(Quiz):
    def __init__(self) -> None:
        self.questions: List[str] = ["Who discovered America?", "When was WW2?"]
        self.correct_answers: List[str] = ["Columbus", "1939-1945"]

    def display_questions(self) -> None:
        print("History Quiz Questions:")
        for q in self.questions:
            print(q)

    def collect_answers(self) -> List[str]:
        print("Collecting answers...")
        return ["Columbus", "1939-1945"]  # simulated answers

    def score_answers(self, answers: List[str]) -> int:
        return sum(1 for a, c in zip(answers, self.correct_answers) if a.lower() == c.lower())

    def show_feedback(self, score: int) -> None:
        print(f"History Quiz Score: {score}/{len(self.questions)}")

class CodingQuiz(Quiz):
    def __init__(self) -> None:
        self.questions: List[str] = ["What does OOP stand for?", "Python is dynamically typed? (yes/no)"]
        self.correct_answers: List[str] = ["Object-Oriented Programming", "yes"]

    def display_questions(self) -> None:
        print("Coding Quiz Questions:")
        for q in self.questions:
            print(q)

    def collect_answers(self) -> List[str]:
        print("Collecting answers...")
        return ["Object-Oriented Programming", "yes"]  # simulated answers

    def score_answers(self, answers: List[str]) -> int:
        return sum(1 for a, c in zip(answers, self.correct_answers) if a.lower() == c.lower())

    def show_feedback(self, score: int) -> None:
        print(f"Coding Quiz Score: {score}/{len(self.questions)}")

#-----------------------------------------
# Example usage
#-----------------------------------------
if __name__ == "__main__":
    quizzes: List[Quiz] = [MathQuiz(), HistoryQuiz(), CodingQuiz()]
    for quiz in quizzes:
        print("\n--- Starting Quiz ---")
        quiz.take_quiz()
