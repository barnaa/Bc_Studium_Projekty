/* **********************************
 * Predmet: IPK, projekt 2 - server
 * Autor: Andrej Barna (xbarna01)
 * E-mail: xbarna01@stud.fit.vutbr.cz
 * Ak. rok: 2015/2016, semester letny
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <thread>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


#define BSIZE 1024



int recvFile(int, char *);
int sendFile(int, char *);
std::string getCode(char *);
std::string getHeaderField(char *, std::string);
int getContentIndex(char *);
std::string getLineContent(char *, int);



int main(int argc, const char * argv[]) {
	// Spracovanie argumentov (validny je len argument -p cislo_portu, teda je primitivne)
	if (argc != 3){
		fprintf(stderr,"Pouzitie: %s -p <port>\n", argv[0]);
		return EXIT_FAILURE;
	}
	int port;
	if(std::string(argv[1]) != "-p"){
		std::cerr << "Chyba: neznamy argument: " << argv[1] << "!\n";
		return EXIT_FAILURE;
	}
	else{
		char *ptr = NULL;
		port = strtoul(argv[2], &ptr, 10);
		if(*ptr != 0 || port > 65565){
			std::cerr << "Chyba: nespravne cislo portu: " << argv[2] << "!\n";
			return EXIT_FAILURE;
		}
	}
	
	// Vytvorenie welcome socketu a jeho priprava na obsluhu klientov
	int wSocket;
	if ((wSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		std::cerr << "Chyba vo funkcii socket()!\n";
		return EXIT_FAILURE;
	}
	
	struct sockaddr_in sa;
	memset(&sa,0,sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port = htons(port);

	if (bind(wSocket, (struct sockaddr*)&sa, sizeof(sa)) < 0){
		std::cerr << "Chyba vo funkcii bind()!\n";
		return EXIT_FAILURE;
	}
	if ((listen(wSocket, 1)) < 0){
		std::cerr << "Chyba vo funkcii listen()!\n";
		return EXIT_FAILURE;
	}


	// Obsluha klientov
	struct sockaddr_in sa_client;
	socklen_t sa_client_len = sizeof(sa_client);
	while(true){
		int cSocket = accept(wSocket, (struct sockaddr*)&sa_client, &sa_client_len);		
		if (cSocket > 0){
			// Pripojil sa klient, vytvorime pre neho proces, ktory ho obsluzi
			pid_t cProcess = fork();
			if(cProcess == 0){ // Client process - zaciatok
				char request[BSIZE];
				int recBytes;
				// Prijmeme poziadavku klienta
				recBytes = recv(cSocket, request, BSIZE, 0);
				if (recBytes < 0){
					std::cerr << "Chyba vo funkcii recv()!\n";
					return EXIT_FAILURE;
				}

				// Podla obsahu prveho riadku rozhodneme, ktora funkcia sa ma zavolat 
				std::string reqType = getLineContent(request, 0);
				if(reqType == "IPKP D")
					return sendFile(cSocket, request);
				else if(reqType == "IPKP U")
					return recvFile(cSocket, request);
				else{
					// Poslat error message klientovi - chybna poziadavka
					send(cSocket, "IPKP E\n\x17\n\0", 10, 0);
					close(cSocket);
					return EXIT_FAILURE;
				}
				// Koniec procesu klienta
			}
			else if(cProcess < 0){
				std::cerr << "Chyba vytvarania procesu pre klienta!" << std::endl;
				close(cSocket);
				close(wSocket);
				return EXIT_FAILURE;
			}
		}
		else{
			std::cerr << "Chyba vo funkcii accept()!" << std::endl;
			close(wSocket);
			return EXIT_FAILURE;
		}
	}
	close(wSocket);
	return EXIT_SUCCESS;
}



/* receive File - preberanie suboru od klienta, ak chce uploadnut subor
 *  cSocket - socket, na ktorom sa ma komunikovat s klientom
 *  request - pole znakov, ktore obsahuje spravu, ktoru poslal klient
 */
