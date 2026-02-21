void actualizarMediciones() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize > 0) {
    if (packetSize == sizeof(Payload)) { 
      LoRa.readBytes((uint8_t *)&p, sizeof(p)); 

      // 1. Guardar la distancia cruda
      distancia = (int)p.dist; 
      
      // 2. IMPORTANTE: Calcular altura y porcentaje para el LCD
      // Usamos las variables que configuraste en WiFiManager (tank_h y sensor_m)
      int h_total = atoi(tank_h);   // Altura total del tanque
      int margen_m = atoi(sensor_m); // Margen del sensor al agua
      
      // Altura real del agua: AlturaTotal - (LecturaSensor - MargenMinimo)
      altura_agua = h_total - (distancia - margen_m);
      
      // Calcular porcentaje
      if (h_total > 0) {
        porcentaje = (altura_agua * 100) / h_total;
      }

      // Limitar valores para que no den negativos o más de 100% en el LCD
      if (porcentaje < 0) porcentaje = 0;
      if (porcentaje > 100) porcentaje = 100;
      if (altura_agua < 0) altura_agua = 0;

      p_in_empaquetado = p.p_in;
      
      Serial.print(F("[LoRa] OK -> Dist: "));
      Serial.print(distancia);
      Serial.print(F(" | Altura: "));
      Serial.println(altura_agua);
      actualizarPantallaInmediato();
    } else {
      while (LoRa.available()) LoRa.read();
    }
  }
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