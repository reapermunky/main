import 'package:flutter/material.dart';
import 'package:hive_flutter/hive_flutter.dart';
import 'models/packet_pal.dart';
import 'models/player.dart'; // If you have a Player model
import 'screens/home_screen.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Hive.initFlutter();
  Hive.registerAdapter(PacketPalAdapter());
  // Hive.registerAdapter(PlayerAdapter()); // If you have a Player model
  await Hive.openBox<PacketPal>('packetPals');
  // await Hive.openBox<Player>('players'); // If you have a Player model
  runApp(PacketPalsApp());
}

class PacketPalsApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Packet Pals',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        brightness: Brightness.dark,
        // Customize for futuristic theme
      ),
      home: HomeScreen(),
    );
  }
}
