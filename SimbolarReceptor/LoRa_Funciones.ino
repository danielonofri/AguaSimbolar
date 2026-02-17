// void actualizarMediciones() {
//   int lecturaCruda = obtenerLecturaDistancia();
//   if (lecturaCruda != -1) {
//     distancia = aplicarFiltroDelta(lecturaCruda);
//     if (distancia > 0) {
//       altura_agua = atoi(tank_h) - distancia;
//       porcentaje = map(distancia, atoi(tank_h), atoi(sensor_m), 0, 100);
//       porcentaje = constrain(porcentaje, 0, 100);
//     }
//   }
// }

void actualizarMediciones() {
  int packetSize = LoRa.parsePacket();
  //Serial.printf("Escuchando LoRa");
  if (packetSize > 0) {
    if (packetSize == sizeof(Paquete)) {
      Paquete p;
      LoRa.readBytes((uint8_t *)&p, sizeof(p));
      
      // Mapeo simple
      sw_remotos[1] = (p.s1 > 0.5);
      sw_remotos[2] = (p.s2 > 0.5);
      sw_remotos[3] = (p.s3 > 0.5);
      sw_remotos[4] = (p.s4 > 0.5);

      distancia = (int)p.distancia;
      if (distancia > 0) {
        altura_agua = atoi(tank_h) - distancia - atoi(sensor_m);
        porcentaje = map(distancia, atoi(tank_h), atoi(sensor_m), 0, 100);
        porcentaje = constrain(porcentaje, 0, 100);
      }
      Serial.printf("[LoRa] OK! Dist: %d cm\n", distancia);
    } 
    else {
      // Ignorar basura y limpiar el módulo inmediatamente
      while (LoRa.available()) LoRa.read();
      // Solo imprimimos si realmente queremos debuguear, sino, silencio.
      // Serial.printf("[LoRa] Basura ignorada: %d\n", packetSize);
    }
  }
}

void actualizarMedicionesVerbose() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print(F("[LoRa] Paquete recibido. Tamaño: "));
    if (packetSize == sizeof(Paquete)) {  // Verificamos que el tamaño sea el correcto
      Paquete p;
      LoRa.readBytes((uint8_t *)&p, sizeof(p));  // Deserialización directa

      // Asignar estados de relés (0 = apagado, 1 = encendido)
      sw_remotos[1] = (p.s1 > 0.5);
      sw_remotos[2] = (p.s2 > 0.5);
      sw_remotos[3] = (p.s3 > 0.5);
      sw_remotos[4] = (p.s4 > 0.5);

      int lecturaCruda = (int)p.distancia;
      if (lecturaCruda > 0) {
        distancia = aplicarFiltroDelta(lecturaCruda);
        if (distancia > 0) {
          altura_agua = atoi(tank_h) - distancia;
          porcentaje = map(distancia, atoi(tank_h), atoi(sensor_m), 0, 100);
          porcentaje = constrain(porcentaje, 0, 100);
        }
      }
      Serial.print(F("[LoRa] Datos: Dist="));
      Serial.println(p.distancia);
    } else {
      {
        Serial.print(F("[LoRa] Error: Tamaño esperado "));
        Serial.print(sizeof(Paquete));
        Serial.print(F(" pero llegó "));
        Serial.println(packetSize);
        // Limpiar búfer si el paquete tiene un tamaño erróneo
        while (LoRa.available()) LoRa.read();
      }
    }
  }
}

int obtenerLecturaDistancia() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String msg = "";
    while (LoRa.available()) msg += (char)LoRa.read();
    return msg.toInt();
  }
  return -1;
}

int aplicarFiltroDelta(int lecturaNueva) {
  if (distanciaAnterior == 0) {
    if (lecturasEstables == 0) {
      distanciaAnterior = lecturaNueva;
      lecturasEstables = 1;
      return lecturaNueva;
    }
    if (abs(lecturaNueva - distanciaAnterior) < delta_max) {
      lecturasEstables++;
    } else {
      distanciaAnterior = lecturaNueva;
      lecturasEstables = 1;
    }
    if (lecturasEstables < LECTURAS_PARA_ARRANQUE) return 0;
  }
  if (abs(lecturaNueva - distanciaAnterior) > delta_max) return distanciaAnterior;
  distanciaAnterior = lecturaNueva;
  return lecturaNueva;
}