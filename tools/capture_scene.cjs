const fs=require('fs'),vm=require('vm'),path=require('path');
process.chdir(path.join(__dirname,'..'));fs.mkdirSync('.asset-build',{recursive:true});
class RecordingCanvas{
 constructor(){this.ops=[];this.m=[1,0,0,1,0,0];this.stack=[];this.clips=[];this.path=[];this.fillStyle='#000';this.strokeStyle='#000';this.lineWidth=1}
 point(x,y){let [a,b,c,d,e,f]=this.m;return [a*x+c*y+e,b*x+d*y+f]}
 transform(a,b,c,d,e,f){const m=this.m;this.m=[m[0]*a+m[2]*b,m[1]*a+m[3]*b,m[0]*c+m[2]*d,m[1]*c+m[3]*d,m[0]*e+m[2]*f+m[4],m[1]*e+m[3]*f+m[5]]}
 translate(x,y){this.transform(1,0,0,1,x,y)}scale(x,y){this.transform(x,0,0,y,0,0)}
 save(){this.stack.push({m:this.m.slice(),clips:this.clips.slice(),fill:this.fillStyle,stroke:this.strokeStyle,width:this.lineWidth})}
 restore(){let s=this.stack.pop();this.m=s.m;this.clips=s.clips;this.fillStyle=s.fill;this.strokeStyle=s.stroke;this.lineWidth=s.width}
 beginPath(){this.path=[]}moveTo(x,y){this.path.push([this.point(x,y)])}lineTo(x,y){if(!this.path.length)this.moveTo(x,y);else this.path.at(-1).push(this.point(x,y))}closePath(){if(this.path.length)this.path.at(-1).push(this.path.at(-1)[0])}
 bezierCurveTo(a,b,c,d,e,f){let p=this.path.at(-1),s=p.at(-1),u=this.point(a,b),v=this.point(c,d),z=this.point(e,f);for(let i=1;i<=16;i++){let t=i/16,q=1-t;p.push([q*q*q*s[0]+3*q*q*t*u[0]+3*q*t*t*v[0]+t*t*t*z[0],q*q*q*s[1]+3*q*q*t*u[1]+3*q*t*t*v[1]+t*t*t*z[1]])}}
 quadraticCurveTo(a,b,c,d){let p=this.path.at(-1),s=p.at(-1),u=this.point(a,b),z=this.point(c,d);for(let i=1;i<=12;i++){let t=i/12,q=1-t;p.push([q*q*s[0]+2*q*t*u[0]+t*t*z[0],q*q*s[1]+2*q*t*u[1]+t*t*z[1]])}}
 ellipse(x,y,rx,ry){this.path.push(Array.from({length:65},(_,i)=>this.point(x+rx*Math.cos(i*Math.PI/32),y+ry*Math.sin(i*Math.PI/32))))}
 rect(x,y,w,h){this.path.push([[x,y],[x+w,y],[x+w,y+h],[x,y+h],[x,y]].map(p=>this.point(...p)))}
 fillRect(x,y,w,h){let old=this.path;this.path=[];this.rect(x,y,w,h);this.fill();this.path=old}
 fill(){this.ops.push({kind:'fill',paths:this.path,style:this.fillStyle,clips:this.clips.slice()})}
 stroke(){this.ops.push({kind:'stroke',paths:this.path,style:this.strokeStyle,width:this.lineWidth*Math.abs(this.m[0]),clips:this.clips.slice()})}
 clip(){this.clips.push(this.path)}
 createPattern(c){return {density:c.ctx.ops.filter(o=>o.style==='#000').length}}
}
const g=new RecordingCanvas();const context={g,state:{weather:0,frame:0},document:{createElement:()=>{const ctx=new RecordingCanvas();return {ctx,getContext:()=>ctx}}},Math};vm.createContext(context);
let html=fs.readFileSync('design/rlcd_clock_v09.html','utf8');let code=html.slice(html.indexOf('const weather='),html.indexOf('function text('));vm.runInContext(code,context);
for(let w=0;w<20;w++)for(let f=0;f<8;f++){g.ops=[];context.state.weather=w;context.state.frame=f*2;vm.runInContext('drawScene()',context);fs.writeFileSync(`.asset-build/${w}-${f}.json`,JSON.stringify(g.ops))}
console.log('Captured 160 native vector frames from approved HTML scene');
