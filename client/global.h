#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <openssl/sha.h>
#include <bits/stdc++.h>
#include <string>
#include <string.h>
#include <fcntl.h>
#include<netinet/in.h>
#include<sys/types.h>
#include <algorithm>
#include <pthread.h>
#include <semaphore.h>
#include<sys/stat.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define MAX_BUFFER_SIZE 1024
#define MAXREQUESTS 10000000
#define CLIENT_THREAD_COUNT 10000000
#define DELIMETER '|'
#define FILEINFOBUFFERSIZE 5120
#define CHUNKSIZE 524288

pthread_t threadIds[CLIENT_THREAD_COUNT];
int liveThreads = 0;

sem_t fileInfoMap_RLOCK;
sem_t fileInfoMap_WLOCK;
int fileInfoMapReadersCount = 0;

int serverPort;
string serverIPAddress;

bool isUserLoggedIn = false;
string userIdOfLoggedInUser;
int trackerPort;
string trackerIPAddress;
int tracker_sd;

pthread_t serverThreadId;

class FileInfo;
unordered_map<string, FileInfo*> fileInfoMap;


// download file commands
int createConnectionWithPeerServer(string peerServerIpAdress,int peerServerPort);
void performFileDownload(vector<string> resp);
void receiveFile(string fileName, int peer_sd);
unordered_map<string,pair<string,int>> generateClientInfoMap(string fileMetaData);
void processChunkBitMap
(vector<string> bitmap,unordered_map<int,vector<string>>& chunkInfoMap,
string userid,string fileName );
vector<string> downloadBitMapFromPeer(int client_sd,string fileName);
bool getChunkDataFromPeer
(int client_sd,string fileName,int chunkNo,string originalChunkSHAFromTracker,
string destPath,string groupid);
void downloadChunkFromPeer(int chunkNo,vector<string> peers,
unordered_map<string,pair<string,int>> clientInfoMap,
string fileName,string destPath,string groupid);
bool performDownloadFileChunkWise(string fileMetaDataFromTracker,
vector<string> commandArgs, long long fileSize,string groupid);
void serveDownloadFile(int client_sd, string command);

// serving client commands
void serveCreateUser(int client_sd,string command);
void serveLogin(int client_sd, string command);
void serveCreateGroup(int client_sd, string command);
void serveJoinGroup(int client_sd, string command);
void serveListPendingRequest(int client_sd, string command);
void serveAcceptRequest(int client_sd, string command);
void serveListAllGroups(int client_sd, string command);
int sha1( const char * name, unsigned char * out );
void bin2hex( unsigned char * src, int len, char * hex );
string getSHA1OfCompleteFile(char* filePath);
string calHashofchunk(char *schunk, int length1, int shorthashflag);
int saveAllChunkSHA(int client_sd,char* filePath,string fileName);
void serveUploadFile(int client_sd, string command);
void serveListAllFiles(int client_sd, string command);
void serveLogout(int client_sd, string command);
void serveLeaveGroup(int client_sd, string command);
void serveStopShare(int client_sd, string command);

// semaphore commands
string getFilePath(string fileName);
void addNewFile(string fileName, string fileSize,string SHA1, 
string filePath, bool isSharable);
bool readTrackerInfoFromFile(int& trackerPort, string& trackerIPAddress);;
string getPeerIP(int peer_sd);
void getServerIPAndPORT(char* input);
void updateChunkBitMap(string fileName, int chunkNo);
bool isAllChunksRecieved(string fileName);
bool isFileExists(string fileName);
void removeFileFromFileInfoMap(string fileName);

// serve serve commands
void* performPeerTasks(void* param);
void serveDownloadChunk(string command);
long long getFileSize(string fileName);
string getChunkSHA(string fileName, int chunkNo);
void serveDownloadChunkBitMapServer(string command);
vector<string> getChunkBitMap(string fileName);
bool isFileShareable(string fileName);