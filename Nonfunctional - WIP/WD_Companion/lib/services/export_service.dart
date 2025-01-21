import 'dart:convert';
import 'dart:io';
import 'package:path_provider/path_provider.dart';
import '../models/packet_pal.dart';
import '../services/packet_pal_service.dart';

class ExportService {
  final PacketPalService packetPalService = PacketPalService();

  Future<String> exportPacketPalsToJson() async {
    // Retrieve Packet Pals data from your service
    List<PacketPal> packetPals = packetPalService.getAllPacketPals();

    // Convert Packet Pals to JSON
    List<Map<String, dynamic>> packetPalsJson = packetPals.map((pal) => pal.toJson()).toList();
    Map<String, dynamic> data = {'packetPals': packetPalsJson};
    String jsonString = jsonEncode(data);

    // Get the directory to save the file
    Directory directory;
    if (Platform.isAndroid) {
      directory = await getExternalStorageDirectory() as Directory;
      // On Android, navigate to a public directory (optional)
      String newPath = "";
      List<String> paths = directory.path.split("/");
      for (int x = 1; x < paths.length; x++) {
        String folder = paths[x];
        if (folder != "Android") {
          newPath += "/$folder";
        } else {
          break;
        }
      }
      newPath = "$newPath/PacketPals";
      directory = Directory(newPath);
    } else {
      directory = await getApplicationDocumentsDirectory();
    }

    // Create the directory if it doesn't exist
    if (!await directory.exists()) {
      await directory.create(recursive: true);
    }

    // Define the file path
    String filePath = '${directory.path}/packet_pals_export.json';

    // Write the JSON string to the file
    File file = File(filePath);
    await file.writeAsString(jsonString);

    return filePath; // Return the path to the saved file
  }
}
