<script setup>
import { ref, onMounted, computed } from 'vue'
import axios from 'axios'

// --- 1. CONFIGURACIÓN Y ESTADO ---
const API_BASE = 'https://simbolar-api-1bc5.onrender.com/api/Sensores';
const solapaActiva = ref('monitoreo'); // 'monitoreo', 'tecnico', 'info'

const distanciaReal = ref(0);
const sensor_m_val = ref(30);
const porcentaje = ref(0);
const altura = ref(0);
const lcdEncendido = ref(true);
const cargando = ref(false);
const errorApi = ref('');
const rawData = ref('');
const datosCompletos = ref(null);

const relays = ref({
  relay1ON: false,
  relay2ON: false,
  relay3ON: false,
  relay4ON: false
});

// --- 2. LÓGICA COMPUTADA ---
const colorAgua = computed(() => {
  const distanciaAlSensor = 300 - altura.value;
  if (altura.value > 0 && distanciaAlSensor <= 30) {
    return '#f1c40f'; // AMARILLO (Alerta)
  }
  return porcentaje.value < 20 ? '#e74c3c' : '#3498db'; // ROJO o AZUL
});

const bitsDevices = computed(() => {
  const num = datosCompletos.value?.Sensores?.p_in || 0;
  // Representación de un byte (8 bits)
  return num.toString(2).padStart(8, '0').split('').map(bit => bit === '1');
});

// --- 3. MÉTODOS ---
const obtenerDatos = async () => {
  try {
    const respuesta = await axios.get(`${API_BASE}/estado`);
    const raiz = respuesta.data.devolver;

    if (raiz) {
      datosCompletos.value = raiz;
      rawData.value = JSON.stringify(raiz, null, 2);

      if (raiz.Sensores) {
        altura.value = raiz.Sensores.altura_agua || 0;
        porcentaje.value = raiz.Sensores.porcentaje || 0;
        distanciaReal.value = raiz.Sensores.distancia || 0;
        sensor_m_val.value = raiz.Sensores.sensor_m || 30;
      }

      if (raiz.Comandos) {
        relays.value = {
          relay1ON: raiz.Comandos.relay1ON,
          relay2ON: raiz.Comandos.relay2ON,
          relay3ON: raiz.Comandos.relay3ON,
          relay4ON: raiz.Comandos.relay4ON
        };
        lcdEncendido.value = raiz.Comandos.lcd;
      }
    }
  } catch (e) {
    console.error("Error capturando datos:", e);
  }
};

const alternarLCD = async () => {
  cargando.value = true;
  errorApi.value = '';
  const nuevoEstado = !lcdEncendido.value;

  const payload = {
    ...relays.value,
    lcd: nuevoEstado
  };

  try {
    await axios.post(`${API_BASE}/comandos`, payload);
    lcdEncendido.value = nuevoEstado;
  } catch (error) {
    console.error("Fallo POST:", error.response?.data || error.message);
    errorApi.value = 'Error al cambiar el LCD';
  } finally {
    cargando.value = false;
  }
};

const copiarAlPortapapeles = () => {
  navigator.clipboard.writeText(rawData.value)
    .then(() => alert("JSON copiado al portapapeles"))
    .catch(err => console.error('Error al copiar:', err));
};

onMounted(() => {
  obtenerDatos();
  setInterval(obtenerDatos, 3000);
});
</script>

<template>
  <div class="contenedor">
    <h1 class="titulo">📡 AguaSimbolar</h1>

    <div class="tabs-header">
      <button @click="solapaActiva = 'monitoreo'" :class="{ activa: solapaActiva === 'monitoreo' }">💧 Tanque</button>
      <button @click="solapaActiva = 'tecnico'" :class="{ activa: solapaActiva === 'tecnico' }">⚙️ Técnico</button>
      <button @click="solapaActiva = 'info'" :class="{ activa: solapaActiva === 'info' }">📝 Info</button>
    </div>

    <div v-if="errorApi" class="error-banner">{{ errorApi }}</div>

    <div v-if="solapaActiva === 'monitoreo'" class="tab-content">
      <div class="tanque-cuerpo">
        <div class="agua" :style="{ height: porcentaje + '%', backgroundColor: colorAgua }">
          <div class="ola"></div>
        </div>
        <div class="texto-porcentaje">
          <div v-if="altura > 0 && (300 - altura) <= 30">
            <div style="font-size: 1.5rem;">⚠️</div>
            <div style="font-size: 0.5rem; font-weight: bold;">AVISO</div>
          </div>
          <div v-else>{{ porcentaje }}%</div>
        </div>
      </div>
      <p class="texto-altura">Nivel de agua: {{ altura }} cm</p>

      <div class="controles">
        <button @click="alternarLCD" :class="['btn-lcd', lcdEncendido ? 'encendido' : 'apagado']" :disabled="cargando">
          {{ cargando ? 'Enviando...' : (lcdEncendido ? 'Apagar Display' : 'Encender Display') }}
        </button>
      </div>
    </div>

    <div v-if="solapaActiva === 'tecnico'" class="tab-content">
      <div class="detalles-container" v-if="datosCompletos">
        <h3 class="subtitulo">📊 Datos Técnicos</h3>
        <div class="grid-detalles">
          <div class="card-detalle">
            <ul>
              <li><span>Distancia:</span> <strong>{{ distanciaReal }} cm</strong></li>
              <li>
                <span>Devices (p_in):</span>
                <div class="pulsos-display">
                  <strong class="valor-decimal">{{ datosCompletos.Sensores.p_in }}</strong>
                  <div class="bits-container">
                    <div v-for="(on, idx) in bitsDevices" :key="idx" :class="['bit-cuadrito', on ? 'on' : 'off']"></div>
                  </div>
                </div>
              </li>
              <li><span>Tanque (h):</span> <strong>{{ datosCompletos.Sensores.tank_h }} cm</strong></li>
            </ul>
          </div>

          <div class="card-detalle">
            <h4>Botones Físicos</h4>
            <div class="botones-status">
              <div v-for="(val, key) in datosCompletos.Sensores.botones" :key="key"
                :class="['indicador-boton', val ? 'activo' : 'inactivo']">
                {{ key.toUpperCase() }}
              </div>
            </div>
          </div>
        </div>
      </div>

      <div class="caja-negra-container">
        <div class="caja-negra-header">
          <span>DEBUG JSON DATA</span>
          <button @click="copiarAlPortapapeles" class="btn-copiar">📋 Copiar</button>
        </div>
        <pre class="caja-negra-content">{{ rawData }}</pre>
      </div>
    </div>

    <div v-if="solapaActiva === 'info'" class="tab-content info-md">
      <h2>AguaSimbolar IoT</h2>
      <p>Sistema de monitoreo de nivel de agua en tiempo real.</p>
      <hr>
      <h3>Hardware</h3>
      <ul>
        <li>NodeMCU / ESP8266</li>
        <li>Sensor Ultrasónico HC-SR04</li>
        <li>Relays para control de periféricos</li>
      </ul>
      <blockquote>
        Integridad referencial manejada por código. SaaS con licencia mensual/anual.
      </blockquote>
    </div>
  </div>
