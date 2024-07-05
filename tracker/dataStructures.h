
vector<string> parseString
(string input, char delimeter){

    stringstream ss(input);
    string token;
    vector<string> res;
    while(getline(ss,token,delimeter)){
        if(token != ""){
            res.push_back(token);
        }
    }

    return res;
}

string getString
(vector<string> data){

    string res = "";

    for(int i = 0;i< data.size();i++){
        if(i == 0){
            res = res + data[i];
        }else{
            res = res + DELIMETER + data[i];
        }
    }

    return res;
}

bool readTrackerInfoFromFile
(char* filename,int& trackerPort, string& trackerIPAddress){

    // read info from tracker_info.txt
    std::ifstream ifs(filename);
    vector<string> info;
    string data;
    if(ifs){
        while ( ifs ) {
            std::getline (ifs, data);
            info.push_back(data);
        }

        trackerIPAddress = info[0];
        trackerPort = atoi(info[1].c_str());
    }

    else{
        // LOGGER::ERROR("Unable to open tracker_info.txt");
        return false;
    }
    ifs.close();

    return true;

}

string getClientIP
(int client_sd){
    
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(client_sd, (struct sockaddr *)&addr, &addr_size);
    char *clientIP = new char[20];
    strcpy(clientIP, inet_ntoa(addr.sin_addr));

    return clientIP;

}

// <userid,clientInfoObj>
unordered_map<string,ClientInfo*> clientInfoMap; 
class ClientInfo{

    private:
    string userId; // char userId[100]
    string password; // char userId[50]
    string ip; // char ip[15]
    int port;
    bool isLoggedIn;

    public:

    ClientInfo() { }

    ClientInfo (string userId, string password){
        this->userId = userId;
        this->password = password;
        this->ip = "$"; // initially offline
        this->port = -1; // initially offline
        this->isLoggedIn = false; // initially offline
    } 

    void setUserId (string userId) { this->userId = userId; }
    void setPassword (string password) { this->password = password; }
    void setIP (string ip) { this->ip = ip; }
    void setPort (int port) { this->port = port; }
    void setIsLoggedIn(bool isLoggedIn) { this->isLoggedIn = isLoggedIn; }
    // void setGroupId (string groupId) { this->groupId = groupId; };

    string getUserId() { return this->userId; }
    string getPassword() { return this->password; }
    string getIP() { return this->ip; }
    bool getIsLoggedIn() { return this->isLoggedIn; }
    // string getGroupId() { return this->groupId; }
    int getPort() { return this->port; }

};


// <gid,groupObj>
unordered_map<string,Group*> groupInfoMap;
class Group{

    private:
    string groupId; // char groupId[100]
    string ownerUserId; // char ownerUserId[100]
    // char memberUserIds[100][100]
    vector<string> memberUserIds; // list of all client-member of groups
    // char fileNames[100][100]
    vector<string> fileNames; // list of all sharable complete files.
    vector<string> pendingRequestIds;

    public:

    Group() { }

    Group(string groupId,string ownerUserId){
        this->groupId = groupId;
        this->ownerUserId = ownerUserId;
        this->memberUserIds.push_back(ownerUserId);
    }

    void setGroupId(string groupId) { this->groupId = groupId; };
    void setOwnerUserId(string ownerUserId) { this->ownerUserId = ownerUserId; };
    void addNewMember(string userId) { this->memberUserIds.push_back(userId); };
    void addFileName(string fileName) { this->fileNames.push_back(fileName); };
    void addNewRequest(string userid) {  
        if(!isUserMemberOfGroup(userid))
            this->pendingRequestIds.push_back(userid); 
    }
    void setListPendingRequests(vector<string> list){
        this->pendingRequestIds = list;
    }

    bool isUserMemberOfGroup(string userid){
        for(string id : memberUserIds){
            if(id == userid){
                return true;
            }
        }  

        return false;
    }

    void removeFile(string fileName){
        int pos = -1;
        for(int i = 0;i < fileNames.size();i++){
            if(fileName == fileNames[i]){
                pos = i;
                break;
            }
        }
        if(pos != -1)
            fileNames.erase(fileNames.begin() + pos);
    }

    void removeMemberFromMemberIds(string userid){
        int pos = 0;
        for(int i = 0;i < memberUserIds.size(); i++){
            if(userid == memberUserIds[i]){
                pos = i;
                break;
            }
        }
        if(pos != -1)
            memberUserIds.erase(memberUserIds.begin() + pos);
    }
    
    string getFirstMember() { return this->memberUserIds[0]; }
    string getGroupId() { return this->groupId = groupId; };
    string getOwnerUserId() { return this->ownerUserId = ownerUserId; };
    vector<string> getMemberUserIds() { return this->memberUserIds; };
    vector<string> getFileNames() { return this->fileNames; };
    vector<string> getListOfPendingRequest() { return this->pendingRequestIds; }

    bool isFileNameExist(string fileName){

        for(string name : fileNames){
            if(name == fileName)
                return true;
        }

        return false;
    }

    void removeFilesSharedByMember(vector<string> files){
        vector<string> filesRemaining;
         set_difference(fileNames.begin(), fileNames.end(), files.begin(), files.end(),
            inserter(filesRemaining, filesRemaining.begin()));
        fileNames = filesRemaining;
    }

};


// <gid$fileName,FileInfo*>
unordered_map<string, FileInfo*> fileInfoMap;
class FileInfo{

    private:
    string fileName;
    long long fileSize;
    string SHA_1;
    vector<string> seederList;
    vector<string> leecherList;

    public:

    FileInfo() { }

    FileInfo(string fileName, long long fileSize,
        string SHA_1, string userid){
            this->fileName = fileName;
            this->fileSize = fileSize;
            this->SHA_1 = SHA_1;
            this->seederList.push_back(userid);
    }

    void setFileName(string fileName) { this->fileName = fileName; };
    void setfileSize(long long fileSize) { this->fileSize = fileSize; }
    void setSHA_1(string SHA_1) { this->SHA_1 = SHA_1; };
    void addInToSeederList(string userid) { 
        if(!isSeederPresent(userid))
            this->seederList.push_back(userid); 
    }
    void addInToLeecherList(string userid) { this->leecherList.push_back(userid); }
    void removeSeeder(string userid,int pos){
        this->seederList.erase(this->seederList.begin() + pos);
    }
    void addSeederList(vector<string> seederList) { this->seederList = seederList; };
    void addLeecherList(vector<string> leecherList) { this->leecherList = leecherList; };

    string getFileName() { return this->fileName; }
    long long getFileSize() { return this->fileSize; }
    string getSHA_1() { return this->SHA_1; }
    vector<string> getSeederList() { return this->seederList; }
    vector<string> getLeecherList() { return this->leecherList; }
    bool isSeederPresent(string userid){
        for(auto user : seederList){
            if(user == userid)
                return true;
        }
        return false;
    }
    bool isLeecherPresent(string userid){
        for(auto user : leecherList){
            if(user == userid)
                return true;
        }
        return false;
    }

    bool removeSeederFromList(string userid){
        for(int i = 0;i < seederList.size();i++){
            if(userid == seederList[i]){
                seederList.erase(seederList.begin() + i);
                return true;
            }
        }
        return false;
    }

    bool removeLeecherFromList(string userid){
        for(int i = 0;i < leecherList.size();i++){
            if(userid == leecherList[i]){
                leecherList.erase(leecherList.begin() + i);
                return true;
            }
        }
        return false;
    }

};
