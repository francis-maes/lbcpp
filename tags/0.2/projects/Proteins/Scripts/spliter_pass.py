fileIn = open("CACHED_REF_SS3-SS8-SA_10_15_9_11_10_10_10_10.pass", "r")
fileSS3 = open("CACHED_REF_SS3-SS8-SA_10_15_9_11_10_10_10_10.pass.SS3", "w+")
fileSS8 = open("CACHED_REF_SS3-SS8-SA_10_15_9_11_10_10_10_10.pass.SS8", "w+")
fileSA = open("CACHED_REF_SS3-SS8-SA_10_15_9_11_10_10_10_10.pass.SA", "w+")
i = 0
line = fileIn.readline()
while line:
	if i % 3 == 0:
		fileSS3.write("%s" % line)
	elif i % 3 == 1:
		fileSS8.write("%s" % line)
	else:
		fileSA.write("%s" % line)
	i += 1
	line = fileIn.readline()
