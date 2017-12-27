const net=require("net")
const readline=require("readline")
const cmd = readline.createInterface({input:process.stdin})
cmd.prompt()
cmd.on("line",(line)=>{
	const command=line.split(" ").filter((str)=>str!=="")
	console.log(command)
	cmd.prompt()
})
const buf = Buffer.allocUnsafe(4)
buf.writeInt16BE(2,0)
buf[2]=0x66
buf[3]=0x77
let socket=net.connect(7647)
socket.write(buf)
// c.pipe(process.stdout)
// process.stdin.pipe(c)