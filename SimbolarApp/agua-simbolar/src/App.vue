<script setup>
import { ref, onMounted, computed } from 'vue'
import axios from 'axios'

// --- 1. CONFIGURACIÓN DE LA API ---
const API_BASE = 'http://45.234.117.236:54625/api/Sensores';

// --- 2. ESTADO DE LA APLICACIÓN ---
const porcentaje = ref(0)
const altura = ref(0)
const lcdEncendido = ref(true) 
const cargando = ref(false)    
const errorApi = ref('')       

const relays = ref({
  relay1ON: false,
  relay2ON: false,
  relay3ON: false,
  relay4ON: false
})

// --- 3. COLOR DEL AGUA ---
const colorAgua = computed(() => {
  return porcentaje.value < 20 ? '#e74c3c' : '#3498db'; 
});

// --- 4. FUNCIÓN GET: LEER ESTRUCTURA "DEVOLVER" ---
const obtenerDatos = async () => {
  try {
    const respuesta = await axios.get(`${API_BASE}/estado`);
    
    // AQUÍ ESTÁ EL CAMBIO IMPORTANTE:
    // Entramos a respuesta.data -> devolver
    const raiz = respuesta.data.devolver; 

    if (raiz) {
      // A. DATOS DEL SENSOR
      if (raiz.sensores) {
        // Mapeamos "altura_agua" del JSON a nuestra variable "altura"
        altura.value = raiz.sensores.altura_agua || 0;
        porcentaje.value = raiz.sensores.porcentaje || 0;
      }

      // B. COMANDOS
      if (raiz.comandos) {
        lcdEncendido.value = raiz.comandos.lcd; 
        
        relays.value = {
          relay1ON: raiz.comandos.relay1ON,
          relay2ON: raiz.comandos.relay2ON,
          relay3ON: raiz.comandos.relay3ON,
          relay4ON: raiz.comandos.relay4ON
        };
      }
      errorApi.value = ''; 
    }

  } catch (error) {
    console.error("Error obteniendo datos:", error);
    // Solo mostramos error si no tenemos datos previos para no parpadear
    if (altura.value === 0) errorApi.value = 'Esperando conexión...';
  }
};

// --- 5. FUNCIÓN POST: ENVIAR COMANDO ---
const alternarLCD = async () => {
  cargando.value = true;
  errorApi.value = '';
  
  const nuevoEstado = !lcdEncendido.value; 

  // El payload sigue igual, enviamos estado de relays + lcd
  const payload = {
    relay1ON: relays.value.relay1ON,
    relay2ON: relays.value.relay2ON,
    relay3ON: relays.value.relay3ON,
    relay4ON: relays.value.relay4ON,
    lcd: nuevoEstado
  };

  try {
    await axios.post(`${API_BASE}/comandos`, payload);
    lcdEncendido.value = nuevoEstado; 
  } catch (error) {
    console.error("Error enviando comando:", error);
    errorApi.value = 'Error al cambiar el LCD';
  } finally {
    cargando.value = false;
  }
};

// --- 6. AL INICIAR ---
onMounted(() => {
  obtenerDatos(); 
  setInterval(obtenerDatos, 3000); 
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
/* ESTILOS (Mantenemos los mismos) */
.contenedor {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  text-align: center; max-width: 500px; margin: 30px auto; padding: 20px;
  background-color: #2c3e50; border-radius: 15px; color: white;
  box-shadow: 0 10px 25px rgba(0,0,0,0.5);
}
.titulo { margin-bottom: 20px; font-weight: 300; }
.error { background-color: #e74c3c; padding: 10px; border-radius: 5px; margin-bottom: 15px; }
.tanque-container { margin-bottom: 30px; }
.tanque-cuerpo {
  position: relative; width: 180px; height: 240px; margin: 0 auto;
  background-color: #ecf0f1; border: 4px solid #95a5a6; border-radius: 15px; overflow: hidden;
  box-shadow: inset 0 0 15px rgba(0,0,0,0.1);
}
.agua {
  position: absolute; bottom: 0; left: 0; width: 100%;
  transition: height 1s ease-in-out, background-color 0.5s; display: flex; align-items: flex-start;
}
.ola {
  width: 200%; height: 15px; background-size: 30px 30px; opacity: 0.5;
  animation: moverOla 4s linear infinite; margin-top: -10px;
}
@keyframes moverOla { 0% { transform: translateX(0); } 100% { transform: translateX(-50%); } }
.texto-porcentaje {
  position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);
  font-size: 2.5em; font-weight: bold; color: rgba(0,0,0,0.6); z-index: 10;
}
.texto-altura { margin-top: 10px; color: #bdc3c7; }
.controles { display: flex; justify-content: center; }
.btn-lcd {
  padding: 15px 25px; border: none; border-radius: 50px; cursor: pointer;
  font-size: 1.1em; font-weight: bold; width: 100%; transition: 0.3s;
}
.btn-lcd.encendido { background-color: #27ae60; color: white; }
.btn-lcd.apagado { background-color: #7f8c8d; color: white; }
.btn-lcd:disabled { opacity: 0.6; }
</style>