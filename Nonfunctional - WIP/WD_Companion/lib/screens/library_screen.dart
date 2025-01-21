// lib/screens/library_screen.dart

import 'package:flutter/material.dart';
import '../services/packet_pal_service.dart';
import '../models/packet_pal.dart';

class LibraryScreen extends StatelessWidget {
  final PacketPalService packetPalService = PacketPalService();

  @override
  Widget build(BuildContext context) {
    List<PacketPal> packetPals = packetPalService.getAllPacketPals();

    return Scaffold(
      appBar: AppBar(title: Text('Library')),
      body: packetPals.isEmpty
          ? Center(child: Text('No Packet Pals collected yet'))
          : GridView.builder(
              gridDelegate: SliverGridDelegateWithFixedCrossAxisCount(
                  crossAxisCount: 2),
              itemCount: packetPals.length,
              itemBuilder: (context, index) {
                final pal = packetPals[index];
                return Card(
                  child: Column(
                    mainAxisAlignment: MainAxisAlignment.center,
                    children: [
                      Text(pal.ssid,
                          style: TextStyle(
                              fontSize: 18, fontWeight: FontWeight.bold)),
                      Text('MAC: ${pal.mac}'),
                      Text('Signal: ${pal.signalStrength} dBm'),
                      Text('Level: ${pal.level}'),
                      // Add more stats or evolution status
                    ],
                  ),
                );
              },
            ),
    );
  }
}
