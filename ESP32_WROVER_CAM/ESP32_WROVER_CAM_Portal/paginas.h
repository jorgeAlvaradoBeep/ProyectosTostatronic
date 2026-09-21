/*
 * ============================================================
 *  PAGINAS WEB DEL SKETCH ESP32_WROVER_CAM_Portal
 *  Tostatronic - www.tostatronic.com
 * ============================================================
 *
 *  PAGINA_PORTAL -> portal cautivo: escanea redes y pide la clave.
 *  PAGINA_CAMARA -> visor de video con el boton "Iniciar captura".
 *
 *  Todo va embebido (CSS y JS incluidos) a proposito: mientras el
 *  telefono esta pegado al portal cautivo NO hay internet, asi que
 *  no se puede cargar nada de un CDN.
 * ============================================================
 */

#pragma once

// ------------------ PORTAL CAUTIVO ------------------

const char PAGINA_PORTAL[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tostatronic CAM - WiFi</title>
<style>
:root{--fondo:#101519;--tarjeta:#1A222A;--borde:#2A353F;--texto:#F3F6F4;--tenue:#93A1AA;
--verde:#2DD4A7;--azul:#4BA8F5;--ambar:#F0B429;--naranja:#FF5C38}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--fondo);color:var(--texto);font:16px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
min-height:100vh;-webkit-tap-highlight-color:transparent}
header{display:flex;justify-content:space-between;align-items:center;padding:16px;
font-size:12px;letter-spacing:.14em;border-bottom:1px solid var(--borde)}
.marca{font-weight:800;color:var(--ambar)}
.modelo{color:var(--tenue)}
main{max-width:460px;margin:0 auto;padding:24px 16px 40px}
h1{font-size:26px;line-height:1.15;letter-spacing:-.01em}
.sub{color:var(--tenue);margin:8px 0 24px}
.fila{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}
h2{font-size:12px;letter-spacing:.12em;text-transform:uppercase;color:var(--tenue);font-weight:600}
button{font:inherit;border:0;border-radius:12px;cursor:pointer;color:var(--fondo);background:var(--verde);
font-weight:700;padding:14px 18px;width:100%}
button:disabled{opacity:.5}
button.sec{width:auto;background:transparent;color:var(--azul);padding:6px 4px;font-weight:600}
ul{list-style:none}
li{display:flex;align-items:center;gap:12px;background:var(--tarjeta);border:1px solid var(--borde);
border-radius:12px;padding:14px;margin-bottom:8px;cursor:pointer}
li:active{border-color:var(--verde)}
.ssid{flex:1;font-weight:600;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.candado{font-size:11px;color:var(--tenue);border:1px solid var(--borde);border-radius:6px;padding:2px 6px}
.barras{display:flex;align-items:flex-end;gap:2px;height:16px}
.barras i{width:4px;background:var(--borde);border-radius:1px}
.barras i.on{background:var(--verde)}
.barras i:nth-child(1){height:25%}.barras i:nth-child(2){height:50%}
.barras i:nth-child(3){height:75%}.barras i:nth-child(4){height:100%}
.nota{color:var(--tenue);text-align:center;padding:24px 0}
.tarjeta{background:var(--tarjeta);border:1px solid var(--borde);border-radius:16px;padding:20px}
.elegida{font-size:20px;font-weight:700;margin:2px 0 16px;word-break:break-all}
label{display:block;font-size:12px;letter-spacing:.12em;text-transform:uppercase;color:var(--tenue);margin-bottom:6px}
input{font:inherit;width:100%;padding:14px;border-radius:12px;border:1px solid var(--borde);
background:var(--fondo);color:var(--texto);margin-bottom:8px}
input:focus{outline:2px solid var(--verde);outline-offset:-1px}
.ver{display:flex;align-items:center;gap:8px;color:var(--tenue);font-size:14px;margin-bottom:16px}
.ver input{width:auto;margin:0}
.aviso{border-radius:12px;padding:12px 14px;margin-bottom:16px;font-size:14px;
background:rgba(255,92,56,.12);border:1px solid var(--naranja)}
.giro{width:44px;height:44px;border-radius:50%;border:4px solid var(--borde);border-top-color:var(--verde);
margin:8px auto 16px;animation:g 1s linear infinite}
@keyframes g{to{transform:rotate(360deg)}}
.centro{text-align:center}
.ok{font-size:44px;color:var(--verde);line-height:1}
.ip{display:block;font:700 22px ui-monospace,Menlo,Consolas,monospace;color:var(--verde);margin:12px 0 4px}
.chico{font-size:14px;color:var(--tenue)}
[hidden]{display:none!important}
</style>
</head>
<body>
<header><span class="marca">TOSTATRONIC</span><span class="modelo">ESP32-WROVER-CAM</span></header>
<main>
  <h1>Conecta tu c&aacute;mara al WiFi</h1>
  <p class="sub">Elige tu red, escribe la clave y listo.</p>

  <div id="aviso" class="aviso" hidden></div>

  <section id="vistaRedes">
    <div class="fila"><h2>Redes disponibles</h2><button id="btnBuscar" class="sec">Buscar de nuevo</button></div>
    <ul id="lista"></ul>
    <p id="nota" class="nota">Buscando redes&hellip;</p>
  </section>

  <section id="vistaClave" class="tarjeta" hidden>
    <h2>Red elegida</h2>
    <p id="elegida" class="elegida"></p>
    <div id="bloqueClave">
      <label for="clave">Clave del WiFi</label>
      <input id="clave" type="password" autocomplete="off" autocapitalize="off" spellcheck="false" maxlength="63">
      <div class="ver"><input id="verClave" type="checkbox"><span>Mostrar clave</span></div>
    </div>
    <button id="btnConectar">Conectar</button>
    <p class="centro"><button id="btnVolver" class="sec">Elegir otra red</button></p>
  </section>

  <section id="vistaEspera" class="tarjeta centro" hidden>
    <div class="giro"></div>
    <p>Conectando a <b id="esperaSsid"></b>&hellip;</p>
    <p class="chico">Puede tardar hasta 20 segundos.</p>
  </section>

  <section id="vistaLista" class="tarjeta centro" hidden>
    <p class="ok">&#10003;</p>
    <p><b>&iexcl;C&aacute;mara conectada!</b></p>
    <span id="ip" class="ip"></span>
    <p class="chico">Con&eacute;ctate a tu WiFi y abre esa direcci&oacute;n en el navegador.
    Tambi&eacute;n funciona <b id="mdns"></b></p>
  </section>
</main>
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

function pintar(redes){
  // Un mismo SSID puede venir repetido (varios AP / repetidores):
  // se deja solo el de mejor senal.
  const mejor={};
  redes.forEach(r=>{if(r.ssid&&(!mejor[r.ssid]||r.rssi>mejor[r.ssid].rssi))mejor[r.ssid]=r;});
  const orden=Object.values(mejor).sort((a,b)=>b.rssi-a.rssi);
  const ul=$('lista');ul.innerHTML='';
  orden.forEach(r=>{
    const li=document.createElement('li'), s=document.createElement('span');
    s.className='ssid';s.textContent=r.ssid;li.appendChild(s);
    if(r.segura){const c=document.createElement('span');c.className='candado';c.textContent='clave';li.appendChild(c);}
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

function estado(){
  // Al enlazarse, el ESP32 puede mover su AP al canal del router y el
  // telefono se suelta un instante: por eso los errores solo reintentan.
  fetch('/estado').then(r=>r.json()).then(j=>{
    if(j.estado==='conectado'){
      $('ip').textContent='http://'+j.ip;
      $('mdns').textContent='http://'+j.mdns+'.local';
      ver('vistaLista');
    }else if(j.estado==='fallo'){
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

// Si el arranque fallo con la red guardada, se avisa de entrada.
fetch('/estado').then(r=>r.json()).then(j=>{
  if(j.estado==='fallo')aviso('No se pudo conectar a "'+j.ssid+'". Elige la red otra vez.');
  else if(j.estado==='conectando'){$('esperaSsid').textContent=j.ssid;ver('vistaEspera');estado();}
}).catch(()=>{});
buscar(false);
</script>
</body>
</html>
)HTML";

// ------------------ VISOR DE LA CAMARA ------------------

const char PAGINA_CAMARA[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Tostatronic CAM</title>
<style>
:root{--fondo:#101519;--tarjeta:#1A222A;--borde:#2A353F;--texto:#F3F6F4;--tenue:#93A1AA;
--verde:#2DD4A7;--azul:#4BA8F5;--ambar:#F0B429;--naranja:#FF5C38}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--fondo);color:var(--texto);font:16px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;
min-height:100vh;-webkit-tap-highlight-color:transparent}
header{display:flex;justify-content:space-between;align-items:center;padding:16px;
font-size:12px;letter-spacing:.14em;border-bottom:1px solid var(--borde)}
.marca{font-weight:800;color:var(--ambar)}
.modelo{color:var(--tenue)}
main{max-width:720px;margin:0 auto;padding:16px 16px 40px}
.visor{position:relative;aspect-ratio:4/3;background:#000;border:1px solid var(--borde);
border-radius:16px;overflow:hidden;display:flex;align-items:center;justify-content:center}
.visor img{width:100%;height:100%;object-fit:contain;display:block}
.reposo{text-align:center;color:var(--tenue);padding:24px}
.reposo .lente{width:72px;height:72px;border-radius:50%;border:3px solid var(--borde);margin:0 auto 14px;
display:flex;align-items:center;justify-content:center}
.reposo .lente::after{content:"";width:28px;height:28px;border-radius:50%;background:var(--borde)}
.vivo{position:absolute;top:12px;left:12px;display:flex;align-items:center;gap:6px;background:rgba(16,21,25,.75);
border-radius:999px;padding:4px 10px;font-size:11px;font-weight:700;letter-spacing:.12em}
.vivo::before{content:"";width:8px;height:8px;border-radius:50%;background:var(--naranja);animation:p 1.2s ease-in-out infinite}
@keyframes p{50%{opacity:.25}}
.fpsTag{position:absolute;top:12px;right:12px;background:rgba(16,21,25,.75);border-radius:999px;padding:4px 10px;
font:700 11px ui-monospace,Menlo,Consolas,monospace;color:var(--verde)}
.acciones{display:grid;grid-template-columns:2fr 1fr;gap:10px;margin:14px 0}
button{font:inherit;border:0;border-radius:14px;cursor:pointer;font-weight:700;padding:16px 12px;
color:var(--fondo);background:var(--verde)}
button.detener{background:var(--naranja);color:#fff}
button.sec{background:var(--tarjeta);color:var(--texto);border:1px solid var(--borde)}
button:disabled{opacity:.45}
h2{font-size:12px;letter-spacing:.12em;text-transform:uppercase;color:var(--tenue);font-weight:600;margin:18px 0 8px}
.chips{display:flex;flex-wrap:wrap;gap:8px}
.chips button{padding:8px 12px;border-radius:999px;font-size:13px;font-weight:600;background:var(--tarjeta);
color:var(--tenue);border:1px solid var(--borde)}
.chips button.on{background:var(--azul);border-color:var(--azul);color:var(--fondo)}
.datos{display:grid;grid-template-columns:repeat(2,1fr);gap:8px}
@media(min-width:560px){.datos{grid-template-columns:repeat(4,1fr)}}
.dato{background:var(--tarjeta);border:1px solid var(--borde);border-radius:12px;padding:12px}
.dato span{display:block;font-size:11px;letter-spacing:.1em;text-transform:uppercase;color:var(--tenue)}
.dato b{font:700 18px ui-monospace,Menlo,Consolas,monospace}
.aviso{border-radius:12px;padding:12px 14px;margin-top:12px;font-size:14px;
background:rgba(255,92,56,.12);border:1px solid var(--naranja)}
footer{text-align:center;margin-top:28px;font-size:13px;color:var(--tenue)}
footer button{background:transparent;color:var(--tenue);font-weight:500;text-decoration:underline;padding:8px}
[hidden]{display:none!important}
</style>
</head>
<body>
<header><span class="marca">TOSTATRONIC</span><span class="modelo">ESP32-WROVER-CAM</span></header>
<main>
  <div class="visor">
    <div id="reposo" class="reposo"><div class="lente"></div>Pulsa <b>Iniciar captura</b> para ver la c&aacute;mara</div>
    <img id="video" alt="Video de la camara" hidden>
    <span id="vivo" class="vivo" hidden>EN VIVO</span>
    <span id="fpsTag" class="fpsTag" hidden></span>
  </div>

  <div class="acciones">
    <button id="btnCaptura">Iniciar captura</button>
    <button id="btnFoto" class="sec">Tomar foto</button>
  </div>
  <div id="aviso" class="aviso" hidden></div>

  <h2>Resoluci&oacute;n</h2>
  <div id="chips" class="chips"></div>

  <h2>Estado</h2>
  <div class="datos">
    <div class="dato"><span>Cuadros/s</span><b id="dFps">&ndash;</b></div>
    <div class="dato"><span>Imagen</span><b id="dRes">&ndash;</b></div>
    <div class="dato"><span>Se&ntilde;al WiFi</span><b id="dRssi">&ndash;</b></div>
    <div class="dato"><span>PSRAM libre</span><b id="dPsram">&ndash;</b></div>
  </div>

  <footer>
    <button id="btnOlvidar">Olvidar esta red WiFi</button>
    <p>tostatronic.com</p>
  </footer>
</main>
<script>
const $=id=>document.getElementById(id);
const img=$('video'), btn=$('btnCaptura');
let enVivo=false, armado=false, nombreAp='Tostatronic-CAM';

function aviso(t){$('aviso').textContent=t||'';$('aviso').hidden=!t;}

function pintarBoton(){
  btn.textContent=enVivo?'Detener captura':'Iniciar captura';
  btn.className=enVivo?'detener':'';
  img.hidden=!enVivo;$('reposo').hidden=enVivo;
  $('vivo').hidden=!enVivo;$('fpsTag').hidden=!enVivo;
}

function iniciar(){
  aviso('');
  // El video sale por el puerto 81: asi el stream no bloquea
  // los botones ni el estado, que se atienden en el puerto 80.
  img.src='http://'+location.hostname+':81/stream?t='+Date.now();
  enVivo=true;pintarBoton();
}

function detener(){
  enVivo=false;pintarBoton();
  // Quitar el src no siempre corta un MJPEG: window.stop() si.
  window.stop();
  img.removeAttribute('src');
}

btn.onclick=()=>enVivo?detener():iniciar();
img.onerror=()=>{if(enVivo){detener();aviso('Se perdio el video. Pulsa Iniciar captura de nuevo.');}};

$('btnFoto').onclick=()=>{
  const a=document.createElement('a');
  a.href='/foto?t='+Date.now();
  a.download='tostacam-'+Date.now()+'.jpg';
  document.body.appendChild(a);a.click();a.remove();
};

function pintarChips(lista,activa){
  const c=$('chips');c.innerHTML='';
  lista.forEach(r=>{
    const b=document.createElement('button');
    b.textContent=r.nombre+' '+r.ancho+'×'+r.alto;
    if(r.clave===activa)b.className='on';
    b.onclick=()=>fetch('/resolucion?val='+r.clave).then(refrescar);
    c.appendChild(b);
  });
}

function refrescar(){
  return fetch('/estado').then(r=>r.json()).then(j=>{
    $('dFps').textContent=j.transmitiendo?j.fps.toFixed(1):'–';
    $('fpsTag').textContent=j.fps.toFixed(1)+' FPS';
    $('dRes').textContent=j.ancho+'×'+j.alto;
    $('dRssi').textContent=j.rssi+' dBm';
    $('dPsram').textContent=j.psram_total?(j.psram_libre/1048576).toFixed(2)+' MB':'sin PSRAM';
    nombreAp=j.ap;
    pintarChips(j.resoluciones,j.res);
  }).catch(()=>{});
}

$('btnOlvidar').onclick=e=>{
  if(!armado){armado=true;e.target.textContent='¿Seguro? Toca otra vez para borrar el WiFi';
    setTimeout(()=>{armado=false;e.target.textContent='Olvidar esta red WiFi';},4000);return;}
  fetch('/olvidar',{method:'POST'}).then(()=>{
    if(enVivo)detener();
    aviso('WiFi borrado. La camara se reinicia y abre la red "'+nombreAp+'" para configurarla de nuevo.');
  });
};

pintarBoton();
refrescar();
setInterval(refrescar,1000);
</script>
</body>
</html>
)HTML";