int recvFile(int cSocket, char *request){
	int sentBytes, recBytes;
	char buffer[BSIZE];

	// Ziskanie nazvu suboru a jeho velkosti z klientovej ziadosti
	std::string fileName = getHeaderField(request, "f");
	if(fileName == "" || getHeaderField(request, "s") == ""){
		sentBytes = send(cSocket, "IPKP E\n\x17\n\0", 10, 0);
		if(sentBytes < 0)
			std::cerr << "Chyba vo funkcii send!\n";
		close(cSocket);
		return EXIT_FAILURE;
	}
	unsigned long fsize = std::stoul(getHeaderField(request, "s"), nullptr, 10);

	// Overovanie, ci klient nie je localhost a patricne opatrenia
	std::ofstream ofile;
	std::string path = getHeaderField(request, "p");
	if(path == std::string(getcwd(buffer, BSIZE*sizeof(char)))){
		// Binarka klienta aj serveru su v tom istom priecinku
		if(fileName.find(path) == 0 && fileName.substr(path.length()).find_last_of('/') == std::string::npos){
			// Posielany subor je v priecinku, kde je aj server
			sentBytes = send(cSocket, "IPKP A\n\x17\n\0", 10, 0);
			if(sentBytes < 0){
				std::cerr << "Chyba vo funkcii send!\n";
				close(cSocket);
				return EXIT_FAILURE;
			}
			close(cSocket);
			return EXIT_SUCCESS;
		}
		else if(fileName.find("./") == 0 && fileName.find_last_of('/') == 1){
			// Relativna cesta, v priecinku serveru/klienta
			sentBytes = send(cSocket, "IPKP A\n\x17\n\0", 10, 0);
			if(sentBytes < 0){
				std::cerr << "Chyba vo funkcii send!\n";
				close(cSocket);
				return EXIT_FAILURE;
			}
			close(cSocket);
			return EXIT_SUCCESS;
		}
		else{
			// Subor je umiestneny v inej lokacii, teda ho normalne preberieme
			try{
				ofile.open(std::string(fileName).substr(std::string(fileName).find_last_of('/')+1),
							std::ios::out | std::ios::trunc | std::ios::binary);
			}
			catch(...){
				sentBytes = send(cSocket, "IPKP E\n\x17\n\0", 10, 0);
				if(sentBytes < 0)
					std::cerr << "Chyba vo funkcii send!\n";
				close(cSocket);
				return EXIT_FAILURE;
			}
		}
	}
	else{
		// Binarky klienta a serveru nie su v totoznom priecinku, alebo sa nejedna o localhost
		try{
			ofile.open(std::string(fileName).substr(std::string(fileName).find_last_of('/')+1),
						std::ios::out | std::ios::trunc | std::ios::binary);
		}
		catch(...){
			sentBytes = send(cSocket, "IPKP E\n\x17\n\0", 10, 0);
			if(sentBytes < 0)
				std::cerr << "Chyba vo funkcii send!\n";
			close(cSocket);
			return EXIT_FAILURE;
		}
	}

	// Posielame prazdnu response spravu, ze sme schopni preberat subor
	sentBytes = send(cSocket, "IPKP R\n\x17\n\0", 10, 0);
	if (sentBytes < 0){
		std::cerr << "Chyba vo funkcii send!\n";
		close(cSocket);
		ofile.close();
		return EXIT_FAILURE;
	}

	// Prijimame a ukladame subor
	recBytes = recv(cSocket, buffer, BSIZE, 0);
	if (recBytes < 0){
		std::cerr << "Chyba vo funkcii recv!\n";
		close(cSocket);
		ofile.close();
		return EXIT_FAILURE;
	}
	while(fsize){
		if(fsize > recBytes){
			ofile.write(buffer, recBytes);
			fsize -= recBytes;
			recBytes = recv(cSocket, buffer, BSIZE, 0);
			if (recBytes < 0){
				std::cerr << "Chyba vo funkcii recv!\n";
				close(cSocket);
				ofile.close();
				return EXIT_FAILURE;
			}
		}
		else{
			ofile.write(buffer, fsize);
			fsize = 0;
		}
		ofile.flush();
	}

	// Zatvarame subor a ukoncujeme spojenie
	ofile.close();
	close(cSocket);
	return EXIT_SUCCESS;
}



