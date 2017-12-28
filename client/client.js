const net=require("net")
const readline=require("readline")
const cmd = readline.createInterface({input:process.stdin})
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
let loggedin=false
socket.on("data",(buf)=>{
	inBuffer=Buffer.concat([inBuffer,buf])
	let expectedLength=inBuffer.readInt16BE(0)
	if(expectedLength+2>=inBuffer.length){
		let packet=inBuffer.slice(2,expectedLength+2)
		inBuffer=inBuffer.slice(expectedLength+2)
		if(packet.length>0){
			if(packet[0]===0){
				if(packet.length>=2){
					if(packet[1]===0){
						loggedin=true
						console.log("Welcome.")
					}else if(packet[1]===1){
						console.log("Username do not exist.")
					}else if(packet[1]===2){
						console.log("Wrong password.")
					}else if(packet[1]===3){
						console.log("Please log out from another client before retry.")
					}
				}
			}
		}
	}
})
cmd.on("line",(line)=>{
	const command=line.split(" ").filter((str)=>str!=="")
	if(command.length>0){
		if(command[0]==="login"){
			if(command.length!==3){
				console.log("login <username> <password>")
			}else{
				let uBuf=Buffer.from(command[1])
				if(uBuf.length>16){
					console.log("username too long")
					return
				}
				let pBuf=Buffer.from(command[2])
				if(uBuf.length>16){
					console.log("password too long")
					return
				}
				let data=Buffer.alloc(33)
				data[0]=0
				uBuf.copy(data,1)
				pBuf.copy(data,17)
				writePacket(data)
			}
		}else{
			console.log("unknown command")
		}
	}
})