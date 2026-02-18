<script setup>
import { ref, onMounted, computed } from 'vue'
import axios from 'axios'

// --- 1. CONFIGURACIÓN DE LA API ---
// Ajusta el puerto (54625) si tu API cambia de puerto al reiniciar
const API_BASE = 'http://45.234.117.236:54625/api/Sensores';

// --- 2. ESTADO DE LA APLICACIÓN ---
const porcentaje = ref(0)
const altura = ref(0)
const lcdEncendido = ref(true) // Estado visual del botón
const cargando = ref(false)    // Para bloquear botón mientras envía
const errorApi = ref('')       // Mensajes de error en rojo

// Guardamos el estado de los relays para reenviarlos al cambiar el LCD
const relays = ref({
  relay1ON: false,
  relay2ON: false,
  relay3ON: false,
  relay4ON: false
})

// --- 3. COLOR DEL AGUA (Visual) ---
const colorAgua = computed(() => {
  return porcentaje.value < 20 ? '#e74c3c' : '#3498db'; // Rojo si es < 20%
});

// --- 4. FUNCIÓN GET: OBTENER DATOS DEL TANQUE ---
const obtenerDatos = async () => {
  try {
    // Llamamos al endpoint de ESTADO
    const respuesta = await axios.get(`${API_BASE}/estado`);
    const data = respuesta.data;

    // A. Procesamos datos del Sensor (Matemática del tanque)
    if (data.sensores) {
      const s = data.sensores;
      // Altura Agua = Fondo (tank_h) - Distancia medida
      // Ejemplo: 300 - 100 = 200cm de agua
      let alturaCalculada = s.tank_h - s.distancia;
      if (alturaCalculada < 0) alturaCalculada = 0; // Por si el sensor mide mal
      altura.value = alturaCalculada;

      // Altura Útil = Fondo (tank_h) - Margen superior (sensor_m)
      // Ejemplo: 300 - 30 = 270cm útiles
      const alturaUtil = s.tank_h - s.sensor_m;

      // Porcentaje = (Altura Agua / Altura Útil) * 100
      let calculo = (alturaCalculada / alturaUtil) * 100;
      
      // Limites 0-100%
      if (calculo > 100) calculo = 100;
      if (calculo < 0) calculo = 0;
      
      porcentaje.value = Math.round(calculo);
    }

    // B. Procesamos Comandos (LCD y Relays)
    if (data.comandos) {
      const cmd = data.comandos;
      lcdEncendido.value = cmd.lcd; // true o false
      
      // Guardamos relays para no perder su estado al hacer POST
      relays.value = {
        relay1ON: cmd.relay1ON,
        relay2ON: cmd.relay2ON,
        relay3ON: cmd.relay3ON,
        relay4ON: cmd.relay4ON
      };
    }
    errorApi.value = ''; // Limpiar errores si tuvo éxito

  } catch (error) {
    console.error("Error obteniendo datos:", error);
    errorApi.value = 'Sin conexión con AguaSimbolar API';
  }
};

// --- 5. FUNCIÓN POST: CAMBIAR LCD ---
const alternarLCD = async () => {
  cargando.value = true;
  errorApi.value = '';
  
  const nuevoEstado = !lcdEncendido.value; // Invertimos estado actual

  // Payload exacto para tu API
  const payload = {
    relay1ON: relays.value.relay1ON,
    relay2ON: relays.value.relay2ON,
    relay3ON: relays.value.relay3ON,
    relay4ON: relays.value.relay4ON,
    lcd: nuevoEstado
  };

  try {
    await axios.post(`${API_BASE}/comandos`, payload);
    
    // Si no da error, asumimos que cambió y actualizamos la UI
    lcdEncendido.value = nuevoEstado;
    
  } catch (error) {
    console.error("Error enviando comando:", error);
    errorApi.value = 'Error al cambiar el LCD';
  } finally {
    cargando.value = false;
  }
};

