// lib/models/player.dart

import 'package:hive/hive.dart';

part 'player.g.dart';

@HiveType(typeId: 1)
class Player extends HiveObject {
  @HiveField(0)
  String username;

  @HiveField(1)
  int level;

  @HiveField(2)
  int experience;

  Player({
    required this.username,
    this.level = 1,
    this.experience = 0,
  });

  void gainExperience(int xp) {
    experience += xp;
    if (experience >= level * 100) {
      level += 1;
      experience -= level * 100;
      // Trigger level-up rewards or Packet Pal evolution
    }
    save();
  }
}
