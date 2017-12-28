const net=require("net")
const readline=require("readline")
const cmd = readline.createInterface({input:process.stdin})
let inBuffer=Buffer.alloc(0)
const outBuffer=[]
let writable=true
const socket=net.connect(7647)
const writePacket=(buf)=>{
	console.log(`<--${buf.length}`)
	const head=Buffer.alloc(4)
	head.writeInt32BE(buf.length,0)
	const packet=Buffer.concat([head,buf])
	// console.log(packet)
	if(writable){
		writable=socket.write(packet)
	}else{
		outBuffer.push(packet)
	}
}
// let packnum=0
const attemptSendFile=()=>{
	if(outFd!==null){
		while(writable){
			let buf=Buffer.alloc(4096)
			let length=require("fs").readSync(outFd,buf,0,4096)
			// packnum++
			// if(packnum%10===0){
			// 	console.log(`--${packnum}--`)
			// }
			if(length===0){
				const data=Buffer.alloc(2)
				data[0]=7
				data[1]=2
				writePacket(data)
				require("fs").closeSync(outFd)
				outFd=null
				console.log("File transport finish")
				break
			}else{
				const data=Buffer.alloc(2)
				data[0]=7
				data[1]=1
				writePacket(Buffer.concat([data,buf.slice(0,length)]))
			}
		}
	}
}
socket.on("drain",()=>{
	while(outBuffer.length>0&&writable){
		writable=socket.write(outBuffer.shift())
	}
	attemptSendFile()
})
let loggedin=false
let chatting=false
let inFd=null
let outFd=null
socket.on("data",(buf)=>{
	inBuffer=Buffer.concat([inBuffer,buf])
	while(inBuffer.length>=4){
		const expectedLength=inBuffer.readInt32BE(0)
		if(expectedLength+4<=inBuffer.length){
			console.log(`-->${expectedLength}`)
			const packet=inBuffer.slice(4,expectedLength+4)
			inBuffer=inBuffer.slice(expectedLength+4)
			if(packet.length>0){
				if(packet[0]===0){// login return
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
				}else if(packet[0]===1){// search return
					const count=Math.floor((packet.length-1)/16)
					console.log(`${count} user(s):`)
					for(let i=0;i<count;i++){
						const nBuf=packet.slice(i*16+1,i*16+17)
						let length=nBuf.indexOf(0)
						if(length<0){
							length=16
						}
						console.log(nBuf.toString("utf-8",0,length))
					}
				}else if(packet[0]===2){// profile return
					if(packet.length>=33){
						const nBuf=packet.slice(1,17)
						let nlength=nBuf.indexOf(0)
						if(nlength<0){
							nlength=16
						}
						const name=nBuf.toString("utf-8",0,nlength)
						if(name!==""){
							console.log(`You are ${name}.`)
						}
						const pnBuf=packet.slice(17,33)
						let pnlength=pnBuf.indexOf(0)
						if(pnlength<0){
							pnlength=16
						}
						const peername=pnBuf.toString("utf-8",0,pnlength)
						if(peername!==""){
							console.log(`You are chatting with ${peername}.`)
						}else{
							console.log(`You are not chatting with anyone.`)
						}
					}
				}else if(packet[0]===3){// add return
					if(packet.length>=2){
						if(packet[1]===0){
							console.log("Friend added.")
						}else if(packet[1]===1){
							console.log("Username do not exist.")
						}else if(packet[1]===2){
							console.log("Cannot add yourself as friend.")
						}else if(packet[1]===3){
							console.log("He/She is already you friend.")
						}
					}
				}else if(packet[0]===4){// ls return
					const count=Math.floor((packet.length-1)/16)
					console.log(`${count} friend(s):`)
					for(let i=0;i<count;i++){
						const nBuf=packet.slice(i*16+1,i*16+17)
						let length=nBuf.indexOf(0)
						if(length<0){
							length=16
						}
						console.log(nBuf.toString("utf-8",0,length))
					}
				}else if(packet[0]===5){// chat/exit return
					if(packet.length>=2){
						if(packet[1]===0){
							console.log(chatting?"Exited.":"Start Chatting.")
							chatting=!chatting
						}else if(packet[1]===1){
							console.log("Username do not exist.")
						}else if(packet[1]===2){
							console.log("Cannot chat with yourself.")
						}
					}
				}else if(packet[0]===6){// msg
					if(packet.length>=17){
						const nBuf=packet.slice(1,17)
						let nlength=nBuf.indexOf(0)
						if(nlength<0){
							nlength=16
						}
						const name=nBuf.toString("utf-8",0,nlength)
						const message=packet.toString("utf-8",17)
						console.log(`${name}: ${message}`)
					}
				}else if(packet[0]===7){// file
					if(packet.length>=17){
						const nBuf=packet.slice(1,17)
						let nlength=nBuf.indexOf(0)
						if(nlength<0){
							nlength=16
						}
						const name=nBuf.toString("utf-8",0,nlength)
						const message=packet.slice(17)
						if(message[0]===0){
							if(inFd!==null){
								console.log("!!! UNEXPECTED NEW FILE PACKET !!!")
								return
							}
							let filename=message.toString("utf-8",1)
							console.log(expectedLength,message.slice(1))
							console.log(`${name} send you a file "${filename}"`)
							try{
								inFd=require("fs").openSync(filename,"w")
							}catch(err){
								console.log(`fs error:${err}`)
							}
						}else if(message[0]===1){
							let buffer=message.slice(1)
							let length=require("fs").writeSync(inFd,buffer)
							console.log(`#${length}#${buffer.length}#`)
						}else if(message[0]===2){
							require("fs").closeSync(inFd)
							inFd=null
							console.log("File saved")
						}
					}
				}
			}
		}else{
			break
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
				if(loggedin){
					console.log("You has logged in.")
					return
				}
				const uBuf=Buffer.from(command[1])
				if(uBuf.length>16){
					console.log("username too long")
					return
				}
				const pBuf=Buffer.from(command[2])
				if(uBuf.length>16){
					console.log("password too long")
					return
				}
				const data=Buffer.alloc(33)
				data[0]=0
				uBuf.copy(data,1)
				pBuf.copy(data,17)
				writePacket(data)
			}
		}else if(command[0]==="search"){
			if(command.length!==1){
				console.log("search")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				const data=Buffer.alloc(1)
				data[0]=1
				writePacket(data)
			}
		}else if(command[0]==="profile"){
			if(command.length!==1){
				console.log("profile")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				const data=Buffer.alloc(1)
				data[0]=2
				writePacket(data)
			}
		}else if(command[0]==="add"){
			if(command.length!==2){
				console.log("add <username>")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				const data=Buffer.alloc(17)
				data[0]=3
				const uBuf=Buffer.from(command[1])
				if(uBuf.length>16){
					console.log("username too long")
					return
				}
				uBuf.copy(data,1)
				writePacket(data)
			}
		}else if(command[0]==="ls"){
			if(command.length!==1){
				console.log("ls")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				const data=Buffer.alloc(1)
				data[0]=4
				writePacket(data)
			}
		}else if(command[0]==="chat"){
			if(command.length!==2){
				console.log("chat <username>")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				if(chatting){
					console.log("Please exit first")
					return
				}
				const data=Buffer.alloc(17)
				data[0]=5
				const uBuf=Buffer.from(command[1])
				if(uBuf.length>16){
					console.log("username too long")
					return
				}
				uBuf.copy(data,1)
				writePacket(data)
			}
		}else if(command[0]==="exit"){
			if(command.length!==1){
				console.log("exit")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				if(!chatting){
					console.log("You has exit")
					return
				}
				const data=Buffer.alloc(17)
				data[0]=5
				writePacket(data)
			}
		}else if(command[0]==="sendmsg"){
			if(command.length!==2){
				console.log("sendmsg <msg>")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				if(!chatting){
					console.log("Please enter a chat")
					return
				}
				const data=Buffer.alloc(1)
				data[0]=6
				writePacket(Buffer.concat([data,Buffer.from(command[1])]))
			}
		}else if(command[0]==="sendfile"){
			if(command.length!==2){
				console.log("sendfile <path>")
			}else{
				if(!loggedin){
					console.log("Please log in")
					return
				}
				if(!chatting){
					console.log("Please enter a chat")
					return
				}
				if(outFd!==null){
					console.log("Please wait for current file transport finish.")
					return
				}
				let name=Buffer.from(require("path").basename(command[1]))
				try{
					outFd=require("fs").openSync(command[1],"r")
				}catch(err){
					console.log(`fs error:${err}`)
					return
				}
				console.log("Start file transport")
				const data=Buffer.alloc(2)
				data[0]=7
				data[1]=0
				writePacket(Buffer.concat([data,name]))
				attemptSendFile()
			}
		}else{
			console.log("unknown command")
		}
	}
})