CC = g++
CFLAGS = -lstdc++ -std=c++0x -lpthread -lcurl -O2
INCLUDE = -I./../queryAnalyze/seg_include
LIBS = -L./../queryAnalyze/seg_lib -lccode -lsegmenter -luldict -lwordselecter -lullib -lpthread
#objects = main.o tool_function.o Calculate_Relevancy.o utf8_to_char.o word_segment.o TransCode.o QueryPreprocess.o Synonym_Termweight.o ic_url.o url_code.o cJSON.o
objects = main_new.o tool_function.o Calculate_Relevancy_new.o utf8_to_char.o word_segment.o TransCode.o QueryPreprocess.o Synonym_Termweight.o ic_url.o url_code.o cJSON.o

main: $(objects)
	$(CC) -o main $(objects) $(CFLAGS) $(INCLUDE) $(LIBS)

#main.o: main.cpp
#	$(CC) $(CFLAGS) -c main.cpp
main_new.o: main_new.cpp
	$(CC) $(CFLAGS) -c main_new.cpp

tool_function.o: tool_function.cpp
	$(CC) $(CFLAGS) -c tool_function.cpp

#Calculate_Relevancy.o: Calculate_Relevancy.cpp
#	$(CC) $(CFLAGS) -c Calculate_Relevancy.cpp
Calculate_Relevancy_new.o: Calculate_Relevancy_new.cpp
	$(CC) $(CFLAGS) -c Calculate_Relevancy_new.cpp

utf8_to_char.o: ../queryAnalyze/utf8_to_char.cpp
	$(CC) -c ../queryAnalyze/utf8_to_char.cpp

word_segment.o: ../queryAnalyze/word_segment.cpp
	$(CC) $(CFLAGS) -c ../queryAnalyze/word_segment.cpp

TransCode.o: ../queryAnalyze/seg_include/TransCode.c
	$(CC) -c ../queryAnalyze/seg_include/TransCode.c

QueryPreprocess.o: ../queryAnalyze/QueryPreprocess.cpp
	$(CC) $(CFLAGS) -c ../queryAnalyze/QueryPreprocess.cpp

Synonym_Termweight.o: ../queryAnalyze/Synonym_Termweight.cpp
	$(CC) $(CFLAGS) -c ../queryAnalyze/Synonym_Termweight.cpp

ic_url.o: ic_url.c
	$(CC) -c ic_url.c

cJSON.o: cJSON.c
	$(CC) -c cJSON.c

url_code.o: url_code.cpp
	$(CC) -c url_code.cpp

clean:
	rm *.o
