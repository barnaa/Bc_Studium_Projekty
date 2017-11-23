/* **********************************
 * Predmet: IPK, projekt 1
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


#define BSIZE 1024
#define MAXREDIRS 5


int parseAddress(std::string, std::string&, std::string&, uint *);
int connectTo(int *, std::string &, int);
std::string replaceSpaces(std::string);
int getVersion(char *);
int getCode(char *);
std::string getHeaderField(char *, std::string);
int getContentIndex(char *);
std::string getLineContent(char *, int);


//using namespace std;
int main(int argc, char **argv){

    if (argc != 2){
       fprintf(stderr,"Pouzitie: %s <URL>\n", argv[0]);
       return(EXIT_FAILURE);
    }

    // Spracovanie adresy
    std::string srvName, fileName, outFileName;
    uint portNum = 80;
    if(parseAddress(std::string(argv[1]), srvName, fileName, &portNum))
        return EXIT_FAILURE; //chyba pri parsovani

    if(!fileName.empty())
        outFileName = std::string(fileName).substr(std::string(fileName).find_last_of('/')+1);
    
    srvName = replaceSpaces(srvName);
    fileName = replaceSpaces(fileName);

    // Otvorime subor na vystup dat
    std::ofstream ofile;
    if(outFileName.empty())
    	ofile.open("index.html", std::ios::out | std::ios::trunc | std::ios::binary);
    else
    	ofile.open(outFileName.data(), std::ios::out | std::ios::trunc | std::ios::binary);



    // Premenna, do ktorej sa bude ukladat client socket
    int cSocket;

    if(connectTo(&cSocket, srvName, portNum))
        return EXIT_FAILURE;


    ///////////////////////////////////////////////////////////////////
    // Riesenie presmerovania, protokolu HTTP 1.0 a chybovych kodov
    //////////////////////////////////////////////////////////////////

    std::string request = "HEAD /" + fileName + " HTTP/1.1\r\nHost: " + srvName + "\r\n\r\n";

    int sentBytes, recBytes;
    char buffer[BSIZE];

    sentBytes = send(cSocket, request.data(), request.length(), 0);
    if (sentBytes < 0){
        std::cerr << "Chyba vo funkcii send!\n";
        return EXIT_FAILURE;
    }

    int httpver = 1;
    for(int i = 0; i <= MAXREDIRS; i++){
        recBytes = recv(cSocket, buffer, BSIZE, 0);
        if (recBytes < 0){
            std::cerr << "Chyba vo funkcii recv!\n";
            return EXIT_FAILURE;
        }

        int retCode = getCode(buffer);
        if(httpver && (retCode == 400 || !getVersion(buffer))){
            httpver = 0;
            request = "HEAD /" + fileName + " HTTP/1.0\r\nHost: " + srvName + "\r\n\r\n";
            i--;
        }
        else if(retCode/100 == 5 || retCode/100 == 4){
            std::cerr << "Chyba! Server vratil chybovy kod: " << retCode << std::endl;
            return EXIT_FAILURE;
        }
        else if(retCode == 200)
            break;
        else if(retCode == 301 || retCode == 302){
            if(i == MAXREDIRS){
                std::cerr << "Chyba! Privela presmerovani!\n";
                return EXIT_FAILURE;
            }
            std::string naddr = getHeaderField(buffer, "Location");
            if(parseAddress(naddr, srvName, fileName, &portNum))
                return EXIT_FAILURE;
            srvName = replaceSpaces(srvName);
            fileName = replaceSpaces(fileName);

            if(httpver)
                request = "HEAD /" + fileName + " HTTP/1.1\r\nHost: " + srvName + "\r\n\r\n";
            else
                request = "HEAD /" + fileName + " HTTP/1.0\r\nHost: " + srvName + "\r\n\r\n";
        }
        else{
            std::cerr << "Nastala neznama chyba pri komunikacii so serverom!\n"<< "Kod poslednej spravy bol " << retCode << "\n";
            return EXIT_FAILURE;
        }

        close(cSocket);
        if(connectTo(&cSocket, srvName, portNum))
            return EXIT_FAILURE;

        sentBytes = send(cSocket, request.data(), request.length(), 0);
        if (sentBytes < 0){
            std::cerr << "Chyba vo funkcii send!\n";
            return EXIT_FAILURE;
        }
    }


    ///////////////////////
    // Stahovanie suboru
    ///////////////////////

    request.replace(0, 4, "GET");
    close(cSocket);
    if(connectTo(&cSocket, srvName, portNum))
        return EXIT_FAILURE;

    // Odosleme GET request
    sentBytes = send(cSocket, request.data(), request.length(), 0);
    if (sentBytes < 0){
        std::cerr << "Chyba vo funkcii send!\n";
        return EXIT_FAILURE;
    }

    // Prijmame prvu odpoved
    recBytes = recv(cSocket, buffer, BSIZE, 0);
    if (recBytes < 0){
        std::cerr << "Chyba vo funkcii recv!\n";
        return EXIT_FAILURE;
    }

    // Podla typu odpovede nasledne stahujeme obsah ziadaneho suboru
    if(getVersion(buffer)){
        // HTTP 1.1
        if(getHeaderField(buffer, "Transfer-Encoding") == "chunked"){
            int cindex = getContentIndex(buffer);
            uint chunkLen = std::stoul(getLineContent(buffer, cindex), nullptr, 16);
            cindex += getLineContent(buffer, cindex).length() + 2;
            while(chunkLen != 0){
                if(chunkLen + cindex > recBytes){
                    ofile.write(buffer + cindex, recBytes - cindex);
                    chunkLen -= recBytes - cindex;
                    cindex = 0;
                }
                else{
                    ofile.write(buffer + cindex, chunkLen);
                    
                    if(chunkLen + cindex + 2 > recBytes){
                        std::string trailer = std::string(buffer).substr(cindex + chunkLen, recBytes - chunkLen - cindex);
                        recBytes = recv(cSocket, buffer, BSIZE, 0);
                        if(recBytes < 0){
                            std::cerr << "Chyba vo funkcii recv!\n";
                            return EXIT_FAILURE;
                        }

                        trailer += std::string(buffer).substr(0, 2 - trailer.length());
                        if(trailer.length() != 2 || trailer != "\r\n"){
                            std::cerr << "Chybajuce CRLF na konci chunku!" << std::endl;
                            return EXIT_FAILURE;
                        }
                        chunkLen = 0;
                    }
                    else if(std::string(buffer).substr(cindex + chunkLen, 2) != "\r\n"){
                        std::cerr << "Chybajuce CRLF na konci chunku!" << std::endl;
                        return EXIT_FAILURE;
                    }

                    if(recBytes <= cindex + chunkLen + 2){
                        recBytes = recv(cSocket, buffer, BSIZE, 0);
                        if(recBytes < 0){
                            std::cerr << "Chyba vo funkcii recv!\n";
                            return EXIT_FAILURE;
                        }
                        cindex = 0;
                    }
                    else cindex = chunkLen + 2;

                    chunkLen = std::stoul(getLineContent(buffer, cindex), nullptr, 16);
                    cindex += getLineContent(buffer, cindex).length() + 2;
                }

                if(!cindex){
                    recBytes = recv(cSocket, buffer, BSIZE, 0);
                    if (recBytes < 0){
                        std::cerr << "Chyba vo funkcii recv!\n";
                        return EXIT_FAILURE;
                    }
                }
            }
        }
        else{
            int cindex = getContentIndex(buffer);
            if(getHeaderField(buffer, "Content-Length") != ""){
                int chars = std::stoul(getHeaderField(buffer, "Content-Length"), nullptr, 10);
                while(chars){
                    if(chars + cindex > recBytes){
                        ofile.write(buffer + cindex, recBytes - cindex);
                        chars -= recBytes - cindex;
                        cindex = 0;
                        recBytes = recv(cSocket, buffer, BSIZE, 0);
                        if (recBytes < 0){
                            std::cerr << "Chyba vo funkcii recv!\n";
                            return EXIT_FAILURE;
                        }
                    }
                    else{
                        ofile.write(buffer + cindex, chars);
                        chars = 0;
                    }
                    ofile.flush();
                }
            }
            else while(recBytes != 0){
                ofile.write(buffer + cindex, recBytes-cindex);
                cindex = 0;
                recBytes = recv(cSocket, buffer, BSIZE, 0);
                if (recBytes < 0){
                    std::cerr << "Chyba vo funkcii recv!\n";
                    return EXIT_FAILURE;
                }
            }
        }
    }
    else{
        // HTTP 1.0
        int cindex = getContentIndex(buffer);
        if(getHeaderField(buffer, "Content-Length") != ""){
            int chars = std::stoul(getHeaderField(buffer, "Content-Length"), nullptr, 10);
            while(chars){
                if(chars + cindex > recBytes){
                    ofile.write(buffer + cindex, recBytes - cindex);
                    chars -= recBytes - cindex;
                    cindex = 0;
                    recBytes = recv(cSocket, buffer, BSIZE, 0);
                    if (recBytes < 0){
                        std::cerr << "Chyba vo funkcii recv!\n";
                        return EXIT_FAILURE;
                    }
                }
                else{
                    ofile.write(buffer + cindex, chars);
                    chars = 0;
                }
                ofile.flush();
            }
        }
        else while(recBytes != 0){
            ofile.write(buffer + cindex, recBytes-cindex);
            cindex = 0;
            recBytes = recv(cSocket, buffer, BSIZE, 0);
            if (recBytes < 0){
                std::cerr << "Chyba vo funkcii recv!\n";
                return EXIT_FAILURE;
            }
        }
    }

    close(cSocket);
    ofile.close();

    return 0;
}



// Rozlozi adresu na diely, ktore sa nasledne pouziju pre pripojenie na server
int parseAddress(std::string address, std::string &srvName, std::string &fileName, uint *portNum){

    char s[address.length()+1];
    strcpy(s, address.data());

    char *protocol = strtok(s, "://");
    if(strcmp(protocol, "http")){
        fprintf(stderr, "Bol pouzity neznamy protokol: %s!\n", protocol);
        return(EXIT_FAILURE);
    }
    char *srvNameWPort = strtok(NULL, "/");
    char *fNamePtr = strtok(NULL, "");
    if(fNamePtr != NULL)
        fileName = fNamePtr;
    
    srvName = strtok(srvNameWPort, ":");
    char *portNumStr = strtok(NULL, "");

    if(portNumStr != NULL){
        char *ptr = NULL;
        *portNum = strtoul(portNumStr, &ptr, 10);
        if(*ptr != 0 || *portNum > 65565){
            fprintf(stderr, "Chybne cislo portu: %s!\n", portNumStr);
            return(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}



// Vytvori socket, spracuje adresu a pripoji sa k serveru
int connectTo(int *cSocket, std::string &srvName, int portNum){

    if ((*cSocket = socket(AF_INET, SOCK_STREAM, 0)) <= 0){
        std::cerr << "Chyba pri otvarani socketu!\n";
        return(EXIT_FAILURE);
    }

    struct hostent *server;
    struct sockaddr_in serverAddr;

    if ((server = gethostbyname(srvName.data())) == NULL){
        std::cerr << "Chyba! " << srvName.data() << " neexistuje!\n";
        return(EXIT_FAILURE);
    }

    bzero((char *) &serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddr.sin_addr.s_addr, server->h_length);
    serverAddr.sin_port = htons(portNum);

    if (connect(*cSocket, (const struct sockaddr *) &serverAddr, sizeof(serverAddr)) != 0){
        std::cerr << "Nastala chyba pri pripajani sa na server!\n";
        return(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}



// Nahradi medzery v stringu s (URL adrese) patricnym kodom
// ... a po edite aj vlnovky
std::string replaceSpaces(std::string s){
    for(int i = s.size()-1; i >= 0; i--){
        if(s[i] == ' ')
            s.replace(i, 1, "%20");
        else if(s[i] == '~')
            s.replace(i, 1, "%7e");
    }
    return s;
}



// Vrati 1, ak je hlavicka v bufferi s typu HTTP 1.1,
// alebo 0 ak je to HTTP 1.0 hlavicka
int getVersion(char *s){
    std::string str = s;
    if(str.find("HTTP/1."))
        return 0;
    int i = str.find('.');
    return atoi(s+i+1);
}



// Ziska z hlavicky cislo HTTP kodu
int getCode(char *s){
    std::string str = s;
    if(str.find("HTTP/"))
        return 0;
    int i = str.find(' ');
    return atoi(s+i+1);
}



// Vrati hodnotu hlavickoveho pola (header field), ktore je dane argumentom field
// Funkcia je case-insensitive
std::string getHeaderField(char *s, std::string field){
    std::string str = s;
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    std::string lfield = field;
    std::transform(lfield.begin(), lfield.end(), lfield.begin(), ::tolower);
    int i = str.find(lfield);
    return i == -1 ? "" : getLineContent(s, i + field.length() + 2);
}



// Vrati index znaku, ktory je za "CRLFCRLF" castou na konci headeru
int getContentIndex(char *s){
	int i = std::string(s).find("\r\n\r\n");
    return i == -1 ? 0 : i+4;
}



// Vrati obsah riadku daneho indexom i v bufferi s
std::string getLineContent(char *s, int i){
    std::string str = std::string(s).substr(i);
    std::smatch m;
    std::regex e("([^\r\n]+)(\r\n)");
    std::regex_search(str, m, e);
    return m[1];
}