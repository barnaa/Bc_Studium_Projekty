/* **********************************
 * Predmet: IPK, projekt 2 - klient
 * Autor: Andrej Barna (xbarna01)
 * E-mail: xbarna01@stud.fit.vutbr.cz
 * Ak. rok: 2015/2016, semester letny
 */


#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#define BSIZE 1024



int parseArgs(int argc, char **argv, std::string&, uint *, std::string&);
int connectTo(int *, std::string &, int);
int recvFile(int, std::string &, bool);
int sendFile(int, std::string &, bool);
std::string getCode(char *);
std::string getHeaderField(char *, std::string);
int getContentIndex(char *);
std::string getLineContent(char *, int);



int main(int argc, char **argv){

	// Spracovanie argumentov
	std::string serverName;
	int port = -1;
	std::string fileName;
	int act = 0;

	try{
		for(int i = 1; i < argc; i++){
			if(i+1 == argc)
				throw std::string("Chyba: nedostatocny pocet argumentov!\n");
			std::string a = std::string(argv[i]);
			if(a == "-d" || a == "-u"){
				if(act)
					throw std::string("Chyba: redefinicia suboru urceneho na nahratie/stiahnutie!\n");
				fileName = std::string(argv[i+1]);
				if(a == "-d")
					act = 1;
				else
					act = 2;
			}
			else if(a == "-p"){
				if(port != -1)
					throw std::string("Chyba: redefinicia portu serveru!\n");
				char *ptr = NULL;
				port = strtoul(argv[i+1], &ptr, 10);
				if(*ptr != 0 || port > 65565)
					throw std::string("Chyba: nespravne cislo portu!\n");
			}
			else if(a == "-h"){
				if(!serverName.empty())
					throw std::string("Chyba: redefinicia serveru!\n");
				serverName = std::string(argv[i+1]);
			}
			else
				throw std::string("Chyba: neznamy parameter!\n");
			i++;
		}
		if(!act)
			throw std::string("Chyba: nebola zadana akcia!\n");
		else if(serverName.empty())
			throw std::string("Chyba: nebol zadany server!\n");
		else if(port == -1)
			throw std::string("Chyba: nebol zadany port!\n");
	}
	catch(std::string s){
		std::cerr << s;
		return EXIT_FAILURE;
	}

	// Pripojenie sa k serveru
	int cSocket;
	int location;
	if(!(location = connectTo(&cSocket, serverName, port)))
		return EXIT_FAILURE;

	// Odoslanie ziadosti a spracovanie odpovede
	if(act == 1){
		if(recvFile(cSocket, fileName, location-1)){
			close(cSocket);
			return EXIT_FAILURE;
		}
	}
	else{
		if(sendFile(cSocket, fileName, location-1)){
			close(cSocket);
			return EXIT_FAILURE;
		}
	}

	// Uzavretie socketu
	close(cSocket);
	return EXIT_SUCCESS;
}


