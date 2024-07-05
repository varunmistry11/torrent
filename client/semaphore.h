
string getFilePath
(string fileName){

    string filePath = "";

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);
    
    // reading here
    FileInfo *fileInfo = fileInfoMap[fileName];
    filePath = fileInfo->getFileNamePath();

    sleep(0.1);
    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount--;

    if (fileInfoMapReadersCount == 0) {
        sem_post(&fileInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    return filePath;
}

void addNewFile
(string fileName, string fileSize,string SHA1, 
string filePath, bool isSharable){

     // Lock the semaphore
    sem_wait(&fileInfoMap_WLOCK);
 
    // writing here
    FileInfo *fileInfo = new 
    FileInfo(fileName,atoll(fileSize.c_str()),SHA1,filePath,isSharable);

    // generate key 
    fileInfoMap[fileName] = fileInfo;

    sleep(0.3);
    // Unlock the semaphore
    sem_post(&fileInfoMap_WLOCK);
   
}

bool readTrackerInfoFromFile
(int& trackerPort, string& trackerIPAddress){

    // read info from tracker_info.txt
    std::ifstream ifs("tracker_info.txt");
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

string getPeerIP
(int peer_sd){
    
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    int res = getpeername(peer_sd, (struct sockaddr *)&addr, &addr_size);
    char *peerIP = new char[20];
    strcpy(peerIP, inet_ntoa(addr.sin_addr));

    return peerIP;

}

void getServerIPAndPORT
(char* input){

    char * token = strtok(input, ":");
    if(token != NULL){
        serverIPAddress = token;
    }
    token = strtok(NULL, ":");
    if(token != NULL){
        serverPort = atoi(token);
    }

}

void updateChunkBitMap(string fileName, int chunkNo){

    // Lock the semaphore
   sem_wait(&fileInfoMap_WLOCK);
    
    // reading here
    FileInfo *fileInfo = fileInfoMap[fileName];
    fileInfo->setIsShareable(true);
    fileInfo->setChunkBitMap(chunkNo,true);
    fileInfoMap[fileName] = fileInfo;

    sleep(0.1);
  
    // UNLock the semaphore
    sem_post(&fileInfoMap_WLOCK);

}

bool isAllChunksRecieved(string fileName){

    bool isRecieved = true;

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);
    
    // reading here
    FileInfo *fileInfo = fileInfoMap[fileName];
    vector<bool> bitmap = fileInfo->getChunkBitMap();
    cout << endl;
    for(bool status : bitmap){
        cout << status << " ";
        isRecieved = isRecieved && status;
    }  
    cout << "is: " << isRecieved << endl;

    sleep(0.1);
    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount--;

    if (fileInfoMapReadersCount == 0) {
        sem_post(&fileInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    return isRecieved;

}

bool isFileExists(string fileName){

    bool isExists = false;

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);
    
    // reading here
    if(fileInfoMap.find(fileName) != fileInfoMap.end()){
        FileInfo *fileInfo = fileInfoMap[fileName];
        if(fileInfo->getIsShareable() == false)
            fileInfo->setIsShareable(true);
        fileInfoMap[fileName] = fileInfo;
        isExists = true;
    }
    

    sleep(0.1);
    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount--;

    if (fileInfoMapReadersCount == 0) {
        sem_post(&fileInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    return isExists;

}

void removeFileFromFileInfoMap(string fileName){

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing here
    if(fileInfoMap.find(fileName) != fileInfoMap.end())
        fileInfoMap.erase(fileName);
        
    // release lock
    sem_post(&fileInfoMap_WLOCK);
}
