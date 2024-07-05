bool isFileShareable(string fileName){

    bool isShareable = false;

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);
    
    // reading here
    // check if file exist
    if(fileInfoMap.find(fileName) != fileInfoMap.end()){
        FileInfo *fileInfo = fileInfoMap[fileName];
        isShareable = fileInfo->getIsShareable();
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

    return isShareable;

}

vector<string> getChunkBitMap(string fileName){

    vector<string> bitmap;

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);
    
    // reading here
    // check if file exist
    if(fileInfoMap.find(fileName) != fileInfoMap.end()){
        bitmap = fileInfoMap[fileName]->getChunkBitMapString();
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

    return bitmap;

}

void serveDownloadChunkBitMapServer(string command){

    // [download_bitmap, filename, peer_sd]
    LOGGER::INFO("serveDownloadChunkBitMapServer start... %s", command.c_str());
    vector<string> commandArgs = parseString(command,' ');
    string fileName = commandArgs[1];
    int peer_sd = atoi(commandArgs[2].c_str());

    LOGGER::DEBUG("serving download_bitmap for file %s",fileName.c_str());

    vector<string> response(2);

    // check is user logged in and is file shareable
    if(isUserLoggedIn){
        if(isFileShareable(fileName)){
            vector<string> chunkBitmap = getChunkBitMap(fileName);
            string bitmap = getString(chunkBitmap);
            response[0] = "true";
            response[1] = "Bit map for file " + fileName + " found!!";
            response.push_back(bitmap);
            LOGGER::SUCCESS("Bit map sent to the client.. for file %s", fileName.c_str());
        }else{
            response[0] = "false";
            response[1] = fileName + " requested file is not shareable!!";
            LOGGER::ERROR("%s requested file is not shareable!!", fileName.c_str());   
        }
    }else{
        response[0] = "false";
        response[1] = "client is offline!!";
        LOGGER::ERROR("client is offline");
    }

    string reply = getString(response);
    send(peer_sd, reply.c_str(), reply.size(), 0);

    LOGGER::DEBUG("serving download_bitmap completed for file %s",commandArgs[1].c_str());
}

string getChunkSHA(string fileName, int chunkNo){

    string chunkSHA;

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);
    
    // reading here
    // check if file exist
    if(fileInfoMap.find(fileName) != fileInfoMap.end()){
        FileInfo *fileInfo = fileInfoMap[fileName];
        chunkSHA = fileInfo->getChunkSHA(chunkNo);
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

    return chunkSHA;

}

long long getFileSize(string fileName){

    long long fileSize;

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);
    
    // reading here
    // check if file exist
    FileInfo *fileInfo = fileInfoMap[fileName];
    fileSize = fileInfo->getFileSize();

    sleep(0.1);
    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount--;

    if (fileInfoMapReadersCount == 0) {
        sem_post(&fileInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    return fileSize;

}

void serveDownloadChunk(string command){

    LOGGER::DEBUG("perform download_chunk start");

    // [download_chunk, filename, chunkNo, peer_sd]
    vector<string> commandArgs = parseString(command,' ');
    string fileName = commandArgs[1];
    int peer_sd = atoi(commandArgs[3].c_str());
    int chunkNo = atoi(commandArgs[2].c_str());

    LOGGER::DEBUG("serving download_chunk for file %s and chunk: %d",fileName.c_str(),chunkNo);

    vector<string> response(2);

    // check is user logged in and is file shareable
    if(isUserLoggedIn){
        if(isFileShareable(fileName)){
            response[0] = "true";
            response[1] = "chunk sha for chunk no : " + to_string(chunkNo) + " found!!";
            response.push_back(getChunkSHA(fileName,chunkNo));
            LOGGER::INFO("%s chunk sha found for chunk no : %d",fileName.c_str(),chunkNo);
        }else{
            response[0] = "false";
            response[1] = fileName + " requested file is not shareable!!";
            LOGGER::ERROR("%s requested file is not shareable!!", fileName.c_str());   
        }
    }else{
        response[0] = "false";
        response[1] = "client is offline!!";
        LOGGER::ERROR("client is offline");
    }

    string reply = getString(response);
    send(peer_sd, reply.c_str(), reply.size(), 0);

    // read ack from client
    char clientRes[MAX_BUFFER_SIZE] = {0};
    read(peer_sd, clientRes,sizeof(clientRes));
    LOGGER::INFO("Response from client %s", clientRes);

    if(response[0] == "true"){

        // send chunkData from file
        string filePath = getFilePath(fileName);
        long long fileSize = getFileSize(fileName);

        long long totalChunks = ceil((float)fileSize / CHUNKSIZE);
        LOGGER::INFO("Total chunks : %lld",fileSize);

        // pread is used to read the data from file
        int fd = open(filePath.c_str(),O_RDONLY);
        if (fd ==-1) 
        { 
            // print which type of error have in a code 
            LOGGER::ERROR("Error Number % d", errno); 
                    
            // print program detail "Success or failure" 
            perror("Program");    
            return;             
        }
        size_t cs = CHUNKSIZE;
        off_t offset = chunkNo * CHUNKSIZE;
        char chunkData[CHUNKSIZE + 1] = {0};

        long long dataToBeRead = 0;
        if(chunkNo == totalChunks - 1){
            dataToBeRead = fileSize % CHUNKSIZE;
        }
        else{
            dataToBeRead = CHUNKSIZE;
        }

        long long bytesRead = pread(fd, chunkData,dataToBeRead,offset);
        close(fd);

        LOGGER::INFO("%lld bytes read and sent for chunk no %d",bytesRead,chunkNo);
        
        // send chunk data to client
        send(peer_sd,chunkData,sizeof(chunkData),0);

    }

    LOGGER::DEBUG("serving download_chunk completed for file %s and chunk: %d",fileName.c_str(),chunkNo);

}

void* performPeerTasks(void* param){
    
    LOGGER::DEBUG("perform peer tasks start");

    int peer_sd = *(int* )param;

    LOGGER::INFO("peer_sd %d",peer_sd);

    // read command form client
    char command[MAX_BUFFER_SIZE] = {0};
    int readR = read(peer_sd,command, sizeof(command));

    LOGGER::INFO("readR value : %d", readR);
    printf("\ncommand recieved : %s", command);

    vector<string> commandArgs = parseString(command,' ');
    if(commandArgs[0] == "download_bitmap"){
        LOGGER::INFO("download_file detected at client server");
        // [download_bitmap, filename]
        vector<string> data{commandArgs[0],commandArgs[1],to_string(peer_sd)};
        string info = getString(data,' ');
        serveDownloadChunkBitMapServer(info);
    }
    else if(commandArgs[0] == "download_chunk"){
        // [download_chunk, fileName, chunkNo]
        LOGGER::INFO("download_chunk detected at server");
        vector<string> data{commandArgs[0],commandArgs[1],commandArgs[2],to_string(peer_sd)};
        string info = getString(data,' ');
        serveDownloadChunk(info);
    }else{
        LOGGER::INFO("Invalid command!!");
    }

    LOGGER::DEBUG("perform peer tasks ended..");

    // pthread_exit(NULL);

}