/* send File - posielanie suboru klientovi, ak chce stiahnut subor
 *  cSocket - socket, na ktorom sa ma komunikovat s klientom
 *  request - pole znakov, ktore obsahuje spravu, ktoru poslal klient
 */
int sendFile(int cSocket, char *request){
	int sentBytes;
	char buffer[BSIZE];

	// Ziskanie nazvu suboru z klientovej ziadosti
	std::string fileName = getHeaderField(request, "f");
	if(fileName.find("/") != std::string::npos){
		if(fileName.find("./") != 0 || fileName.find_last_of('/') != 1){
			// Klient ziada o subor v inom priecinku nez je priecinok servera
			send(cSocket, "IPKP E\n\x17\n\0", 10, 0);
			close(cSocket);
			return EXIT_FAILURE;
		}
	}

	if(getHeaderField(request, "p") == std::string(getcwd(buffer, BSIZE*sizeof(char)))){
		// Jedna sa o localhost spojenie, kde binarky serveru a klienta su v rovnakom priecinku
		send(cSocket, "IPKP A\n\x17\n\0", 10, 0);
		close(cSocket);
		return EXIT_SUCCESS;
	}

	// Nacitanie obsahu suboru
	std::string response = "IPKP R\nf: " + fileName + "\ns: ";
	try{
		std::ifstream ifile;
		ifile.open(fileName, std::ios::in | std::ios::ate | std::ios::binary);
		unsigned long fsize = ifile.tellg();
		response += std::to_string(fsize) + "\n\x17\n";
		ifile.seekg(0, std::ios::beg);

		while(fsize){
			if(fsize > BSIZE){
				ifile.read(buffer, BSIZE);
				response.append(buffer, BSIZE);
				fsize -= BSIZE;
			}
			else{
				ifile.read(buffer, fsize);
				response.append(buffer, fsize);
				break;
			}
		}

		ifile.close();
	}
	catch(std::exception){
		std::cerr << "Subor nebolo mozne otvorit na citanie: " << fileName <<"!\n";
		send(cSocket, "IPKP E\n\x17\n\0", 10, 0);
		close(cSocket);
		return EXIT_FAILURE;
	}

	// Poslanie suboru klientovi
	sentBytes = send(cSocket, response.data(), response.length(), 0);
	if (sentBytes < 0){
		std::cerr << "Chyba vo funkcii send!\n";
		close(cSocket);
		return EXIT_FAILURE;
	}

	// Ukoncenie komunikacie
	close(cSocket);
	return EXIT_SUCCESS;
}



// Vrati zvysok riadku za castou "IPKP ", co by malo byt jedno z pismen A, E, D, R, U
std::string getCode(char *s){
	return getLineContent(s, 5);
}



// Vrati hodnotu hlavickoveho pola (header field), ktore je dane argumentom field
// Funkcia je case-insensitive
std::string getHeaderField(char *s, std::string field){
	std::string str = s;
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	std::string lfield = field;
	std::transform(lfield.begin(), lfield.end(), lfield.begin(), ::tolower);
	return getLineContent(s, str.find("\n" + lfield + ": ") + field.length() + 3);
}



// Vrati index znaku, ktory je za hlavickou, tj. zaciatok obsahu
int getContentIndex(char *s){
	int i = std::string(s).find("\n\x17\n");
	return i == -1 ? 0 : i+3;
}



// Vrati obsah riadku daneho indexom i v bufferi s
std::string getLineContent(char *s, int i){
	std::string str = std::string(s).substr(i);
	std::smatch m;
	//std::regex e("([^\r\n]+)(\r\n)");
	std::regex e("(.+)(\r?\n)");
	std::regex_search(str, m, e);
	return m[1];
}
