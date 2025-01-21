import 'package:hive/hive.dart';
import '../models/packet_pal.dart';

class PacketPalService {
  final Box<PacketPal> packetPalBox = Hive.box<PacketPal>('packetPals');

  void addPacketPals(List<dynamic> networks) {
    for (var network in networks) {
      PacketPal pal = PacketPal.fromJson(network);
      if (!packetPalBox.containsKey(pal.id)) {
        packetPalBox.put(pal.id, pal);
      }
    }
  }

  List<PacketPal> getAllPacketPals() {
    return packetPalBox.values.toList();
  }

  // Additional methods for evolving Packet Pals, etc.
}
