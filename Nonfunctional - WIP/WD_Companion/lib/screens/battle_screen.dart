// lib/screens/battle_screen.dart

import 'package:flutter/material.dart';
import '../services/player_service.dart';
import '../services/packet_pal_service.dart';
import '../models/packet_pal.dart';
import '../data/questions.dart';

class BattleScreen extends StatefulWidget {
  @override
  _BattleScreenState createState() => _BattleScreenState();
}

class _BattleScreenState extends State<BattleScreen> {
  final PlayerService playerService = PlayerService();
  final PacketPalService packetPalService = PacketPalService();

  List<PacketPal> playerPals = [];
  List<PacketPal> opponentPals = []; // For multiplayer or AI
  int currentQuestionIndex = 0;
  bool isChallengeActive = false;

  @override
  void initState() {
    super.initState();
    playerPals = packetPalService.getAllPacketPals();
    // Initialize opponentPals
  }

  void startBattle() {
    // Implement battle logic
    // Optionally trigger a STEM challenge
    setState(() {
      isChallengeActive = true;
    });
  }

  void answerQuestion(int selectedOption) {
    if (selectedOption ==
        sampleQuestions[currentQuestionIndex].correctOption) {
      // Correct answer, grant experience
      playerService.addExperience(50);
      // Apply battle advantage
    } else {
      // Incorrect answer, apply disadvantage
    }

    setState(() {
      isChallengeActive = false;
      currentQuestionIndex =
          (currentQuestionIndex + 1) % sampleQuestions.length;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(title: Text('Battle')),
        body: isChallengeActive
            ? buildChallenge()
            : Center(
                child: ElevatedButton(
                  onPressed: startBattle,
                  child: Text('Start Battle'),
                ),
              ));
  }

  Widget buildChallenge() {
    Question currentQuestion = sampleQuestions[currentQuestionIndex];
    return Padding(
      padding: const EdgeInsets.all(16.0),
      child: Column(
        children: [
          Text(currentQuestion.question,
              style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold)),
          ...currentQuestion.options.asMap().entries.map((entry) {
            int idx = entry.key;
            String option = entry.value;
            return ListTile(
              title: Text(option),
              leading: Radio<int>(
                value: idx,
                groupValue: null,
                onChanged: (value) {
                  answerQuestion(idx);
                },
              ),
            );
          }).toList(),
        ],
      ),
    );
  }
}
