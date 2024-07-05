#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <bits/stdc++.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>

#define MAXREQUESTS 10000000
#define TRACKERTHREADCOUNT 10000000
#define MAXBUFFERSIZE 1024
#define MAXCLIENTLIVECOUNT 10000000
#define DELIMETER "|"
#define FILEINFOBUFFERSIZE 5120
#define CHUNKSIZE 524288

using namespace std;

// flags for synchronization
sem_t clientInfoMap_RLOCK;
sem_t clientInfoMap_WLOCK;
int clientInfoMapReadersCount = 0;


sem_t groupInfoMap_RLOCK;
sem_t groupInfoMap_WLOCK;
int groupInfoMapReadersCount = 0;

sem_t fileInfoMap_RLOCK;
sem_t fileInfoMap_WLOCK;
int fileInfoMapReadersCount = 0;

sem_t clientInfoFile_RLOCK;
sem_t clientInfoFile_WLOCK;
int clientInfoFileReadersCount = 0;

sem_t fileInfoFile_RLOCK;
sem_t fileInfoFile_WLOCK;
int fileInfoFileReadersCount = 0;

sem_t groupInfoFile_RLOCK;
sem_t groupInfoFile_WLOCK;
int groupInfoFileReadersCount = 0;


class ClientInfo;
class Group;
class FileInfo;

int noOfClientsLoggedIn = 0;
int liveThreads = 0;

pthread_t trackerThreadIds[TRACKERTHREADCOUNT];
pthread_t clientLoginThreadIds[MAXCLIENTLIVECOUNT];

// SERVING CLIENT COMMANDS
void serveCreateUser(int client_sd,string commandInfo);
void serveCreateGroup(int client_sd, string commandInfo);
void serveJoinGroup(int client_sd, string commandInfo);
void serveListPendingRequest(int client_sd, string commandInfo);
void serveAcceptRequest(int client_sd, string commandInfo);
void serveListAllGroups(int client_sd, string commandInfo);
void serveUploadFile(int client_sd, string commandInfo);
void serveDownloadFile(int client_sd, string commandInfo);
bool serveClientLogin(int client_sd,string commandInfo);
void serveListFiles(int client_sd, string commandInfo);
void serveLogout(int client_sd,bool isClientLoggedIn,string loggedInUserId);
void serveLeaveGroup(int client_sd, string commandInfo);
void serveStopShare(int client_sd, string commandInfo);
void serveAddLeecher(int client_sd, string commandInfo);
void serveAddSeeder(int client_sd,string commandInfo);

// SEMAPHORE FUNCTIONS
bool isClientMemberOfGroup(string groupid,string userid);
bool isUserPresentInPendingList(string groupid, string userid);
bool isLoggedInUserOwner(string groupid,string loggedInUserId);
vector<string> getListOfPendingRequest(string groupid);
vector<string> getListOfAllGroups();
void addUserToPendingRequestList(string groupid, string userid);
bool updatePendingRequestList(string groupid, string userid);
bool isGroupExists(string groupid);
void createNewGroup(string groupid,string userid);
bool isUserValid(string userid, string password);
bool isUserExists(string userid);
void addNewUser(string userid, string password);
bool updateIpAndPort(string userid,string ip,string port);
void addNewFile
(string fileName, string groupid, string fileSize,string SHA1, string loggedInUserId);
void addFileNameToGroup(string fileName, string groupid);
bool isFileNameExistInGroup(string groupid, string fileName);
vector<string> getClientsInfo(vector<string> userids);
string getFileMetaData(string groupid, string fileName);
bool isClientOnline(string userid);
vector<string> getSeederList(string groupid, string fileName);
vector<string> getFileNames(string groupid);
void removeFilesFromFileInfoMap(string groupid,vector<string> files);
vector<string> removeFilesSharedByMemberFromFileInfo
(string groupid, vector<string> files, string userid);
void performLeaveGroupForOwner(string groupid, string userid);
void performLeaveGroupForMember(string groupid, string userid);
void removeFileNameFromGroup(string groupid,string fileName);
bool stopSharingTheFile(string groupid, string fileName,string userid);
void addLeecherInToFileInfo(string groupid, string filename,string userid);
void addSeederInToFileInfo(string groupid, string filename, string userid);
bool isFileExists(string groupid, string fileName);
bool addNewSeeder(string fileName, string groupid, string SHA1,string loggedInUserId);
void updateClientInfoMap(string loggedInUserId);

// FILE FUNCTIONS
void addNewUserFile(string,string);
void reloadClientInfoFromFile();
void reloadGroupInfoFromFile();
void reloadFileInfoFromFile();
void updateClientInfoFile();
void updateGroupInfoFile();
void updateFileInfoFile();

