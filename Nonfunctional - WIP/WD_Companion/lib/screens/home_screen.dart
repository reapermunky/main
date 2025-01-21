import 'package:flutter/material.dart';
import '../services/ble_service.dart';
import '../services/mock_data_service.dart';
import '../services/packet_pal_service.dart';
import '../models/packet_pal.dart';
import 'export_screen.dart';

class HomeScreen extends StatefulWidget {
  @override
  _HomeScreenState createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  BLEService bleService = BLEService();
  PacketPalService packetPalService = PacketPalService();
  MockDataService mockDataService = MockDataService();
  bool isConnected = false;

  @override
  void initState() {
    super.initState();
    connectToESP32();
  }

  void connectToESP32() async {
    try {
      await bleService.connect();
      setState(() {
        isConnected = true;
      });

      // Listen to BLE data stream and add Packet Pals
      bleService.dataStream?.listen((data) {
        packetPalService.addPacketPals(data['networks']);
        setState(() {});
      });
    } catch (e) {
      setState(() {
        isConnected = false;
      });
      print('Failed to connect to ESP32: $e');
      // Load mock data
      Map<String, dynamic> mockData = await mockDataService.loadMockData();
      packetPalService.addPacketPals(mockData['networks']);
      setState(() {});
    }
  }

  @override
  void dispose() {
    bleService.disconnect();
    bleService.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    List<PacketPal> packetPals = packetPalService.getAllPacketPals();

    return Scaffold(
      appBar: AppBar(title: Text('Packet Pals Home')),
      body: Column(
        children: [
          Expanded(
            child: isConnected
                ? packetPals.isNotEmpty
                    ? ListView.builder(
                        itemCount: packetPals.length,
                        itemBuilder: (context, index) {
                          final pal = packetPals[index];
                          return ListTile(
                            title: Text(pal.ssid),
                            subtitle: Text('Signal: ${pal.signalStrength} dBm'),
                          );
                        },
                      )
                    : Center(child: Text('No networks detected'))
                : Center(child: Text('Disconnected. Showing mock data')),
          ),
          Padding(
            padding: const EdgeInsets.all(16.0),
            child: ElevatedButton(
              onPressed: () {
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => ExportScreen()),
                );
              },
              child: Text('Export & Share Data'),
            ),
          ),
        ],
      ),
    );
  }
}
