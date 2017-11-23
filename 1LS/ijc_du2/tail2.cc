  // tail2.cc
  // Riesenie IJC-DU2, priklad a), 26.4.2015
  // Autor: Andrej Barna (xbarna01)
  // Prelozene: g++ (GCC) 4.8.4 na serveri Merlin
  // Popis: Funkcia tail prepisana do jazyka C++
 
 
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <cstring>
#include <fstream>
#include <queue>


int main(int argc, char **argv){
  
  // Default nastavenia
  std::ifstream srcFile;
  std::istream *sstream;
  sstream = &std::cin; // Zdrojovy subor je stdin
  int fromLine = 0; // Bool - ci sa pocita od riadku alebo n poslednych riadkov
  unsigned int lineCount = 10; // Default pocet riadkov
	  
  // Spracovanie argumentov
  int cArg = 1;

  if(argc-1){
    // Specifikovany pocet riadkov, resp. od ktoreho riadku sa ma vypisovat
    if(!strcmp(argv[cArg],"-n")){
      if(argc<3){
        fprintf(stderr,"Chyba ciselny argument!\n");
        return 1;
      }

      // Od ktoreho riadku pocitat - cislo s plusom
      char *strPtr;
      if(*argv[cArg+1]=='+'){
        lineCount = strtol(argv[cArg+1]+1,&strPtr,10);
        if(*strPtr || lineCount < 1){
          fprintf(stderr, "Pocet riadkov v nespravnom formate!\n");
          return 1;
        }
        fromLine = 1;
      }

      // Kolko riadkov pocitat
      else{
        lineCount = strtol(argv[cArg+1],&strPtr,10);
        if(*strPtr || lineCount < 1){
          fprintf(stderr, "Pocet riadkov v nespravnom formate!\n");
          return 1;
        }
      }

      cArg += 2;
    }

    // Zadany nazov suboru
    if(argc-cArg){
      srcFile.open(argv[cArg],std::ifstream::in);
      if(!srcFile.is_open()){
        fprintf(stderr, "Chyba pri otvarani suboru!\n");
        return 1;
      }
      sstream = &srcFile;
    }

  }
  

  std::string str; // Buffer string

  // Vypis od zadaneho riadku
  if(fromLine){
    unsigned int ln = 1;
    while(std::getline(*sstream,str)){
      // Vypis riadkov
      if(ln >= lineCount)
        std::cout << str << std::endl;
      ln++;
    }
  }

  // Vypis poslednych n riadkov
  else{
    // Tvorba queue, citanie zo suboru a ukladanie
    std::queue<std::string> ttail;
    while(std::getline(*sstream,str)){
      ttail.push(str);
      if(ttail.size() > lineCount)
        ttail.pop();
    }

    // Vypis
    while(ttail.size()){
      std::cout << ttail.front() << std::endl;
      ttail.pop();
    }
  }

  return 0;
}
