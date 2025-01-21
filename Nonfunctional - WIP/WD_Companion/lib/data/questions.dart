// main.dart

class Question {
  final String question;
  final List<String> options;
  final int correctOption;

  Question({
    required this.question,
    required this.options,
    required this.correctOption,
  });

  void displayQuestion() {
    print(question);
    for (int i = 0; i < options.length; i++) {
      print('${i + 1}. ${options[i]}');
    }
  }

  bool isCorrect(int selectedOption) {
    return selectedOption == correctOption;
  }
}

void main() {
  List<Question> sampleQuestions = [
    Question(
      question: "Which encryption method is the strongest?",
      options: ["WEP", "WPA", "WPA2", "WPA3"],
      correctOption: 3,
    ),
    Question(
      question: "What does RSSI represent?",
      options: [
        "Receive Signal Strength Indicator",
        "Radio Signal Strength Index",
        "Remote Signal Strength Indicator",
        "Random Signal Strength Index"
      ],
      correctOption: 0,
    ),
  ];

  // Example usage:
  for (var i = 0; i < sampleQuestions.length; i++) {
    print('Question ${i + 1}:');
    sampleQuestions[i].displayQuestion();
    print('Correct Answer: ${sampleQuestions[i].options[sampleQuestions[i].correctOption]}\n');
  }
}
