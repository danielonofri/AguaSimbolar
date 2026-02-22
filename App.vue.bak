<script setup>
import { ref, onMounted, computed } from 'vue'
import axios from 'axios'

// --- 1. CONFIGURACIÓN ---
const API_BASE = 'https://simbolar-api-1bc5.onrender.com/api/Sensores';
const distanciaReal = ref(0);
const sensor_m_val = ref(30);
const porcentaje = ref(0)
const altura = ref(0)
const lcdEncendido = ref(true)
const cargando = ref(false)
const errorApi = ref('')
const rawData = ref('');
const mostrarCajaNegra = ref(false); // Inicia colapsado
const datosCompletos = ref(null);

const bitsDevices = computed(() => {
  // Tomamos el valor de p_in que representa los dispositivos/estados
  const num = datosCompletos.value?.Sensores?.p_in || 0;
  // Representamos el byte completo (8 bits) para ver el estado de todos los canales
  return num.toString(2).padStart(8, '0').split('').map(bit => bit === '1');
});

const relays = ref({
  relay1ON: false,
  relay2ON: false,
  relay3ON: false,
  relay4ON: false
})

// --- 2. LÓGICA COMPUTADA ---
const colorAgua = computed(() => {
  const distanciaAlSensor = 300 - altura.value;
  if (altura.value > 0 && distanciaAlSensor <= 30) {
    return '#f1c40f'; // AMARILLO (Alerta cercanía sensor)
  }
  return porcentaje.value < 20 ? '#e74c3c' : '#3498db';
});

