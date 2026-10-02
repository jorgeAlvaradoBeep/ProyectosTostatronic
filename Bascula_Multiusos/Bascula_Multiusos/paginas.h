/*
 * ============================================================
 *  PAGINAS WEB DE LA BASCULA MULTIUSOS
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  PAGINA_ESTILO     -> hoja de estilos comun (/estilo.css)
 *  PAGINA_PORTAL     -> portal cautivo: lista las redes y pide la clave
 *  PAGINA_BASCULA    -> el peso en vivo, el contador de piezas y los
 *                       ajustes (conexion, recalibrar, firmware)
 *  PAGINA_ACTUALIZAR -> recibe un firmware nuevo por WiFi
 *
 *  Todo va embebido en el firmware y nada se pide a internet (ni
 *  fuentes, ni librerias): mientras el telefono esta en el portal
 *  NO hay internet, y la bascula debe servir igual en una red sin
 *  salida.
 * ============================================================
 */

#pragma once
#include <Arduino.h>

// ------------------ ESTILOS ------------------
// Misma paleta que la pantalla de la bascula (pantalla.h).

const char PAGINA_ESTILO[] PROGMEM = R"CSS(
:root{--fondo:#101519;--tarjeta:#1A222A;--borde:#2A353F;--texto:#F3F6F4;--tenue:#93A1AA;
--verde:#2DD4A7;--azul:#4BA8F5;--ambar:#F0B429;--naranja:#FF5C38}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--fondo);color:var(--texto);font:16px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
min-height:100vh;-webkit-tap-highlight-color:transparent}
header{display:flex;justify-content:space-between;align-items:center;padding:16px;
font-size:12px;letter-spacing:.14em;border-bottom:1px solid var(--borde)}
.marca{font-weight:800;color:var(--azul)}
.modelo{color:var(--tenue)}
main{max-width:460px;margin:0 auto;padding:24px 16px 32px}
h1{font-size:26px;line-height:1.15;letter-spacing:-.01em}
.sub{color:var(--tenue);margin:8px 0 24px}
.fila{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}
h2{font-size:12px;letter-spacing:.12em;text-transform:uppercase;color:var(--tenue);font-weight:600}
button,.boton{display:block;font:inherit;border:0;border-radius:12px;cursor:pointer;color:var(--fondo);background:var(--azul);
font-weight:700;padding:14px 18px;width:100%;text-align:center;text-decoration:none}
button:disabled{opacity:.45;cursor:default}
button.sec{width:auto;display:inline-block;background:transparent;color:var(--azul);padding:6px 4px;font-weight:600}
button.borde,.boton.borde{background:var(--tarjeta);color:var(--texto);border:1px solid var(--borde)}
ul{list-style:none}
li{display:flex;align-items:center;gap:12px;background:var(--tarjeta);border:1px solid var(--borde);
border-radius:12px;padding:14px;margin-bottom:8px;cursor:pointer}
li:active{border-color:var(--azul)}
.ssid{flex:1;font-weight:600;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.marcaRed{font-size:11px;color:var(--tenue);border:1px solid var(--borde);border-radius:6px;padding:2px 6px;white-space:nowrap}
.marcaRed.b5{color:var(--azul);border-color:var(--azul)}
.barras{display:flex;align-items:flex-end;gap:2px;height:16px}
.barras i{width:4px;background:var(--borde);border-radius:1px}
.barras i.on{background:var(--verde)}
.barras i:nth-child(1){height:25%}.barras i:nth-child(2){height:50%}
.barras i:nth-child(3){height:75%}.barras i:nth-child(4){height:100%}
.nota{color:var(--tenue);text-align:center;padding:24px 0}
.tarjeta{background:var(--tarjeta);border:1px solid var(--borde);border-radius:16px;padding:20px;margin-bottom:12px}
.elegida{font-size:20px;font-weight:700;margin:2px 0 16px;word-break:break-all}
label{display:block;font-size:12px;letter-spacing:.12em;text-transform:uppercase;color:var(--tenue);margin-bottom:6px}
input{font:inherit;width:100%;padding:14px;border-radius:12px;border:1px solid var(--borde);
background:var(--fondo);color:var(--texto);margin-bottom:8px}
input:focus{outline:2px solid var(--azul);outline-offset:-1px}
.ver{display:flex;align-items:center;gap:8px;color:var(--tenue);font-size:14px;margin-bottom:16px}
.ver input{width:auto;margin:0}
.aviso{border-radius:12px;padding:12px 14px;margin-bottom:16px;font-size:14px;
background:rgba(255,92,56,.12);border:1px solid var(--naranja)}
.aviso.bien{background:rgba(45,212,167,.12);border-color:var(--verde)}
.giro{width:44px;height:44px;border-radius:50%;border:4px solid var(--borde);border-top-color:var(--azul);
margin:8px auto 16px;animation:g 1s linear infinite}
@keyframes g{to{transform:rotate(360deg)}}
.centro{text-align:center}
.ok{font-size:44px;color:var(--verde);line-height:1}
.ip{display:block;font:700 22px ui-monospace,Menlo,Consolas,monospace;color:var(--verde);margin:12px 0 4px;word-break:break-all}
.chico{font-size:14px;color:var(--tenue)}
.datos{display:grid;grid-template-columns:repeat(2,1fr);gap:8px;margin-bottom:12px}
.dato{background:var(--tarjeta);border:1px solid var(--borde);border-radius:12px;padding:12px;min-width:0}
.dato.ancho{grid-column:1/-1}
.dato span{display:block;font-size:11px;letter-spacing:.1em;text-transform:uppercase;color:var(--tenue)}
.dato b{display:block;font:700 17px ui-monospace,Menlo,Consolas,monospace;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.dato b.azul{color:var(--azul)}
.barra{height:10px;border-radius:5px;background:var(--borde);overflow:hidden;margin:14px 0 8px}
.barra i{display:block;height:100%;width:0;background:var(--verde);transition:width .2s}
ol{margin:8px 0 0 20px;color:var(--tenue);font-size:14px}
ol b{color:var(--texto)}
.acciones{display:grid;gap:8px;margin-top:16px}
footer{text-align:center;padding:8px 16px 32px;font-size:13px;color:var(--tenue)}
footer a{color:var(--azul);text-decoration:none}
.pestanas{display:flex;border-bottom:1px solid var(--borde)}
.pestanas button{flex:1;background:transparent;color:var(--tenue);border-radius:0;padding:12px 4px;font-weight:600;
border-bottom:2px solid transparent}
.pestanas button.on{color:var(--azul);border-bottom-color:var(--azul)}
.visor{background:var(--tarjeta);border:1px solid var(--borde);border-radius:20px;padding:20px 12px;text-align:center;margin-bottom:12px}
.rotulo{font-size:12px;letter-spacing:.14em;color:var(--azul);font-weight:700}
.numero{font-size:64px;font-weight:800;line-height:1.1;font-variant-numeric:tabular-nums}
.numero.texto{font-size:30px;line-height:2.35}
.unidadGrande{font-size:22px;color:var(--tenue)}
.estadoPeso{font-size:14px;color:var(--tenue);min-height:1.5em}
.bien{color:var(--verde)}.alerta{color:var(--ambar)}.mal{color:var(--naranja)}
.dos{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-bottom:12px}
.chips{display:flex;gap:8px;margin:8px 0 12px}
.chips button{flex:1;padding:10px;border-radius:999px;background:var(--tarjeta);color:var(--tenue);
border:1px solid var(--borde);font-weight:600}
.chips button.on{background:var(--azul);border-color:var(--azul);color:var(--fondo)}
.separado{margin:20px 0 10px}
.pregunta{margin:14px 0 8px}
.pieza{cursor:default;flex-wrap:wrap}
.pieza.activa{border-color:var(--azul)}
.pieza small{display:block;font-weight:400;color:var(--tenue)}
button.mini{width:auto;padding:6px 10px;font-size:13px;border-radius:8px}
[hidden]{display:none!important}
)CSS";

// ------------------ PORTAL CAUTIVO ------------------

const char PAGINA_PORTAL[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Báscula Tostatronic - WiFi</title>
<link rel="stylesheet" href="/estilo.css">
</head>
<body>
<header><span class="marca">TOSTATRONIC</span><span class="modelo">BÁSCULA MULTIUSOS</span></header>
<main>
  <h1>Conecta tu báscula al WiFi</h1>
  <p class="sub">Elige tu red, escribe la clave y listo. La báscula sigue pesando mientras tanto.</p>

  <div id="aviso" class="aviso" hidden></div>

  <section id="vistaRedes">
    <div class="fila"><h2>Redes disponibles</h2><button id="btnBuscar" class="sec">Buscar de nuevo</button></div>
    <ul id="lista"></ul>
    <p id="nota" class="nota">Buscando redes…</p>
  </section>

  <section id="vistaClave" class="tarjeta" hidden>
    <h2>Red elegida</h2>
    <p id="elegida" class="elegida"></p>
    <div id="bloqueClave">
      <label for="clave">Clave del WiFi</label>
      <input id="clave" type="password" autocomplete="off" autocapitalize="off" spellcheck="false" maxlength="64">
      <div class="ver"><input id="verClave" type="checkbox"><span>Mostrar clave</span></div>
    </div>
    <button id="btnConectar">Conectar</button>
    <p class="centro"><button id="btnVolver" class="sec">Elegir otra red</button></p>
  </section>

  <section id="vistaEspera" class="tarjeta centro" hidden>
    <div class="giro"></div>
    <p>Conectando a <b id="esperaSsid"></b>…</p>
    <p class="chico">Puede tardar hasta 20 segundos.</p>
  </section>

  <section id="vistaLista" class="tarjeta centro" hidden>
    <p class="ok">✓</p>
    <p><b>¡Báscula conectada!</b></p>
    <span id="ip" class="ip"></span>
    <p id="enlace" class="chico"></p>
    <p class="chico">Conéctate a tu WiFi y abre esa dirección en el navegador.
    También funciona <b id="mdns"></b>. La dirección queda en la pantalla de la báscula:
    MENÚ → Conexión.</p>
  </section>
</main>
<footer>
  <p><a href="/bascula">Usar la báscula sin conectarla</a> · <a href="/actualizar">Actualizar firmware</a></p>
  <p>Desarrollado por <b>Tostatronic</b> — Ing. Jorge Alvarado · tostatronic.com</p>
</footer>
<script>
const $=id=>document.getElementById(id);
const vistas=['vistaRedes','vistaClave','vistaEspera','vistaLista'];
let red=null, sondeoRedes=null, sondeoEstado=null;

function ver(v){vistas.forEach(x=>$(x).hidden=(x!==v));}
function aviso(t){$('aviso').textContent=t||'';$('aviso').hidden=!t;}
function barras(rssi){
  const n=rssi>-55?4:rssi>-67?3:rssi>-78?2:1, d=document.createElement('span');
  d.className='barras';
  for(let i=1;i<=4;i++){const b=document.createElement('i');if(i<=n)b.className='on';d.appendChild(b);}
  return d;
}
function marca(texto,clase){
  const c=document.createElement('span');c.className='marcaRed'+(clase?' '+clase:'');c.textContent=texto;return c;
}

function pintar(redes){
  // Un mismo SSID puede venir repetido (varios AP, repetidores, o la misma
  // red en 2.4 y 5 GHz): se deja solo el de mejor senal.
  const mejor={};
  redes.forEach(r=>{if(r.ssid&&(!mejor[r.ssid]||r.rssi>mejor[r.ssid].rssi))mejor[r.ssid]=r;});
  const orden=Object.values(mejor).sort((a,b)=>b.rssi-a.rssi);
  const ul=$('lista');ul.innerHTML='';
  orden.forEach(r=>{
    const li=document.createElement('li'), s=document.createElement('span');
    s.className='ssid';s.textContent=r.ssid;li.appendChild(s);
    if(r.banda5)li.appendChild(marca('5 GHz','b5'));
    if(r.segura)li.appendChild(marca('clave'));
    li.appendChild(barras(r.rssi));
    li.onclick=()=>elegir(r);
    ul.appendChild(li);
  });
  $('nota').hidden=orden.length>0;
  $('nota').textContent='No se encontraron redes. Intenta de nuevo.';
}

function buscar(nuevo){
  clearTimeout(sondeoRedes);
  $('btnBuscar').disabled=true;
  if(nuevo){$('lista').innerHTML='';$('nota').hidden=false;$('nota').textContent='Buscando redes…';}
  fetch('/redes'+(nuevo?'?nuevo=1':'')).then(r=>r.json()).then(j=>{
    if(j.estado==='listo'){pintar(j.redes);$('btnBuscar').disabled=false;}
    else sondeoRedes=setTimeout(()=>buscar(false),1200);
  }).catch(()=>{sondeoRedes=setTimeout(()=>buscar(false),2000);});
}

function elegir(r){
  red=r;aviso('');
  $('elegida').textContent=r.ssid;
  $('bloqueClave').hidden=!r.segura;
  $('clave').value='';
  ver('vistaClave');
  if(r.segura)$('clave').focus();
}

function conectar(){
  const clave=red.segura?$('clave').value:'';
  if(red.segura&&clave.length<8){aviso('La clave debe tener al menos 8 caracteres.');return;}
  aviso('');
  $('esperaSsid').textContent=red.ssid;
  ver('vistaEspera');
  const datos=new URLSearchParams({ssid:red.ssid,clave:clave});
  fetch('/conectar',{method:'POST',body:datos}).then(r=>{
    if(!r.ok)throw 0;
    sondeoEstado=setTimeout(estado,1500);
  }).catch(()=>{aviso('No se pudo enviar la clave. Intenta de nuevo.');ver('vistaClave');});
}

function conectada(j){
  $('ip').textContent='http://'+j.ip;
  $('enlace').textContent=j.estandar+(j.norma?' ('+j.norma+')':'')+' · '+j.banda+' · canal '+j.canal;
  $('mdns').textContent='http://'+j.mdns+'.local';
  ver('vistaLista');
}

function estado(){
  // Al enlazarse, la bascula puede mover su red al canal del router y el
  // telefono se suelta un instante: por eso los errores solo reintentan.
  fetch('/estado').then(r=>r.json()).then(j=>{
    if(j.red==='conectada')conectada(j);
    else if(j.fallo){
      aviso('No se pudo conectar a "'+j.ssid+'". Revisa la clave e intenta de nuevo.');
      ver(red?'vistaClave':'vistaRedes');
    }else sondeoEstado=setTimeout(estado,1000);
  }).catch(()=>{sondeoEstado=setTimeout(estado,1500);});
}

$('btnBuscar').onclick=()=>buscar(true);
$('btnConectar').onclick=conectar;
$('btnVolver').onclick=()=>{aviso('');ver('vistaRedes');};
$('verClave').onchange=e=>{$('clave').type=e.target.checked?'text':'password';};
$('clave').onkeydown=e=>{if(e.key==='Enter')conectar();};

// Si la red guardada no respondio, o ya hay un intento en curso, se avisa de entrada.
fetch('/estado').then(r=>r.json()).then(j=>{
  if(j.red==='conectada')conectada(j);
  else if(j.red==='conectando'){$('esperaSsid').textContent=j.ssid;ver('vistaEspera');estado();}
  else if(j.fallo)aviso('No se pudo conectar a "'+j.ssid+'". Elige la red otra vez.');
}).catch(()=>{});
buscar(false);
</script>
</body>
</html>
)HTML";

// ------------------ LA BASCULA (peso en vivo, piezas, ajustes) ------------------

const char PAGINA_BASCULA[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Báscula Tostatronic</title>
<link rel="stylesheet" href="/estilo.css">
</head>
<body>
<header><span class="marca">TOSTATRONIC</span><span id="placa" class="modelo">BÁSCULA MULTIUSOS</span></header>
<nav class="pestanas">
  <button data-v="vBascula" class="on">Báscula</button>
  <button data-v="vPiezas">Piezas</button>
  <button data-v="vAjustes">Ajustes</button>
</nav>
<main>
  <div id="aviso" class="aviso" hidden></div>

  <section id="vBascula">
    <div class="visor">
      <p id="titulo" class="rotulo">PESO</p>
      <p><span id="numero" class="numero">–</span> <span id="unidad" class="unidadGrande"></span></p>
      <p id="estado" class="estadoPeso">conectando…</p>
      <p id="taraTexto" class="chico"></p>
    </div>
    <div class="dos"><button id="btnTara">Tara</button><button id="btnCero" class="borde">Cero</button></div>
    <h2>Unidad</h2>
    <div id="unidades" class="chips"><button data-u="0">g</button><button data-u="1">kg</button><button data-u="2">oz</button></div>
  </section>

  <section id="vPiezas" hidden>
    <div id="pzContando" hidden>
      <div class="visor">
        <p class="rotulo">PIEZAS</p>
        <p><span id="pzNumero" class="numero">–</span></p>
        <p id="pzEstado" class="estadoPeso"></p>
        <p id="pzDetalle" class="chico"></p>
      </div>
      <div class="dos"><button id="btnPzTara">Tara</button><button id="btnPzOtra" class="borde">Otra muestra</button></div>
      <div id="pzGuardarBloque" class="tarjeta">
        <label for="pzNombre">Guardar esta pieza con nombre</label>
        <input id="pzNombre" maxlength="24" placeholder="Tornillo M3×10" autocomplete="off">
        <button id="btnPzGuardar">Guardar</button>
      </div>
    </div>

    <div id="pzMuestra" class="tarjeta">
      <h2>Muestra nueva</h2>
      <ol>
        <li>Pon el contenedor vacío y toca <b>Tara</b>.</li>
        <li>Coloca unas cuantas piezas.</li>
        <li>Escribe cuántas son y toca <b>Tomar muestra</b>.</li>
      </ol>
      <p class="pregunta">El peso actual es de <b id="pzPeso">–</b>. ¿Cuántas unidades son?</p>
      <input id="pzCantidad" type="number" inputmode="numeric" min="1" max="9999" value="10">
      <div class="dos"><button id="btnPzTara2" class="borde">Tara</button><button id="btnPzMuestra">Tomar muestra</button></div>
    </div>

    <h2 class="separado">Piezas guardadas</h2>
    <ul id="pzLista"></ul>
    <p id="pzVacia" class="nota">Aún no hay piezas guardadas.</p>
  </section>

  <section id="vAjustes" hidden>
    <h2>Conexión</h2>
    <div class="datos">
      <div class="dato ancho"><span>Red</span><b id="dRed">–</b></div>
      <div class="dato"><span>Estándar</span><b id="dEstandar" class="azul">–</b></div>
      <div class="dato"><span>Banda</span><b id="dBanda" class="azul">–</b></div>
      <div class="dato"><span>Señal</span><b id="dRssi">–</b></div>
      <div class="dato"><span>Canal</span><b id="dCanal">–</b></div>
      <div class="dato ancho"><span>Dirección</span><b id="dIp">–</b></div>
      <div class="dato ancho"><span>Firmware</span><b id="dVersion">–</b></div>
    </div>

    <div class="tarjeta">
      <h2 id="calPermiso">Recalibrar · permiso cerrado</h2>
      <ol>
        <li>En la báscula: <b>MENÚ → Conexión → OK → Calibrar web</b>.</li>
        <li>Retira todo del plato y toca <b>Tomar el cero</b>.</li>
        <li>Pon un peso conocido, escribe cuánto pesa y toca <b>Guardar</b>.</li>
      </ol>
      <div class="acciones">
        <button id="btnCalCero" class="borde" disabled>1 · Tomar el cero (plato vacío)</button>
        <div>
          <label for="calGramos">Peso conocido, en gramos</label>
          <input id="calGramos" type="number" inputmode="numeric" min="1" value="500">
        </div>
        <button id="btnCalCarga" disabled>2 · Guardar calibración</button>
      </div>
    </div>

    <div class="acciones">
      <a class="boton borde" href="/actualizar">Actualizar firmware</a>
      <button id="btnOlvidar" class="borde">Olvidar esta red WiFi</button>
    </div>
  </section>
</main>
<footer>
  <p>Desarrollado por <b>Tostatronic</b> — Ing. Jorge Alvarado · tostatronic.com</p>
</footer>
<script>
const $=id=>document.getElementById(id);
const UNIDADES=['g','kg','oz'];
const MOTIVOS={
  inestable:'El peso no se asentó. Deja quieto el plato e intenta de nuevo.',
  sin_celda:'La báscula no detecta la celda de carga.',
  sin_calibrar:'La báscula no está calibrada.',
  sin_peso:'No hay piezas en el plato.',
  dato_invalido:'Revisa el dato.',
  lleno:'Ya no caben más piezas guardadas: borra alguna.',
  sin_muestra:'Primero toma una muestra.',
  sin_permiso:'Abre el permiso en la báscula: MENÚ → Conexión → OK → Calibrar web.',
  sin_cero:'Primero toma el cero con el plato vacío.',
  peso_invalido:'Ese peso no sirve para calibrar esta celda.',
  poco_cambio:'La lectura casi no cambió: pon más peso.'
};
let e=null, ultimoMs=0, avisoT=null, piezas=[], perfilPintado=-2, armado=false, nombreAp='Tostatronic-Bascula', olvidada=false;

function aviso(t,bien){
  const a=$('aviso');a.textContent=t||'';a.hidden=!t;a.className='aviso'+(bien?' bien':'');
  clearTimeout(avisoT);if(t)avisoT=setTimeout(()=>aviso(''),7000);
}

// Mismo redondeo que la pantalla de la bascula: primero a la division de
// la celda, luego los decimales que tocan a cada unidad.
function fmt(g){
  const r=Math.round(g/e.div)*e.div, dg=e.div<1?1:0;
  let v=r,d=dg;
  if(e.unidad===1){v=r/1000;d=dg+3;}
  else if(e.unidad===2){v=r/28.349523125;d=e.div<0.3?3:2;}
  if(Math.abs(v)<0.5*Math.pow(10,-d))v=0;
  return v.toFixed(d);
}
function conUnidad(g){return fmt(g)+' '+UNIDADES[e.unidad];}

function orden(url,datos){
  return fetch(url,{method:'POST',body:new URLSearchParams(datos||{})}).then(r=>r.json());
}
// Tara, cero y capturas piden peso estable: se reintenta unos segundos,
// como hace la bascula cuando se presiona una tecla.
async function ordenEstable(url,datos){
  const fin=Date.now()+4000;
  for(;;){
    const j=await orden(url,datos);
    if(j.ok||j.motivo!=='inestable'||Date.now()>fin)return j;
    await new Promise(r=>setTimeout(r,300));
  }
}
function hacer(promesa,bien){
  return promesa.then(j=>{
    if(j.ok){if(bien)aviso(bien,true);}else aviso(MOTIVOS[j.motivo]||'La báscula no pudo hacerlo.');
    return j;
  }).catch(()=>{aviso('No hay respuesta de la báscula.');return {ok:false};});
}

function poner(id,texto,clase){const x=$(id);x.textContent=texto;if(clase!==undefined)x.className=clase;}

function pintar(){
  ultimoMs=Date.now();
  const listo=e.celda&&e.cal&&!e.sobre;

  // ---- bascula ----
  if(!e.celda){poner('numero','SIN CELDA','numero texto mal');poner('estado','revisa el HX711','estadoPeso');}
  else if(!e.cal){poner('numero','SIN CALIBRAR','numero texto alerta');poner('estado','calíbrala en Ajustes o en la báscula','estadoPeso');}
  else if(e.sobre){poner('numero','SOBRECARGA','numero texto mal');poner('estado','máximo '+e.cap/1000+' kg','estadoPeso');}
  else{poner('numero',fmt(e.neto),'numero');poner('estado',e.estable?'● ESTABLE':'midiendo…','estadoPeso'+(e.estable?' bien':''));}
  poner('unidad',listo?UNIDADES[e.unidad]:'');
  poner('titulo',e.conTara?'NETO':'PESO','rotulo'+(e.conTara?' alerta':''));
  poner('taraTexto',e.cal?(e.conTara?'Tara '+conUnidad(e.tara):'Máx '+e.cap/1000+' kg · d '+e.div+' g'):'');
  document.querySelectorAll('#unidades button').forEach(b=>b.className=(+b.dataset.u===e.unidad)?'on':'');

  // ---- piezas ----
  const pz=e.pz;
  $('pzContando').hidden=!pz.activo;
  $('pzMuestra').hidden=pz.activo;
  if(pz.activo){
    poner('pzNumero',listo?pz.n:'–','numero'+(pz.confiable?'':' alerta'));
    if(!listo)poner('pzEstado','la báscula no puede pesar ahora','estadoPeso mal');
    else if(!pz.confiable)poner('pzEstado','Una pieza pesa menos que la división ('+e.div+' g): el conteo no es confiable','estadoPeso mal');
    else if(e.estable&&pz.ambiguo){const a=Math.floor(pz.exacto);poner('pzEstado','Entre '+a+' y '+(a+1)+' piezas: revísalo a mano','estadoPeso alerta');}
    else poner('pzEstado',e.estable?'● ESTABLE':'contando…','estadoPeso'+(e.estable?' bien':''));
    poner('pzDetalle',(pz.nombre?pz.nombre+' · ':'')+pz.unit.toFixed(pz.unit<10?3:2)+' g por pieza · '+(e.cal?conUnidad(e.neto):''));
    $('pzGuardarBloque').hidden=pz.perfil>=0;
  }
  poner('pzPeso',listo?conUnidad(e.neto):'–');
  if(pz.perfil!==perfilPintado)pintarPiezas();

  // ---- recalibrar ----
  const c=e.calWeb, abierto=c.seg>0;
  poner('calPermiso',abierto?'Recalibrar · permiso abierto '+Math.floor(c.seg/60)+':'+String(c.seg%60).padStart(2,'0'):'Recalibrar · permiso cerrado');
  $('calPermiso').style.color=abierto?'var(--verde)':'';
  $('btnCalCero').disabled=!abierto;
  $('btnCalCarga').disabled=!(abierto&&c.paso===1);
  $('btnCalCero').textContent=(c.paso===1?'✓ Cero tomado · repetir':'1 · Tomar el cero (plato vacío)');
  $('calGramos').min=c.min;$('calGramos').max=e.cap;
}

// ---- piezas guardadas ----
function mini(texto,accion){
  const b=document.createElement('button');b.className='mini borde';b.textContent=texto;b.onclick=accion;return b;
}
function pintarPiezas(){
  perfilPintado=e?e.pz.perfil:-1;
  const ul=$('pzLista');ul.innerHTML='';
  piezas.forEach((p,i)=>{
    const li=document.createElement('li'), s=document.createElement('span'), d=document.createElement('small');
    li.className='pieza'+(i===perfilPintado?' activa':'');
    s.className='ssid';s.textContent=p.nombre;
    d.textContent=p.unit.toFixed(p.unit<10?3:2)+' g por pieza';s.appendChild(d);
    li.appendChild(s);
    li.appendChild(mini('Usar',()=>hacer(orden('/piezas/usar',{i:i}))));
    li.appendChild(mini('Nombre',()=>{
      const n=prompt('Nombre de la pieza',p.nombre);
      if(n&&n.trim())hacer(orden('/piezas/renombrar',{i:i,nombre:n.trim()})).then(cargarPiezas);
    }));
    li.appendChild(mini('Borrar',ev=>{
      const b=ev.target;
      if(b.dataset.seguro){hacer(orden('/piezas/borrar',{i:i})).then(cargarPiezas);return;}
      b.dataset.seguro='1';b.textContent='¿Seguro?';
      setTimeout(()=>{delete b.dataset.seguro;b.textContent='Borrar';},3000);
    }));
    ul.appendChild(li);
  });
  $('pzVacia').hidden=piezas.length>0;
}
function cargarPiezas(){
  return fetch('/piezas').then(r=>r.json()).then(j=>{piezas=j.lista;pintarPiezas();}).catch(()=>{});
}

// ---- conexion (solo mientras se ve la pestana de ajustes) ----
function refrescarConexion(){
  if($('vAjustes').hidden||olvidada)return;
  fetch('/estado').then(r=>r.json()).then(j=>{
    nombreAp=j.ap;
    $('placa').textContent=j.placa.toUpperCase();
    $('dVersion').textContent='v'+j.version+' · '+j.placa;
    if(j.red!=='conectada'){$('dRed').textContent='sin WiFi: usando la red de la báscula';return;}
    $('dRed').textContent=j.ssid;
    $('dEstandar').textContent=j.estandar+(j.norma&&j.norma!==j.estandar?' · '+j.norma:'');
    $('dBanda').textContent=j.banda;
    $('dRssi').textContent=j.rssi+' dBm';
    $('dCanal').textContent=j.canal;
    $('dIp').textContent=j.ip+' · '+j.mdns+'.local';
  }).catch(()=>{});
}

// ---- botones ----
document.querySelectorAll('.pestanas button').forEach(b=>b.onclick=()=>{
  document.querySelectorAll('.pestanas button').forEach(x=>x.className=(x===b)?'on':'');
  ['vBascula','vPiezas','vAjustes'].forEach(v=>$(v).hidden=(v!==b.dataset.v));
  aviso('');
  if(b.dataset.v==='vPiezas')cargarPiezas();
  if(b.dataset.v==='vAjustes')refrescarConexion();
});

const tara=()=>hacer(ordenEstable('/tara'),'Tara lista.');
$('btnTara').onclick=tara;$('btnPzTara').onclick=tara;$('btnPzTara2').onclick=tara;
$('btnCero').onclick=()=>hacer(ordenEstable('/cero'),'Cero tomado.');
document.querySelectorAll('#unidades button').forEach(b=>b.onclick=()=>hacer(orden('/unidad',{u:b.dataset.u})));

$('btnPzMuestra').onclick=()=>{
  const n=parseInt($('pzCantidad').value,10);
  if(!(n>=1&&n<=9999)){aviso('Escribe cuántas piezas hay en el plato (de 1 a 9999).');return;}
  hacer(ordenEstable('/piezas/muestra',{n:n})).then(j=>{
    if(!j.ok)return;
    if(j.calidad==='chica')aviso('Muestra tomada, pero es ligera: para contar con más exactitud usa '+j.sugeridas+' piezas o más.');
    else if(j.calidad==='ligera')aviso('Una pieza pesa menos de lo que la báscula distingue: el conteo no será confiable.');
    else aviso('Muestra tomada: '+j.unit.toFixed(j.unit<10?3:2)+' g por pieza.',true);
  });
};
$('btnPzOtra').onclick=()=>hacer(orden('/piezas/quitar'));
$('btnPzGuardar').onclick=()=>hacer(orden('/piezas/guardar',{nombre:$('pzNombre').value.trim()}),'Pieza guardada.').then(j=>{
  if(j.ok){$('pzNombre').value='';cargarPiezas();}
});

$('btnCalCero').onclick=()=>hacer(ordenEstable('/calibrar/cero'),'Cero tomado. Ahora pon el peso conocido.');
$('btnCalCarga').onclick=()=>{
  const g=parseInt($('calGramos').value,10);
  if(!(g>=e.calWeb.min&&g<=e.cap)){aviso('Usa un peso de '+e.calWeb.min+' a '+e.cap+' g.');return;}
  hacer(ordenEstable('/calibrar/carga',{gramos:g}),'Calibración guardada.');
};

$('btnOlvidar').onclick=ev=>{
  if(!armado){armado=true;ev.target.textContent='¿Seguro? Toca otra vez para borrar el WiFi';
    setTimeout(()=>{armado=false;ev.target.textContent='Olvidar esta red WiFi';},4000);return;}
  fetch('/olvidar',{method:'POST'}).then(()=>{
    olvidada=true;ev.target.disabled=true;
    aviso('WiFi borrado. La báscula abre la red "'+nombreAp+'" para configurarla de nuevo. Sigue pesando igual.',true);
  });
};

// ---- peso en vivo ----
// La bascula deja la conexion abierta y escribe cuando hay algo nuevo. Si
// se corta, el navegador reconecta solo.
const fuente=new EventSource('/eventos');
fuente.onmessage=m=>{e=JSON.parse(m.data);pintar();};
setInterval(()=>{
  if(Date.now()-ultimoMs>3500)poner('estado','sin conexión con la báscula…','estadoPeso mal');
  refrescarConexion();
},3000);
cargarPiezas();
</script>
</body>
</html>
)HTML";

// ------------------ ACTUALIZAR FIRMWARE ------------------

const char PAGINA_ACTUALIZAR[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Báscula Tostatronic - Firmware</title>
<link rel="stylesheet" href="/estilo.css">
</head>
<body>
<header><span class="marca">TOSTATRONIC</span><span id="placa" class="modelo">BÁSCULA MULTIUSOS</span></header>
<main>
  <h1>Actualizar firmware</h1>
  <p class="sub">Instalado: <b id="version">–</b></p>

  <div id="aviso" class="aviso" hidden></div>

  <section id="sinLugar" class="tarjeta" hidden>
    <h2>Esta placa no puede actualizarse por WiFi</h2>
    <p class="chico">Se cargó con un esquema de particiones sin lugar para un segundo firmware.
    Cárgala una vez por USB con <b>Partition Scheme: Minimal SPIFFS (1.9MB APP with OTA)</b>.
    La calibración no se pierde.</p>
  </section>

  <section id="pasos" class="tarjeta">
    <h2 id="permiso">Permiso: cerrado</h2>
    <ol>
      <li>En la báscula: <b>MENÚ → Conexión → OK → Actualizar</b>.</li>
      <li>Elige aquí el archivo <b>.ino.bin</b> compilado para <b id="placa2">tu placa</b>.</li>
      <li>No apagues la báscula hasta que se reinicie sola.</li>
    </ol>
  </section>

  <section id="carga" class="tarjeta">
    <label for="archivo">Archivo de firmware (.bin)</label>
    <input id="archivo" type="file" accept=".bin,application/octet-stream">
    <button id="btnSubir" disabled>Actualizar</button>
    <div id="avance" hidden>
      <div class="barra"><i id="barra"></i></div>
      <p id="textoAvance" class="chico centro"></p>
    </div>
  </section>

  <p class="centro"><a class="boton borde" href="/">Volver</a></p>
</main>
<footer>
  <p>Desarrollado por <b>Tostatronic</b> — Ing. Jorge Alvarado · tostatronic.com</p>
</footer>
<script>
const $=id=>document.getElementById(id);
const TOPE=1966080;   // lo que cabe en una particion de programa
let abierta=false, subiendo=false, esperandoReinicio=false, versionAntes='';

function aviso(t,bien){const a=$('aviso');a.textContent=t||'';a.hidden=!t;a.className='aviso'+(bien?' bien':'');}
function reloj(s){return Math.floor(s/60)+':'+String(s%60).padStart(2,'0');}
function boton(){$('btnSubir').disabled=subiendo||!abierta||!$('archivo').files.length;}

function refrescar(){
  if(subiendo)return;
  fetch('/estado').then(r=>r.json()).then(j=>{
    $('placa').textContent=j.placa.toUpperCase();
    $('placa2').textContent=j.placa;
    $('version').textContent='v'+j.version+' · '+j.placa;
    if(esperandoReinicio){
      if(j.ota==='lista')return;          // todavia no se reinicia
      esperandoReinicio=false;
      aviso('Listo: la báscula ya corre la versión '+j.version+(j.version===versionAntes?' (la misma de antes)':'')+'.',true);
    }
    $('sinLugar').hidden=j.otaLugar;
    $('pasos').hidden=$('carga').hidden=!j.otaLugar;
    abierta=j.ota==='abierta'||j.ota==='error';
    $('permiso').textContent=abierta?'Permiso: abierto · '+reloj(j.otaSegundos):'Permiso: cerrado';
    $('permiso').style.color=abierta?'var(--verde)':'';
    boton();
  }).catch(()=>{});
}

function subir(){
  const archivo=$('archivo').files[0];
  if(!archivo)return;
  if(!/\.bin$/i.test(archivo.name)){aviso('Elige el archivo .bin que exporta el Arduino IDE.');return;}
  // El IDE exporta varios .bin; solo el del programa sirve aqui.
  if(/merged|bootloader|partitions/i.test(archivo.name)){aviso('Ese no: elige el que termina en .ino.bin (sin "merged", "bootloader" ni "partitions").');return;}
  if(archivo.size>TOPE){aviso('El archivo es más grande que el espacio de programa de la báscula.');return;}
  aviso('');
  subiendo=true;boton();
  versionAntes=$('version').textContent.split(' ')[0].slice(1);
  $('avance').hidden=false;

  const x=new XMLHttpRequest(), datos=new FormData();
  datos.append('firmware',archivo,archivo.name);
  x.upload.onprogress=e=>{
    if(!e.lengthComputable)return;
    const p=Math.round(e.loaded*100/e.total);
    $('barra').style.width=p+'%';
    $('textoAvance').textContent=p<100?'Enviando… '+p+' %':'Verificando…';
  };
  x.onload=()=>{
    subiendo=false;
    if(x.status===200){
      esperandoReinicio=true;
      $('textoAvance').textContent='Firmware recibido.';
      aviso('Firmware recibido y verificado. La báscula se reinicia sola…',true);
    }else{
      $('avance').hidden=true;$('barra').style.width='0';
      aviso(x.responseText||'La báscula rechazó el archivo.');
    }
    boton();
  };
  x.onerror=()=>{
    subiendo=false;$('avance').hidden=true;$('barra').style.width='0';
    aviso('Se cortó la conexión con la báscula. Intenta de nuevo.');boton();
  };
  x.open('POST','/actualizar');
  x.send(datos);
}

$('archivo').onchange=boton;
$('btnSubir').onclick=subir;
refrescar();
setInterval(refrescar,1500);
</script>
</body>
</html>
)HTML";
