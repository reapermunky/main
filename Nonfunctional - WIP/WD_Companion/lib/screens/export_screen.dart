import 'package:flutter/material.dart';
import '../services/export_service.dart';
import '../services/native_share.dart'; // The new native share service

class ExportScreen extends StatefulWidget {
  @override
  _ExportScreenState createState() => _ExportScreenState();
}

class _ExportScreenState extends State<ExportScreen> {
  final ExportService exportService = ExportService();
  bool isExporting = false;
  String? exportPath;

  Future<void> exportData() async {
    setState(() {
      isExporting = true;
    });

    try {
      String path = await exportService.exportPacketPalsToJson();
      setState(() {
        exportPath = path;
      });
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Data exported to $path')),
      );
    } catch (e) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('Failed to export data: $e')),
      );
    } finally {
      setState(() {
        isExporting = false;
      });
    }
  }

  Future<void> shareData() async {
    if (exportPath != null) {
      await NativeShare.shareFile(exportPath!);
    } else {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(content: Text('No data to share. Please export first.')),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(title: Text('Export Data')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              ElevatedButton(
                onPressed: isExporting ? null : exportData,
                child: isExporting
                    ? CircularProgressIndicator()
                    : Text('Export to JSON'),
              ),
              SizedBox(height: 20),
              ElevatedButton(
                onPressed: exportPath != null ? shareData : null,
                child: Text('Share Exported Data'),
              ),
            ],
          ),
        ));
  }
}
