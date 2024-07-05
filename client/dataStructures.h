
class FileInfo{

    private:
    string fileName;
    long long fileSize;
    string SHA_1;
    string filePath;
    // it will tell that i have chunk available for sharing or not
    vector<bool> chunkBitMap;
    vector<string> allChunkSHA;
    bool isShareable;

    public:
    FileInfo(string fileName, long long fileSize,
        string SHA_1, string filePath, bool isShareable){
            this->fileName = fileName;
            this->fileSize = fileSize;
            this->SHA_1 = SHA_1;
            this->filePath = filePath;
            this->isShareable = isShareable;
            this->initChunkMap();
    }

    void initChunkMap(){
        int noOfChunks = getNoOfChunks();
        this->chunkBitMap.resize(noOfChunks);
        for(int i = 0;i < noOfChunks; i++){
            chunkBitMap[i] = isShareable;
        }
    }

    void setIsShareable(bool isShareable) { this->isShareable = isShareable; }
    void setFileName(string fileName) { this->fileName = fileName; };
    void setfileSize(long long fileSize) { this->fileSize = fileSize; }
    void setSHA_1(string SHA_1) { this->SHA_1 = SHA_1; };
    void setFilePath(string filePath) { this->filePath = filePath; }
    void setChunkBitMap(int chunkNo, bool value) { this->chunkBitMap[chunkNo] = value; }
    void addAllChunkSHA(vector<string> chunkSHA){
        this->allChunkSHA = chunkSHA;
    }

    string getFileName() { return this->fileName; }
    long long getFileSize() { return this->fileSize; }
    string getSHA_1() { return this->SHA_1; }
    string getFileNamePath() { return this->filePath; }
    bool getIsShareable() { return this->isShareable; }
    bool isChunkAvailable(int chunkNo) { return this->chunkBitMap[chunkNo]; }
    vector<string> getAllChunkSHA() { return this->getAllChunkSHA(); }
    int getNoOfChunks() { return ceil((float)this->fileSize / CHUNKSIZE); }
    vector<bool> getChunkBitMap() { return this->chunkBitMap; }
    vector<string> getChunkBitMapString(){
        vector<string> bitmap(getNoOfChunks());
        for(int i = 0;i < getNoOfChunks();i++){
            bitmap[i] = chunkBitMap[i] ? "1" : "0";
        }
        return bitmap;
    }

    string getChunkSHA(int chunkNo){
        return this->allChunkSHA[chunkNo];
    }


};

bool readTrackerInfoFromFile(string fileName){

    // read info from tracker_info.txt
    std::ifstream ifs(fileName);
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
        LOGGER::ERROR("Unable to open tracker_info.txt");
        return false;
    }

    ifs.close();
    
    return true;

}

vector<string> parseString
(string input, char delimiter){

    stringstream ss(input);
    string token;
    vector<string> res;
    while(getline(ss,token,delimiter)){
        if(token != ""){
            res.push_back(token);
        }
    }

    return res;

}

string getString
(vector<string> data, char delimiter = '|'){

    string res = "";

    for(int i = 0;i< data.size();i++){
        if(i == 0){
            res = res + data[i];
        }else{
            res = res + delimiter + data[i];
        }
    }

    return res;

}

void logTrackerResponse
(string response){
    
    vector<string> res = parseString(response,'|');

    if(res[0] == "true" || res[0] == "success") { printf(res[1].c_str()); }
    
    else { printf(res[1].c_str()); }

    printf("\n");

}

vector<string> getCommandArguments
(string command){

    stringstream ss(command);
    string token;
    vector<string> res;
    while(getline(ss,token,' ')){
        if(token != ""){
            res.push_back(token);
        }
    }
    
    return res;

}
