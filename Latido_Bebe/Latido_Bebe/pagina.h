/*
 * ============================================================
 *  PAGINA WEB - se sirve desde el ESP32-C3 en http://192.168.4.1
 *  Desarrollado por Tostatronic - Ing. Jorge Alvarado
 * ============================================================
 *
 *  La conversion del video se hace en el navegador del celular o la compu:
 *  recorta el video al circulo de 240 x 240, saca los cuadros como JPEG,
 *  pasa el sonido a ADPCM de 16 kHz y arma el archivo .ltd que se manda al
 *  ESP32. No hace falta instalar nada.
 */

#pragma once
#include <Arduino.h>

static const char PAGINA[] PROGMEM = R"LATIDO(<!doctype html>
<html lang="es"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Latido</title>
<style>
:root{--f:#14101a;--c:#211a2b;--b:#33293f;--t:#f4eef8;--s:#a99bb8;--r:#ff4d6d}
*{box-sizing:border-box}
body{margin:0;background:var(--f);color:var(--t);font:16px/1.45 system-ui,-apple-system,Segoe UI,Roboto,sans-serif}
main{max-width:460px;margin:0 auto;padding:16px}
h1{margin:8px 0 2px;font-size:28px}h1 span{color:var(--r)}
h2{margin:0 0 12px;font-size:17px}
#est{margin:0 0 14px;color:var(--s);font-size:14px}
.card{background:var(--c);border:1px solid var(--b);border-radius:16px;padding:16px;margin-bottom:14px}
.vista{display:flex;justify-content:center;margin:14px 0 6px}
canvas{width:240px;height:240px;border-radius:50%;background:#000;border:3px solid var(--b);touch-action:none;cursor:grab}
label{display:block;margin:12px 0 0;font-size:14px;color:var(--s)}
label b{color:var(--t);float:right;font-weight:600}
input[type=range]{width:100%;margin:6px 0 0;accent-color:var(--r)}
select,input[type=file]{width:100%;margin-top:6px;padding:10px;border-radius:10px;border:1px solid var(--b);background:var(--f);color:var(--t);font-size:15px}
.fila{display:flex;align-items:center;gap:8px;color:var(--t)}
.fila input{width:20px;height:20px;accent-color:var(--r)}
button{width:100%;margin-top:14px;padding:13px;border:0;border-radius:12px;background:var(--r);color:#fff;font-size:16px;font-weight:600}
button.sec{background:transparent;border:1px solid var(--b);color:var(--t)}
button:disabled{opacity:.45}
.nota{font-size:13px;color:var(--s);margin:6px 0 0}
.barra{height:10px;border-radius:5px;background:var(--b);overflow:hidden;margin-top:14px}
.barra i{display:block;height:100%;width:0;background:var(--r);transition:width .2s}
#pt{font-size:14px;margin:8px 0 0}
.bien{color:#7be0a4}.mal{color:#ff8a8a}
#vid{position:fixed;left:0;top:0;width:2px;height:2px;opacity:0;pointer-events:none}
</style></head><body><main>
<h1>Latido <span>&#9829;</span></h1>
<p id="est">Conectando...</p>

<div class="card">
 <h2>Video del ultrasonido</h2>
 <input type="file" id="arch" accept="video/*,.ltd">
 <div id="editor" hidden>
  <div class="vista"><canvas id="cv" width="240" height="240"></canvas></div>
  <p class="nota">Arrastra la imagen para centrarla. Lo que queda dentro del c&iacute;rculo es lo que sale en la pantalla.</p>
  <label>Acercar <b id="tZoom">1.0x</b><input type="range" id="zoom" min="1" max="4" step="0.05" value="1"></label>
  <label>Empieza en <b id="tIni">0 s</b><input type="range" id="ini" min="0" max="0" step="0.1" value="0"></label>
  <label>Duraci&oacute;n <b id="tDur">0 s</b><input type="range" id="dur" min="1" max="60" step="0.5" value="20"></label>
  <label>Fluidez
   <select id="fps"><option value="10">10 cuadros por segundo (caben m&aacute;s segundos)</option>
   <option value="12" selected>12 cuadros por segundo</option>
   <option value="15">15 cuadros por segundo (m&aacute;s fluido)</option></select></label>
  <label class="fila"><input type="checkbox" id="conAudio" checked> Incluir el sonido del latido</label>
  <p class="nota" id="cabe"></p>
  <button class="sec" id="probar">&#9654; Probar el tramo con sonido</button>
  <button id="enviar">Convertir y enviar</button>
 </div>
 <div id="prog" hidden><div class="barra"><i id="pb"></i></div><p id="pt"></p></div>
</div>

<div class="card">
 <h2>Ajustes</h2>
 <label>Volumen <b id="tVol">-</b><input type="range" id="vol" min="0" max="100" value="70"></label>
 <label id="lBri">Brillo <b id="tBri">-</b><input type="range" id="bri" min="5" max="100" value="100"></label>
 <button class="sec" id="pausa">Pausar</button>
 <button class="sec" id="borrar">Borrar el video</button>
</div>
<p class="nota" style="text-align:center">Tostatronic &middot; <span id="fw"></span></p>
</main>
<video id="vid" playsinline muted preload="auto"></video>

<script>
'use strict';
const $=id=>document.getElementById(id), HZ=16000, LADO=240;
const v=$('vid'), cv=$('cv'), cx2=cv.getContext('2d');
const cvJ=document.createElement('canvas'); cvJ.width=cvJ.height=LADO; const cxJ=cvJ.getContext('2d');
let E={capacidad:2400000}, archivo=null, audBuf=null, zoom=1, cX=0, cY=0, ocupado=false, probando=false;

// ---------- estado del dispositivo ----------
async function estado(){
  try{
    E=await (await fetch('/estado')).json();
    let t=E.video.hay?('Video cargado: '+E.video.seg.toFixed(1)+' s, '+(E.video.bytes/1024).toFixed(0)+' KB'+(E.video.audio?', con sonido':', sin sonido')):'Todavía no hay video';
    if(E.bateria) t+=' · Batería '+(E.bateria/1000).toFixed(2)+' V';
    $('est').textContent=t; $('fw').textContent='Latido '+E.fw;
    if(!ocupado){ $('vol').value=E.volumen; $('tVol').textContent=E.volumen+' %'; $('bri').value=E.brillo; $('tBri').textContent=E.brillo+' %'; }
    $('lBri').hidden=!E.brilloOk; $('pausa').textContent=E.pausa?'Seguir':'Pausar';
    $('pausa').disabled=$('borrar').disabled=!E.video.hay;
    cabe();
  }catch(e){ $('est').textContent='Sin conexión con el dispositivo'; }
}
function ajuste(datos){ return fetch('/ajustes',{method:'POST',body:new URLSearchParams(datos)}).catch(()=>{}); }
let tAj=0;
function alVuelo(d){ clearTimeout(tAj); tAj=setTimeout(()=>ajuste(d),120); }
$('vol').oninput=e=>{ $('tVol').textContent=e.target.value+' %'; alVuelo({volumen:e.target.value}); };
$('vol').onchange=e=>ajuste({volumen:e.target.value,guardar:1});
$('bri').oninput=e=>{ $('tBri').textContent=e.target.value+' %'; alVuelo({brillo:e.target.value}); };
$('bri').onchange=e=>ajuste({brillo:e.target.value,guardar:1});
$('pausa').onclick=async()=>{ await ajuste({pausa:E.pausa?0:1}); estado(); };
$('borrar').onclick=async()=>{ if(!confirm('¿Borrar el video del dispositivo?'))return; await fetch('/borrar',{method:'POST'}); estado(); };

// ---------- vista previa y recorte ----------
function recorte(){
  const w=v.videoWidth,h=v.videoHeight,l=Math.min(w,h)/zoom;
  let sx=Math.max(0,Math.min(w-l,cX-l/2)), sy=Math.max(0,Math.min(h-l,cY-l/2));
  cX=sx+l/2; cY=sy+l/2; return [sx,sy,l];
}
function pinta(c){
  const [sx,sy,l]=recorte();
  c.fillStyle='#000'; c.fillRect(0,0,LADO,LADO);
  c.save(); c.beginPath(); c.arc(LADO/2,LADO/2,LADO/2,0,7); c.clip();
  try{ c.drawImage(v,sx,sy,l,l,0,0,LADO,LADO); }catch(e){}
  c.restore();
}
function ir(t){
  t=Math.max(0,Math.min(t,(v.duration||0)-0.04));
  if(Math.abs(v.currentTime-t)<0.0005&&v.readyState>=2) return Promise.resolve();
  return new Promise(ok=>{ let fin=false; const h=()=>{ if(fin)return; fin=true; v.removeEventListener('seeked',h); ok(); };
    v.addEventListener('seeked',h); v.currentTime=t; setTimeout(h,4000); });
}
function textos(){
  $('tZoom').textContent=zoom.toFixed(1)+'x'; $('tIni').textContent=(+$('ini').value).toFixed(1)+' s';
  $('tDur').textContent=(+$('dur').value).toFixed(1)+' s'; cabe();
}
function cabe(){
  if(!archivo||$('editor').hidden) return;
  // estimado grueso: un cuadro de ultrasonido anda en 5 KB con buena calidad
  const fps=+$('fps').value, seg=E.capacidad/(fps*5200+($('conAudio').checked?HZ/2:0));
  $('cabe').textContent='En la memoria caben unos '+seg.toFixed(0)+' s con buena calidad a esta fluidez. Si pides más, se baja la calidad de la imagen para que quepa.';
}
$('arch').onchange=async e=>{
  archivo=e.target.files[0]; audBuf=null; if(!archivo) return;
  $('prog').hidden=true;
  if(/\.ltd$/i.test(archivo.name)){ $('editor').hidden=true; if(confirm('Enviar '+archivo.name+' tal cual al dispositivo?')) subir(archivo); return; }
  v.src=URL.createObjectURL(archivo);
};
v.onloadedmetadata=async()=>{
  zoom=1; $('zoom').value=1; cX=v.videoWidth/2; cY=v.videoHeight/2;
  const d=v.duration; $('ini').max=Math.max(0,d-1).toFixed(1); $('ini').value=0;
  $('dur').max=Math.min(90,d).toFixed(1); $('dur').value=Math.min(20,d).toFixed(1);
  $('editor').hidden=false; textos(); await ir(0.05); pinta(cx2);
};
v.onerror=()=>{ if(archivo) mensaje('Este navegador no pudo abrir el video. Prueba con un MP4.',false); };
$('zoom').oninput=e=>{ zoom=+e.target.value; textos(); pinta(cx2); };
$('ini').oninput=async e=>{ const t=+e.target.value; $('dur').max=Math.min(90,v.duration-t).toFixed(1); textos(); await ir(t); pinta(cx2); };
$('dur').oninput=textos; $('fps').onchange=textos; $('conAudio').onchange=textos;
let arr=null;
cv.onpointerdown=e=>{ arr=[e.clientX,e.clientY]; cv.setPointerCapture(e.pointerId); };
cv.onpointermove=e=>{ if(!arr||!v.videoWidth)return; const k=recorte()[2]/cv.getBoundingClientRect().width;
  cX-=(e.clientX-arr[0])*k; cY-=(e.clientY-arr[1])*k; arr=[e.clientX,e.clientY]; pinta(cx2); };
cv.onpointerup=cv.onpointercancel=()=>{ arr=null; };

$('probar').onclick=async()=>{
  if(probando){ probando=false; return; }
  const t0=+$('ini').value, t1=t0+(+$('dur').value);
  probando=true; $('probar').textContent='■ Detener'; $('enviar').disabled=true;
  await ir(t0); v.muted=false;
  try{ await v.play(); }catch(e){ v.muted=true; try{ await v.play(); }catch(e2){ probando=false; } }
  const paso=()=>{ pinta(cx2);
    if(probando&&v.currentTime<t1&&!v.ended) return requestAnimationFrame(paso);
    v.pause(); v.muted=true; probando=false; $('probar').textContent='▶ Probar el tramo con sonido'; $('enviar').disabled=false; };
  paso();
};

// ---------- sonido ----------
async function decodificar(){
  if(audBuf) return audBuf;
  const AC=window.AudioContext||window.webkitAudioContext, ac=new AC(), ab=await archivo.arrayBuffer();
  audBuf=await new Promise((ok,mal)=>{ const p=ac.decodeAudioData(ab,ok,mal); if(p&&p.catch) p.catch(mal); });
  if(ac.close) ac.close();
  return audBuf;
}
// Devuelve el tramo en mono a 16 kHz, ya nivelado (Float32, -1..1)
async function sonido(t0,dur){
  const b=await decodificar(), n=Math.round(dur*HZ), OC=window.OfflineAudioContext||window.webkitOfflineAudioContext;
  let k=1, off; try{ off=new OC(1,n,HZ); }catch(e){ k=3; off=new OC(1,n*3,HZ*3); }
  const s=off.createBufferSource(); s.buffer=b;
  // La bocina de 40 mm no baja de ~200 Hz: quitar los graves que solo la hacen vibrar deja mas volumen util
  const pa=off.createBiquadFilter(); pa.type='highpass'; pa.frequency.value=110; pa.Q.value=0.7;
  const pb=off.createBiquadFilter(); pb.type='lowpass'; pb.frequency.value=6000; pb.Q.value=0.7;
  s.connect(pa); pa.connect(pb); pb.connect(off.destination); s.start(0,t0,dur);
  const r=await new Promise((ok,mal)=>{ off.oncomplete=e=>ok(e.renderedBuffer); const p=off.startRendering(); if(p&&p.catch) p.catch(mal); });
  let d=r.getChannelData(0);
  if(k>1){ const o=new Float32Array(n); for(let i=0;i<n;i++) o[i]=(d[i*3]+d[i*3+1]+d[i*3+2])/3; d=o; }
  // Nivel: se toma el 99.7 % de las muestras (no el pico suelto) y el resto se redondea suave
  const m=new Float32Array(Math.ceil(d.length/4)); for(let i=0,j=0;i<d.length;i+=4,j++) m[j]=Math.abs(d[i]); m.sort();
  const ref=m[Math.floor(m.length*0.997)]||0; if(ref<0.0005) throw new Error('silencio');
  const g=0.8/ref;
  for(let i=0;i<d.length;i++){ let x=d[i]*g, a=Math.abs(x); if(a>0.7){ a=0.7+0.29*Math.tanh((a-0.7)/0.29); x=x<0?-a:a; } d[i]=x; }
  return d;
}
const PASO=[7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767];
const SALTO=[-1,-1,-1,-1,2,4,6,8];
// IMA ADPCM, 4 bits por muestra, nibble bajo primero. n par.
function adpcm(d,n){
  const fun=Math.min(Math.round(HZ*0.02),n>>1), sal=new Uint8Array(n>>1); let pred=0, idx=0;
  for(let i=0;i<n;i++){
    let x=d[i]; if(i<fun) x*=i/fun; else if(i>=n-fun) x*=(n-1-i)/fun;      // entrada y salida suaves: el bucle no truena
    const s=Math.max(-32768,Math.min(32767,Math.round(x*32767)));
    let paso=PASO[idx], dif=s-pred, c=0, vp=paso>>3;
    if(dif<0){ c=8; dif=-dif; }
    if(dif>=paso){ c|=4; dif-=paso; vp+=paso; } paso>>=1;
    if(dif>=paso){ c|=2; dif-=paso; vp+=paso; } paso>>=1;
    if(dif>=paso){ c|=1; vp+=paso; }
    pred+=(c&8)?-vp:vp; pred=Math.max(-32768,Math.min(32767,pred));
    idx=Math.max(0,Math.min(88,idx+SALTO[c&7]));
    sal[i>>1]|=(i&1)?c<<4:c;
  }
  return sal;
}

// ---------- conversion ----------
function jpg(q){ return new Promise((ok,mal)=>cvJ.toBlob(b=>b?new Response(b).arrayBuffer().then(a=>ok(new Uint8Array(a)),mal):mal(new Error('jpeg')),'image/jpeg',q)); }
function avance(p,t){ $('prog').hidden=false; $('pb').style.width=(p*100).toFixed(1)+'%'; $('pt').className=''; $('pt').textContent=t; }
function mensaje(t,bien){ $('prog').hidden=false; $('pt').className=bien?'bien':'mal'; $('pt').textContent=t; }
function trabar(s){ ocupado=s; for(const id of ['arch','enviar','probar','zoom','ini','dur','fps','conAudio']) $(id).disabled=s; }

async function convertir(){
  const fps=+$('fps').value, t0=+$('ini').value, CAL=[0.82,0.72,0.62,0.52,0.42,0.33,0.25];
  let dur=Math.min(+$('dur').value,v.duration-t0), N=Math.max(1,Math.round(dur*fps));
  const conAudio=$('conAudio').checked, cap=E.capacidad;
  v.pause(); v.muted=true;
  try{ await v.play(); v.pause(); }catch(e){}       // iOS: hace falta un play() con el toque del usuario

  // 1) cuanto pesa un cuadro a cada calidad (muestra de 8 cuadros)
  avance(0,'Midiendo el video...');
  const prom=CAL.map(()=>0), M=Math.min(8,N);
  for(let i=0;i<M;i++){
    await ir(t0+(Math.floor((i+0.5)*N/M)+0.5)/fps); pinta(cxJ);
    for(let k=0;k<CAL.length;k++) prom[k]+=(await jpg(CAL[k])).length/M;
  }
  const porSeg=k=>fps*(prom[k]*1.07+4)+(conAudio?HZ/2:0);
  let q=CAL.findIndex((c,k)=>N/fps*porSeg(k)+64<=cap);
  let aviso='';
  if(q<0){ q=CAL.length-1; N=Math.max(1,Math.floor((cap-64)/porSeg(q)*fps)); aviso=' Se recortó a '+(N/fps).toFixed(1)+' s porque no cabía completo.'; }

  // 2) sonido
  let pcm=null;
  if(conAudio){
    avance(0.03,'Preparando el sonido...');
    try{ pcm=await sonido(t0,N/fps); }catch(e){ aviso+=' El video no trae sonido que se pueda leer: se envía sin audio.'; }
  }

  // 3) cuadros
  const cuadros=[]; let total=0; const tope=cap-64-(pcm?Math.ceil(N/fps*HZ/2)+2:0);
  for(let i=0;i<N;i++){
    await ir(t0+(i+0.5)/fps); pinta(cxJ);
    const j=await jpg(CAL[q]);
    if(total+j.length+4*(i+2)>tope){ N=i; aviso+=' Se recortó a '+(N/fps).toFixed(1)+' s para que cupiera.'; break; }
    cuadros.push(j); total+=j.length;
    if(i%3==0){ pinta(cx2); avance(0.05+0.6*i/N,'Convirtiendo: cuadro '+(i+1)+' de '+N); }
  }
  if(!cuadros.length) throw new Error('No se pudo leer ningún cuadro del video');
  N=cuadros.length;

  // 4) archivo .ltd
  let aud=null;
  if(pcm){ let n=Math.min(pcm.length,Math.round(N/fps*HZ)); n-=n%2; aud=adpcm(pcm,n); }
  const cab=new ArrayBuffer(32+4*(N+1)), dv=new DataView(cab);
  [0x4C,0x54,0x44,0x31].forEach((b,i)=>dv.setUint8(i,b));
  dv.setUint16(4,LADO,true); dv.setUint16(6,LADO,true); dv.setUint16(8,Math.round(fps*100),true); dv.setUint16(10,N,true);
  let pos=cab.byteLength;
  for(let i=0;i<N;i++){ dv.setUint32(32+4*i,pos,true); pos+=cuadros[i].length; }
  dv.setUint32(32+4*N,pos,true);
  dv.setUint32(12,HZ,true); dv.setUint32(16,aud?aud.length*2:0,true); dv.setUint32(20,pos,true); dv.setUint32(24,aud?aud.length:0,true);
  dv.setUint8(28,aud?1:0); dv.setUint8(29,1);
  const partes=[cab,...cuadros]; if(aud) partes.push(aud);
  return {blob:new Blob(partes,{type:'application/octet-stream'}),aviso,seg:N/fps};
}

function subir(blob,aviso,seg){
  return new Promise(ok=>{
    trabar(true);
    const x=new XMLHttpRequest(), f=new FormData(); f.append('archivo',blob,'latido.ltd');
    x.open('POST','/subir?bytes='+blob.size);
    x.upload.onprogress=e=>avance(0.65+0.35*e.loaded/(e.total||blob.size),'Enviando al dispositivo: '+(e.loaded/1024).toFixed(0)+' de '+(blob.size/1024).toFixed(0)+' KB');
    x.onload=()=>{ let r={}; try{ r=JSON.parse(x.responseText); }catch(e){}
      if(x.status==200&&r.ok){ $('pb').style.width='100%'; mensaje('Listo: '+(seg?seg.toFixed(1)+' s, ':'')+(blob.size/1024).toFixed(0)+' KB.'+(aviso||''),true); }
      else mensaje('No se pudo guardar: '+(r.error||('error '+x.status)),false);
      trabar(false); estado(); ok(); };
    x.onerror=()=>{ mensaje('Se perdió la conexión con el dispositivo. Vuelve a intentarlo.',false); trabar(false); ok(); };
    x.send(f);
  });
}
$('enviar').onclick=async()=>{
  if(ocupado||!archivo) return;
  trabar(true);
  try{ const r=await convertir(); await subir(r.blob,r.aviso,r.seg); }
  catch(e){ mensaje('No se pudo convertir: '+(e.message||e),false); trabar(false); }
};
estado(); setInterval(()=>{ if(!ocupado) estado(); },8000);
</script></body></html>
)LATIDO";
