import 'package:hive/hive.dart';

part 'packet_pal.g.dart';

@HiveType(typeId: 0)
class PacketPal extends HiveObject {
  @HiveField(0)
  String id; // Could be MAC address

  @HiveField(1)
  String ssid;

  @HiveField(2)
  String mac;

  @HiveField(3)
  int signalStrength;

  @HiveField(4)
  int level;

  PacketPal({
    required this.id,
    required this.ssid,
    required this.mac,
    required this.signalStrength,
    this.level = 1,
  });

  factory PacketPal.fromJson(Map<String, dynamic> json) {
    return PacketPal(
      id: json['mac'],
      ssid: json['ssid'],
      mac: json['mac'],
      signalStrength: json['signal_strength'],
      level: 1,
    );
  }

  Map<String, dynamic> toJson() => {
        'id': id,
        'ssid': ssid,
        'mac': mac,
        'signal_strength': signalStrength,
        'level': level,
      };
}
