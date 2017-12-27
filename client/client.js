const net=require("net")
const readline=require("readline")
const cmd = readline.createInterface({input:process.stdin})
cmd.on("line",(line)=>{
	const command=line.split(" ").filter((str)=>str!=="")
	console.log(command)
})
let inBuffer=Buffer.alloc(0)
let outBuffer=[]
let writable=true
let socket=net.connect(7647)
const writePacket=(buf)=>{
	let head=Buffer.alloc(2)
	head.writeInt16BE(buf.length,0)
	let packet=Buffer.concat([head,buf])
	if(writable){
		writable=socket.write(packet)
	}else{
		outBuffer.push(packet)
	}
}
socket.on("drain",()=>{
	while((outBuffer.length>0)&&writable){
		writable=socket.write(outBuffer.shift())
	}
})
socket.on("data",(buf)=>{
	inBuffer=Buffer.concat([inBuffer,buf])
	let expectedLength=inBuffer.readInt16BE(0)
	if(expectedLength+2>=inBuffer.length){
		let packet=inBuffer.slice(2,expectedLength+2)
		//process packet
	}
	console.log(buf.toString("utf-8",2))
})