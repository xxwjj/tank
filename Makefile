headerFile1 = agent
headerFile2 = game
headerFile3 = graph
headerFile4 = ai
headerFile5 = framewrok
commonHeadFile = third_party\jsoncpp\include

agent.o : 
	g++ -c agent\agent.cpp -I$(headerFile1) -I$(headerFile2) -I$(headerFile3) -I$(headerFile4) -I$(headerFile5) -I$(commonHeadFile)
	
