#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sqlite3
import time
from pylab import *
import pylab
import numpy as np
import os.path

runFlag = True

dbFile = raw_input("Banco de dados (/home/asantos/sqlite3.db):")	

if ((dbFile=='') or not(os.path.isfile(dbFile))):
	dbFile = '/home/asantos/sqlite3.db'
# /home/asantos/Dropbox/Mestrado/Capturas/sqlite20160523.db

# Cria uma conexão e um cursor
con = sqlite3.connect(dbFile)
cur = con.cursor()

# Obtem lista de tabelas a partir da tabela CONTROL
tables = []
for row in cur.execute("SELECT ts, dataTable FROM CONTROL;"):
	nUsed, nTable = row[1].split('MEASURE')
	tables.append([int(nTable), time.strftime('%d %b %Y %H:%M:%S +0000',time.gmtime(row[0]))])
nTables = len(tables)
print 'Quantidade de tabelas= %d'%nTables


while(runFlag):
	# Obtem quantidade de linhas em cada tabela
	for idx, tTable in tables:
		query = 'SELECT COUNT(*) FROM MEASURE%010d;'%idx
		cur.execute(query)
		lenTable = cur.fetchone();
		print '%d. Table MEASURE%010d has %d records (%s)' % (idx, idx, lenTable[0], tTable)

	# Selecionando uma tabela e obtendo seus valores
	sel = input('Selecione uma opcao: ')
	if( sel<0 or sel>nTables):
		continue
	result = cur.execute('SELECT * FROM MEASURE%010d ORDER BY ts, id;'%(sel)).fetchall()

	# Identificando trechos não-contiguos
	ID_POS = 1
	last = [0, result[0][ID_POS]]
	tr = []
	count = 0
	for i, r in enumerate(result[1:]):
		if (int(r[ID_POS]) != int(result[i][ID_POS])+1):
			print('Indice %d ERRO %d'%(i, result[i][ID_POS]))
			tr.append([last, [i-1, result[i][ID_POS]]])
			count += 1		
			last = [i, r[ID_POS]]
	tr_order = []
	if (tr == []):
		# Não há trechos não-contiguos
		print("Already ordened")
		tr_order.append([[0, 0],[len(result[1:]),result[1:]]])
	else:
		# Reordenando trechos não-contiguos
		for row in tr:
			print(row)

		tr_order.append(tr.pop(0))

		tam = len(tr)
		while len(tr):
	
			for j, nxt in enumerate(tr):
				if ((tr_order[len(tr_order)-1][1][1] + 1) % 65536 == nxt[0][1]):
					tr_order.append(tr.pop(j))
					break
			if(tam == len(tr)):
				break
			tam = len(tr)
	
	
	print('Desordenados: %d'%(len(tr)))
	#for row in tr:
	#	print(row)
	print('Ordenados: %d'%(len(tr_order)))
	d = []
	fExport = open('/home/asantos/export.csv', 'w')
	for row in tr_order:
		for res in result[row[0][0]:row[1][0]]:
			d.append(res)
			for element in res:
				if (str(element)=='None'):
					fExport.write('-999999999 ')
				else:
					fExport.write(str(element)+' ')
				
			fExport.write('\n')		
	fExport.close()	

	v = np.asarray(result, dtype=np.float64)

	for nEletrodo in range(1,9):
		amp = np.square(np.add(np.multiply(v[:,nEletrodo*3-1],v[:,nEletrodo*3-1]), np.multiply(v[:,nEletrodo*3],v[:,nEletrodo*3-1])))
		print 'Media=%8.4e\tMax=%8.4e\tMin=%8.4e'%(np.mean(amp),np.max(amp),np.min(amp))

	n_graph = 0

	while(1):
		print('Analizando Dados:')
		print('1 - Gráfico amplitudes')
		print('2 - Gráfico fases')
		print('3 - Histograma amplitudes')
		print('4 - Histograma fases')
		print('5 - Imprimir amplitudes')
		print('6 - Imprimir fases')
		print('9 - Mostrar gráficos')
		print('0 - Sair');
		opcao = input('Digite:')

		if(opcao==0):
			runFlag = False
			break

		if(opcao==1):
			nEletrodo = input('Selecione o eletrodo:');
			amp = np.square(np.add(np.multiply(v[:,nEletrodo*3-1],v[:,nEletrodo*3-1]), np.multiply(v[:,nEletrodo*3],v[:,nEletrodo*3-1])))
			pylab.figure(n_graph)
			n_graph = n_graph + 1
			pylab.plot(amp)
			pylab.title('Amplitude Eletrodo '+ str(nEletrodo))

		if(opcao==2):
			nEletrodo = input('Selecione o eletrodo:');
			fase = np.arctan2(v[:,nEletrodo*3], v[:,nEletrodo*3-1])
			pylab.figure(n_graph)
			n_graph = n_graph + 1
			pylab.plot(fase)
			pylab.title('Fase Eletrodo '+ str(nEletrodo))
		
		if(opcao==3):
			nEletrodo = input('Selecione o eletrodo:');
			amp = np.square(np.add(np.multiply(v[:,nEletrodo*3-1],v[:,nEletrodo*3-1]), np.multiply(v[:,nEletrodo*3],v[:,nEletrodo*3-1])))
			pylab.figure(n_graph)
			n_graph = n_graph + 1
			pylab.hist(amp,100)
			pylab.title('Amplitude Eletrodo '+ str(nEletrodo))

		if(opcao==4):
			nEletrodo = input('Selecione o eletrodo:');
			fase = np.arctan2(v[:,nEletrodo*3], v[:,nEletrodo*3-1])
			pylab.figure(n_graph)
			n_graph = n_graph + 1
			pylab.hist(fase,100)
			pylab.title('Fase Eletrodo '+ str(nEletrodo))

		if(opcao==5):
			nEletrodo = input('Selecione o eletrodo:');
			amp = np.square(np.add(np.multiply(v[:,nEletrodo*3-1],v[:,nEletrodo*3-1]), np.multiply(v[:,nEletrodo*3],v[:,nEletrodo*3-1])))
			print(amp)

		if(opcao==6):
			nEletrodo = input('Selecione o eletrodo:');
			fase = np.arctan2(v[:,nEletrodo*3], v[:,nEletrodo*3-1])
			print(fase)
		
		if(opcao==9):
			pylab.show()
			n_graph = 1
			pylab.close('all')

		if(opcao==10):
			break