</template>

<style scoped>
.contenedor {
  display: flex;
  flex-direction: column;
  align-items: center;
  min-height: 100vh;
  padding: 40px 20px;
  background-color: #121212;
  color: white;
}

.titulo {
  font-weight: 300;
  margin-bottom: 20px;
}

/* Tabs Navigation */
.tabs-header {
  display: flex;
  width: 100%;
  max-width: 400px;
  background: #222;
  padding: 5px;
  border-radius: 12px;
  margin-bottom: 25px;
}

.tabs-header button {
  flex: 1;
  background: transparent;
  border: none;
  color: #777;
  padding: 10px;
  border-radius: 8px;
  cursor: pointer;
  font-weight: bold;
}

.tabs-header button.activa {
  background: #3498db;
  color: white;
}

.tab-content {
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  animation: fadeIn 0.3s ease;
}

/* Tanque */
.tanque-cuerpo {
  position: relative;
  width: 160px;
  height: 220px;
  background-color: #ecf0f1;
  border: 4px solid #95a5a6;
  border-radius: 15px;
  overflow: hidden;
}

.agua {
  position: absolute;
  bottom: 0;
  width: 100%;
  transition: height 1s ease-in-out, background-color 0.5s;
}

.texto-porcentaje {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  font-size: 2.5rem;
  font-weight: bold;
  color: rgba(0, 0, 0, 0.6);
  z-index: 10;
}

.texto-altura {
  margin-top: 10px;
  color: #bdc3c7;
}

/* Controles */
.controles {
  margin-top: 25px;
  width: 100%;
  max-width: 250px;
}

.btn-lcd {
  width: 100%;
  padding: 15px;
  border-radius: 50px;
  border: none;
  font-weight: bold;
  cursor: pointer;
}

.btn-lcd.encendido {
  background: #27ae60;
  color: white;
}

.btn-lcd.apagado {
  background: #7f8c8d;
  color: white;
}

/* Datos Técnicos & Bits */
.detalles-container {
  width: 100%;
  max-width: 400px;
  background: #1e1e1e;
  padding: 15px;
  border-radius: 12px;
  border: 1px solid #333;
}

.subtitulo {
  border-bottom: 1px solid #444;
  padding-bottom: 5px;
  margin-bottom: 15px;
}

.card-detalle li {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 10px;
}

.pulsos-display {
  display: flex;
  align-items: center;
  gap: 8px;
}

.bits-container {
  display: flex;
  gap: 1px;
  background: #000;
  padding: 2px;
  border: 1px solid #444;
}

.bit-cuadrito {
  width: 10px;
  height: 10px;
}

.bit-cuadrito.on {
  background: #2ecc71;
  box-shadow: 0 0 4px #2ecc71;
}

.bit-cuadrito.off {
  background: #333;
}

/* Botones Físicos */
.botones-status {
  display: flex;
  gap: 5px;
}

.indicador-boton {
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 0.7rem;
  font-weight: bold;
}

.indicador-boton.activo {
  background: #27ae60;
}

.indicador-boton.inactivo {
  background: #444;
  color: #777;
}

/* Caja Negra */
.caja-negra-container {
  margin-top: 20px;
  width: 100%;
  max-width: 400px;
  background: #000;
  border-radius: 8px;
  border: 1px solid #444;
  overflow: hidden;
}

.caja-negra-header {
  background: #222;
  padding: 5px 10px;
  display: flex;
  justify-content: space-between;
  font-size: 0.7rem;
}

.caja-negra-content {
  text-align: left;
  padding: 15px;
  font-family: monospace;
  font-size: 0.8rem;
  color: #9cdcfe;
  white-space: pre;
  overflow-x: auto;
}

.btn-copiar {
  background: #444;
  border: none;
  color: white;
  font-size: 0.6rem;
  border-radius: 3px;
  cursor: pointer;
}

/* Info Page */
.info-md {
  text-align: left;
  max-width: 400px;
  line-height: 1.6;
}

.info-md h2 {
  color: #3498db;
}

.info-md blockquote {
  border-left: 4px solid #3498db;
  padding-left: 10px;
  color: #888;
  font-style: italic;
}

@keyframes fadeIn {
  from {
    opacity: 0;
  }

  to {
    opacity: 1;
  }
}
</style>