// --- 6. AL INICIAR LA APP ---
onMounted(() => {
  obtenerDatos(); // Primera carga
  
  // Actualización automática cada 5 segundos
  setInterval(obtenerDatos, 5000);
});
</script>

<template>
  <div class="contenedor">
    <h1 class="titulo">💧 AguaSimbolar</h1>
    
    <div v-if="errorApi" class="error">{{ errorApi }}</div>

    <div class="tanque-container">
      <div class="tanque-cuerpo">
        <div class="agua" :style="{ height: porcentaje + '%', backgroundColor: colorAgua }">
          <div class="ola" :style="{ backgroundImage: `linear-gradient(45deg, ${colorAgua} 25%, transparent 25%, transparent 50%, ${colorAgua} 50%, ${colorAgua} 75%, transparent 75%, transparent)` }"></div>
        </div>
        <div class="texto-porcentaje">{{ porcentaje }}%</div>
      </div>
      <p class="texto-altura">Nivel de agua: {{ altura }} cm</p>
    </div>

    <div class="controles">
      <button 
        @click="alternarLCD" 
        :class="['btn-lcd', lcdEncendido ? 'encendido' : 'apagado']"
        :disabled="cargando"
      >
        <span v-if="!cargando">
          {{ lcdEncendido ? 'Apagar Display' : 'Encender Display' }}
        </span>
        <span v-else>Enviando...</span>
      </button>
    </div>

    </div>
</template>

<style scoped>
/* ESTILOS GENERALES */
.contenedor {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  text-align: center;
  max-width: 500px;
  margin: 30px auto;
  padding: 20px;
  background-color: #2c3e50; /* Gris Oscuro */
  border-radius: 15px;
  color: white;
  box-shadow: 0 10px 25px rgba(0,0,0,0.5);
}

.titulo { margin-bottom: 20px; font-weight: 300; }

.error {
  background-color: #e74c3c;
  color: white;
  padding: 10px;
  border-radius: 5px;
  margin-bottom: 15px;
  font-size: 0.9em;
}

/* ESTILOS DEL TANQUE */
.tanque-container { margin-bottom: 30px; }

.tanque-cuerpo {
  position: relative;
  width: 180px;
  height: 240px;
  margin: 0 auto;
  background-color: #ecf0f1; /* Fondo vacío */
  border: 4px solid #95a5a6;
  border-radius: 15px;
  overflow: hidden;
  box-shadow: inset 0 0 15px rgba(0,0,0,0.1);
}

.agua {
  position: absolute;
  bottom: 0;
  left: 0;
  width: 100%;
  transition: height 1s ease-in-out, background-color 0.5s;
  display: flex;
  align-items: flex-start;
}

.ola {
  width: 200%;
  height: 15px;
  background-size: 30px 30px;
  opacity: 0.5;
  animation: moverOla 4s linear infinite;
  margin-top: -10px;
}

@keyframes moverOla {
  0% { transform: translateX(0); }
  100% { transform: translateX(-50%); }
}

.texto-porcentaje {
  position: absolute;
  top: 50%; left: 50%;
  transform: translate(-50%, -50%);
  font-size: 2.5em;
  font-weight: bold;
  color: rgba(0,0,0,0.6);
  z-index: 10;
}

.texto-altura { margin-top: 10px; color: #bdc3c7; }

/* BOTONES */
.controles { display: flex; justify-content: center; gap: 10px; }

.btn-lcd {
  padding: 15px 25px;
  border: none;
  border-radius: 50px;
  cursor: pointer;
  font-size: 1.1em;
  font-weight: bold;
  width: 100%;
  transition: transform 0.2s, background-color 0.3s;
}

.btn-lcd:active { transform: scale(0.98); }

.btn-lcd.encendido { background-color: #27ae60; color: white; } /* Verde */
.btn-lcd.apagado { background-color: #7f8c8d; color: white; }   /* Gris */

.btn-lcd:disabled { opacity: 0.6; cursor: not-allowed; }

.debug { margin-top: 20px; font-size: 0.7em; color: #7f8c8d; }
</style>