// Vytvori socket, spracuje adresu a pripoji sa k serveru
int connectTo(int *cSocket, std::string &srvName, int portNum){

	if ((*cSocket = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
		std::cerr << "Chyba pri otvarani socketu!\n";
		return(0);
	}

	struct hostent *server;
	struct sockaddr_in serverAddr;

	if ((server = gethostbyname(srvName.data())) == NULL){
		std::cerr << "Chyba! " << srvName.data() << " neexistuje!\n";
		return(0);
	}

	bzero((char *) &serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
	serverAddr.sin_port = htons(portNum);


	if (connect(*cSocket, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0){
		std::cerr << "Nastala chyba pri pripajani sa na server!\n";
		return(0);
	}

    struct in_addr lHost;
    inet_aton("127.0.0.1", &lHost);

    // Ak je to localhost spojenie, tak vrati 2, inak vrati 1
    if(lHost.s_addr == serverAddr.sin_addr.s_addr)
    	return(2);
    return(1);
}



/* receive File - preberanie suboru od serveru
 *  cSocket  - socket, na ktorom sa ma komunikovat so serverom
 *  fileName - nazov suboru, ktory chceme stiahnut
 *  isLocal  - true, ak je server na tom istom stroji ako klient (localhost)
 */
int recvFile(int cSocket, std::string &fileName, bool isLocal){
	int sentBytes, recBytes;
	char buffer[BSIZE];

	// Poslanie ziadosti o subor a prebranie odpovede
	std::string request = "IPKP D\nf: " + fileName + "\n" + (isLocal? "p: " +
	  std::string(getcwd(buffer, BSIZE*sizeof(char))) + "\n" : "") + "\x17\n";
	sentBytes = send(cSocket, request.data(), request.length(), 0);
	if (sentBytes < 0){
		std::cerr << "Chyba vo funkcii send!\n";
		return EXIT_FAILURE;
	}
	recBytes = recv(cSocket, buffer, BSIZE, 0);
	if (recBytes < 0){
		std::cerr << "Chyba vo funkcii recv!\n";
		return EXIT_FAILURE;
	}

	// Spracovanie kodu odpovede od serveru
	if(std::string(buffer).find("IPKP ") != 0){
		// Neznamy protokol
		std::cerr << "Chyba: server poslal odpoved v nespravnom formate!\n";
		return EXIT_FAILURE;
	}
	if(getCode(buffer) == "A") 
		// Jedna sa o lokalne spojenie a klient aj server su v tom istom priecinku
		return EXIT_SUCCESS;
	else if(getCode(buffer) == "E"){
		// Chyba spojenia
		std::cerr << "Chyba: nie je mozne preniest pozadovany subor!\n";
		return EXIT_FAILURE;
	}
	else if(getCode(buffer) != "R"){
		std::cerr << "Chyba: server poslal odpoved v nespravnom formate!\n";
		return EXIT_FAILURE;
	}
	
	// Otvorenie suboru pre zapis
	std::ofstream ofile;
	try{
		ofile.open(std::string(fileName).substr(std::string(fileName).find_last_of('/')+1),
				std::ios::out | std::ios::trunc | std::ios::binary);
	}
	catch(std::exception){
		std::cerr << "Nebolo mozne ulozit pozadovany subor!\n";
		return EXIT_FAILURE;
	}

	// Ziskanie velkosti suboru
	unsigned long fsize = std::stoul(getHeaderField(buffer, "s"), nullptr, 10);

	// Preberanie a ulozenie suboru
	int cindex = getContentIndex(buffer);
	while(fsize){
		if(fsize + cindex > recBytes){
			ofile.write(buffer + cindex, recBytes - cindex);
			fsize -= recBytes - cindex;
			cindex = 0;
			recBytes = recv(cSocket, buffer, BSIZE, 0);
			if (recBytes < 0){
				std::cerr << "Chyba vo funkcii recv!\n";
				ofile.close();
				return EXIT_FAILURE;
			}
		}
		else{
			ofile.write(buffer + cindex, fsize);
			fsize = 0;
		}
		ofile.flush();
	}
	
	// Zavretie suboru
	ofile.close();
	return EXIT_SUCCESS;
}



/* send File - posielanie suboru na server
 *  cSocket  - socket, na ktorom sa ma komunikovat so serverom
 *  fileName - nazov suboru, ktory chceme poslat
 *  isLocal  - true, ak je server na tom istom stroji ako klient (localhost)
 */
int sendFile(int cSocket, std::string &fileName, bool isLocal){
	int sentBytes, recBytes;
	char buffer[BSIZE];

	// Otvorenie a nacitanie suboru, ktory chceme odoslat a priprava ziadosti pre server
	std::string request;
	std::string fileContent;
	try{
		std::ifstream ifile;
		ifile.open(fileName, std::ios::in | std::ios::ate | std::ios::binary);
		unsigned long fsize = ifile.tellg();
		ifile.seekg(0, std::ios::beg);
		request = "IPKP U\nf: " + fileName + "\ns: " + std::to_string(fsize)
		+ "\n" + (isLocal? "p: " + std::string(getcwd(buffer, BSIZE*sizeof(char)))
		+ "\n" : "") + "\x17\n";

		while(fsize){
			if(fsize > BSIZE){
				ifile.read(buffer, BSIZE);
				fileContent.append(buffer, BSIZE);
				fsize -= BSIZE;
			}
			else{
				ifile.read(buffer, fsize);
				fileContent.append(buffer, fsize);
				break;
			}
		}

		ifile.close();
	}
	catch(...){
		std::cerr << "Pozadovany subor nebolo mozne otvorit!\n";
		return EXIT_FAILURE;
	}

	// Odoslanie ziadosti na nahranie suboru na server a prijem odpovede
	sentBytes = send(cSocket, request.data(), request.length(), 0);
	if (sentBytes < 0){
		std::cerr << "Chyba vo funkcii send!\n";
		return EXIT_FAILURE;
	}
	recBytes = recv(cSocket, buffer, BSIZE, 0);
	if (recBytes < 0){
		std::cerr << "Chyba vo funkcii recv!\n";
		return EXIT_FAILURE;
	}

	// Spracovanie odpovede od serveru
	if(std::string(buffer).find("IPKP ") != 0){
		// Neznamy protokol
		std::cerr << "Chyba: server poslal odpoved v nespravnom formate!\n";
		return EXIT_FAILURE;
	}
	if(getCode(buffer) == "A")
		// Localhost spojenie, klient aj server su v tom istom priecinku - prenos sa nevykona
		return EXIT_SUCCESS;
	else if(getCode(buffer) == "E"){
		// Chyba - neda sa zapisovat do suboru, alebo ine
		std::cerr << "Chyba: nie je mozne preniest pozadovany subor!\n";
		return EXIT_FAILURE;
	}
	else if(getCode(buffer) != "R"){
		std::cerr << "Chyba: server poslal odpoved v nespravnom formate!\n";
		return EXIT_FAILURE;
	}

	// Posielame data
	sentBytes = send(cSocket, fileContent.data(), fileContent.length(), 0);
	if (sentBytes < 0){
		std::cerr << "Chyba vo funkcii send!\n";
		return EXIT_FAILURE;
	}

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