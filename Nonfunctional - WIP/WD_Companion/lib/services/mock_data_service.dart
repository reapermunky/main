import 'dart:convert';
import 'package:flutter/services.dart' show rootBundle;

class MockDataService {
  Future<Map<String, dynamic>> loadMockData() async {
    String jsonString = await rootBundle.loadString('assets/mock_data.json');
    return jsonDecode(jsonString);
  }
}
