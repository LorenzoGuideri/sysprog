import sys

def calculate_crc(chunk_type, chunk_data):
    crc = 0xFFFFFFFF
    for byte in chunk_type:
        crc ^= byte
        for _ in range(8):
            mask = -(crc & 1)
            crc = (crc >> 1) ^ (0xEDB88320 & mask)
    for byte in chunk_data:
        crc ^= byte
        for _ in range(8):
            mask = -(crc & 1)
            crc = (crc >> 1) ^ (0xEDB88320 & mask)
    return crc ^ 0xFFFFFFFF

def is_valid_crc(chunk_type, chunk_data, expected_crc):
    calculated_crc = calculate_crc(chunk_type, chunk_data)
    return calculated_crc==int.from_bytes(expected_crc, "big")

cdef = "\t_n: _t (_l)\n_D"
kdef = "\t_k: _t\n"
pdef = "_f: _w x _h, _c, _d bits per sample, _N chunks\n_C"
png_header = bytes([137, 80, 78, 71, 13, 10, 26, 10])

end_of_options=len(sys.argv)

for i in range(1, len(sys.argv)):
	if i < end_of_options and (sys.argv[i].startswith("c=") or sys.argv[i].startswith("p=") or sys.argv[i].startswith("k=")):
		option_type = sys.argv[i][0]
		value = sys.argv[i][2:]
		if option_type == 'c':
			cdef = value
		elif option_type == 'p':
			pdef = value
		elif option_type == 'k':
			kdef = value
	else:
		if sys.argv[i].startswith("--"):
			end_of_options = i
		else:
			f = open(sys.argv[i], mode="rb")
			data=f.read()
			if not f:
				print("file not valid")
				break
			if len(data)<(8+4+4+13+4) or data[0:8]!=png_header:
				print("Invalid signature")
				break
			valid=1
			filename=sys.argv[i]
			width=int.from_bytes(data[16:20], "big")
			heigth=int.from_bytes(data[20:24], "big")
			bdp=data[24]
			color=data[25]
			types,dati,crcs,metadata=[],[],[],[]
			ii=8
			while True:
				lun=data[ii]*256**3+data[ii+1]*256**2+data[ii+2]*256+data[ii+3]
				t=data[ii+4:ii+8]
				d=data[ii+8:ii+8+lun]
				c=data[ii+8+lun:ii+8+lun+4]
				if not is_valid_crc(t,d,c):
					valid=0
					print("NOT VALID CRC")
					break
				types.append(t)
				dati.append(d)
				crcs.append(c)
				if  t==b"IEND":
					break
				if  t==b"tEXt":
					metadata.append(d.split(b'\x00'))
				ii+=8 #len and type
				ii+=lun+4
			if valid:
				escaped=0
				for iii in range(len(pdef)):
					if escaped:
						if pdef[iii]=="f":
							print(filename,end="")
						elif pdef[iii]=="w":
							print(width,end="")
						elif pdef[iii]=="h":
							print(heigth,end="")
						elif pdef[iii]=="d":
							print(bdp,end="")
						elif pdef[iii]=="c":
							if color==0:
								print("Greyscale",end="")
							elif color==2:
								print("Truecolor",end="")
							elif color==3:
								print("Indexed",end="")
							elif color==4:
								print("Greyscale+alpha",end="")
							elif color==6:
								print("Truecolor+alpha",end="")
							else:
								print("Unknown color type",end="")
						elif pdef[iii]=="N":
							print(len(types),end="")
						elif pdef[iii]=="C":
							for iv in range(len(types)):
								escaped=0
								for v in range(len(cdef)):
									if escaped:
										if cdef[v]=="n": print(str(iv+1),end="")
										elif cdef[v]=="t": print(types[iv].decode(),end="")
										elif cdef[v]=="l": print(len(dati[iv]),end="")
										elif cdef[v]=="c": print(crcs[iv],end="")
										elif cdef[v]=="D":
											for vi in range(len(dati[iv])):
												end=" " if (vi+1)%16 else "\n"
												if dati[iv][vi]<16: print(" ",end='') # ATTENTO PD
												print(hex(dati[iv][vi])[2:],end=end)
											print()
										else: print(cdef[v],end="")
										escaped=0
									else:
										if cdef[v]=='_': escaped=1
										else: print(cdef[v],end="")
						elif pdef[iii]=="K":

							for iv in range(len(metadata)):
								escaped=0
								for v in range(len(kdef)):
									if escaped:
										if kdef[v]=="k": print(metadata[iv][0].decode(),end="")
										elif kdef[v]=="t": print(metadata[iv][1].decode(),end="")
										else: print(kdef[v],end="")
										escaped=0
									else:
										if kdef[v]=='_': escaped=1
										else: print(kdef[v],end="")
						else: print(pdef[iii],end="")
						escaped=0
					else:
						if pdef[iii]=='_': escaped=1
						else: print(pdef[iii],end="")
			f.close()
