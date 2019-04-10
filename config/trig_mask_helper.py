

def convert_ch_list(chs):
	ch_bits = 30
	binstr = ''
	for i in range(30):
		if(i in chs):
			binstr += '1'
		else:
			binstr += '0'

	binmask = binstr[::-1]
	return hex(int(binmask, 2))



if __name__ == "__main__":
	l2_chs = [22,23,24]
	l1_chs = [6,7,8]
	print "l2 mask = " + convert_ch_list(l2_chs)
	print "l1 mask = " + convert_ch_list(l1_chs)
	
