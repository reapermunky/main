// lib/services/player_service.dart

import 'package:hive/hive.dart';
import '../models/player.dart';

class PlayerService {
  final Box<Player> playerBox = Hive.box<Player>('players');

  Player getPlayer() {
    if (playerBox.isEmpty) {
      Player newPlayer = Player(username: "Player1");
      playerBox.put('currentPlayer', newPlayer);
      return newPlayer;
    }
    return playerBox.get('currentPlayer')!;
  }

  void addExperience(int xp) {
    Player player = getPlayer();
    player.gainExperience(xp);
  }
}
