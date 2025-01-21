// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'packet_pal.dart';

// **************************************************************************
// TypeAdapterGenerator
// **************************************************************************

class PacketPalAdapter extends TypeAdapter<PacketPal> {
  @override
  final int typeId = 0;

  @override
  PacketPal read(BinaryReader reader) {
    final numOfFields = reader.readByte();
    final fields = <int, dynamic>{
      for (int i = 0; i < numOfFields; i++) reader.readByte(): reader.read(),
    };
    return PacketPal(
      id: fields[0] as String,
      ssid: fields[1] as String,
      mac: fields[2] as String,
      signalStrength: fields[3] as int,
      level: fields[4] as int,
    );
  }

  @override
  void write(BinaryWriter writer, PacketPal obj) {
    writer
      ..writeByte(5)
      ..writeByte(0)
      ..write(obj.id)
      ..writeByte(1)
      ..write(obj.ssid)
      ..writeByte(2)
      ..write(obj.mac)
      ..writeByte(3)
      ..write(obj.signalStrength)
      ..writeByte(4)
      ..write(obj.level);
  }

  @override
  int get hashCode => typeId.hashCode;

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is PacketPalAdapter &&
          runtimeType == other.runtimeType &&
          typeId == other.typeId;
}