// --- 3. MÉTODOS ---
const obtenerDatos = async () => {
  try {
    const respuesta = await axios.get(`${API_BASE}/estado`);
    const raiz = respuesta.data.devolver;

    if (raiz) {
      datosCompletos.value = raiz;
      // Formateo con indentación de 2 espacios
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
    errorApi.value = 'Error de conexión con la API';
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
    .then(() => alert("JSON copiado"))
    .catch(err => console.error('Error al copiar', err));
};

onMounted(() => {
  obtenerDatos();
  setInterval(obtenerDatos, 3000);
});
</script>

<template>
  <div class="contenedor-principal">
    <h1 class="titulo">📡 Agua</h1>

    <div v-if="errorApi" class="error-banner">{{ errorApi }}</div>

    <div class="tanque-seccion">
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
    </div>

    <div class="controles">
      <button @click="alternarLCD" :class="['btn-lcd', lcdEncendido ? 'encendido' : 'apagado']" :disabled="cargando">
        {{ cargando ? 'Enviando...' : (lcdEncendido ? 'Apagar Display' : 'Encender Display') }}
      </button>

      <button @click="mostrarCajaNegra = !mostrarCajaNegra" class="btn-toggle-estado">
        {{ mostrarCajaNegra ? '▲ Ocultar estado' : '▼ Mostrar estado' }}
      </button>
    </div>

    <div v-if="mostrarCajaNegra && datosCompletos" class="seccion-detalle">

      <div class="detalles-container">
        <h3 class="subtitulo">📊 Datos Técnicos</h3>
        <div class="grid-detalles">
          <div class="card-detalle">
            <ul>
              <li><span>Distancia:</span> <strong>{{ datosCompletos.Sensores.distancia }} cm</strong></li>
              <li>
                <span>Pulsos (p_in):</span>
                <div class="pulsos-display">
                  <strong class="valor-decimal">{{ datosCompletos.Sensores.p_in }}</strong>
                  <div class="bits-container">
                    <div v-for="(on, idx) in bitsPulsos" :key="idx" :class="['bit-cuadrito', on ? 'on' : 'off']">
                    </div>
                  </div>
                </div>
              </li>
              <li><span>Tanque (h):</span> <strong>{{ datosCompletos.Sensores.tank_h }} cm</strong></li>
            </ul>
          </div>
          <div class="card-detalle">
            <h4>Configuración de Hardware</h4>
            <ul>
              <li><span>Distancia:</span> <strong>{{ datosCompletos.Sensores.distancia }} cm</strong></li>
              <li>
                <span>Devices (p_in):</span>
                <div class="pulsos-display">
                  <strong class="valor-decimal">{{ datosCompletos.Sensores.p_in }}</strong>
                  <div class="bits-container">
                    <div v-for="(on, idx) in bitsDevices" :key="idx" :class="['bit-cuadrito', on ? 'on' : 'off']"
                      :title="'Bit ' + (7 - idx)">
                    </div>
                  </div>
                </div>
              </li>
              <li><span>Tanque (h):</span> <strong>{{ datosCompletos.Sensores.tank_h }} cm</strong></li>
            </ul>
          </div>
        </div>
      </div>

      <div class="caja-negra-wrapper">
        <div class="caja-negra-header">
          <span>DEBUG JSON</span>
          <button @click="copiarAlPortapapeles" class="btn-copiar">📋 Copiar</button>
        </div>
        <pre class="caja-negra-content">{{ rawData }}</pre>
      </div>

    </div>
  </div>
</template>

<style scoped>
.pulsos-display {
  display: flex;
  align-items: center;
  gap: 10px;
}

.bits-container {
  display: flex;
  gap: 1px;
  /* Pegados como bits de un registro */
  background: #000;
  padding: 2px;
  border: 1px solid #444;
}

.bit-cuadrito {
  width: 10px;
  height: 10px;
  /* Cuadrados sin bordes redondeados */
}

.bit-cuadrito.on {
  background-color: #2ecc71;
  /* Verde activo */
  box-shadow: 0 0 4px rgba(46, 204, 113, 0.8);
}

.bit-cuadrito.off {
  background-color: #222;
  /* Negro/Gris muy oscuro apagado */
}

/* Aseguramos que la lista se alinee bien */
.card-detalle li {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 4px 0;
}

.contenedor-principal {
  display: flex;
  flex-direction: column;
  align-items: center;
  min-height: 100vh;
  padding: 40px 20px;
  background-color: #121212;
  color: white;
  text-align: center;
}

.titulo {
  font-weight: 300;
  margin-bottom: 25px;
}

.tanque-cuerpo {
  position: relative;
  width: 160px;
  height: 220px;
  margin: 0 auto;
  background-color: #2c3e50;
  border: 4px solid #7f8c8d;
  border-radius: 15px;
  overflow: hidden;
}

.agua {
  position: absolute;
  bottom: 0;
  width: 100%;
  transition: height 1s ease, background-color 0.5s;
}

.texto-porcentaje {
  position: absolute;
  top: 50%;
  left: 50%;
  transform: translate(-50%, -50%);
  font-size: 2rem;
  font-weight: bold;
  color: white;
  text-shadow: 1px 1px 4px rgba(0, 0, 0, 0.5);
}

.controles {
  display: flex;
  flex-direction: column;
  gap: 12px;
  width: 220px;
  margin-top: 20px;
}

.btn-lcd {
  padding: 12px;
  border-radius: 25px;
  border: none;
  font-weight: bold;
  cursor: pointer;
}

.btn-lcd.encendido {
  background: #27ae60;
  color: white;
}

.btn-lcd.apagado {
  background: #c0392b;
  color: white;
}

.btn-toggle-estado {
  background: transparent;
  border: 1px solid white;
  color: white;
  padding: 8px;
  border-radius: 8px;
  cursor: pointer;
  font-size: 0.9rem;
}

.seccion-detalle {
  width: 100%;
  max-width: 400px;
  margin-top: 25px;
  animation: slideDown 0.3s ease-out;
}

.detalles-container {
  background: #1e1e1e;
  padding: 15px;
  border-radius: 12px;
  margin-bottom: 15px;
  border: 1px solid #333;
}

.caja-negra-wrapper {
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
  /* Alineado a la izquierda */
  padding: 15px;
  font-family: 'Courier New', monospace;
  font-size: 0.75rem;
  color: #00ff00;
  margin: 0;
  white-space: pre;
  /* Mantiene indentación */
  overflow-x: auto;
}

.botones-status {
  display: flex;
  gap: 5px;
  justify-content: center;
}

.indicador-boton {
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 0.6rem;
}

.indicador-boton.activo {
  background: #27ae60;
}

.indicador-boton.inactivo {
  background: #444;
}

@keyframes slideDown {
  from {
    opacity: 0;
    transform: translateY(-10px);
  }

  to {
    opacity: 1;
    transform: translateY(0);
  }
}
</style>