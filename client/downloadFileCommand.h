struct minHeapComp {
    constexpr bool operator()(
        pair<int, int> const& a,
        pair<int, int> const& b)
        const noexcept
    {
        return a.second < b.second;
    }
};

int createConnectionWithPeerServer
(string peerServerIpAdress,int peerServerPort){

    int client_sd,peer_sd;

    if ((client_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        LOGGER::ERROR("Socket creation error client-server");
        return -1;
    }
    
    struct sockaddr_in peerServerAdd;

    LOGGER::SUCCESS("Socket created from client server.");

    LOGGER::INFO("peer-server port => %d",peerServerPort);
    LOGGER::INFO("peer-server ip address =>%s",peerServerIpAdress.c_str());

    peerServerAdd.sin_family = AF_INET;
    peerServerAdd.sin_port = htons(peerServerPort);
    peerServerAdd.sin_addr.s_addr = inet_addr(peerServerIpAdress.c_str());
    
    if ((peer_sd = connect(client_sd, (struct sockaddr *)&peerServerAdd,
                                 sizeof(peerServerAdd))) < 0){
        LOGGER::ERROR("Connection Failed on ip : %s",peerServerIpAdress.c_str());
        return -1;
    }

    LOGGER::SUCCESS("Connected to peer server successfully.. ");

    return client_sd;
}

void performFileDownload
(vector<string> resp){

    // [status,message,fileName, fileSize, SHA1, user1, ip1, port1...]

    // send filename to peer-server and check that is peer online
    // if yes then start downloading
    //[download_file, filename]

    string fileName = resp[2];
    long long fileSize = atoll(resp[3].c_str());
    int size = resp.size();
    bool isFileTransferDone = false;
    bool isPeerServerFound = false;
    for(int i = 5;i < size;i+=3){

        string userid = resp[i];
        string peerIP = resp[i+1];
        int peerPort = atoi(resp[i+2].c_str());

        int peer_sd = createConnectionWithPeerServer(peerIP,peerPort);

        if(peer_sd == -1){
            LOGGER::ERROR("Connection failed with peerserver");
            return ;
        }

        // now send command info to peer server
        string command = "download_file" + ' ' + fileName;
        send(peer_sd, command.c_str(), command.size(), 0);

        // read response from peer server
        char response[MAX_BUFFER_SIZE] = {0};
        read(peer_sd, response, sizeof(response));

        vector<string> res = parseString(response,' ');

        if(res[0] == "true"){
            isPeerServerFound = true;
            receiveFile(fileName,peer_sd);
            break;
        }
    }

}

void receiveFile
(string fileName, int peer_sd){

    char buffer[CHUNKSIZE];
    int bytesRead;
    int fd = open(fileName.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

    if (fd == -1)
        perror("couldn't open file");

    while ((bytesRead = read(peer_sd, buffer, CHUNKSIZE)) > 0)
    {
        fflush(stdout);

        write(fd, buffer, bytesRead);
        bzero(buffer, sizeof(buffer));
    }

    if (bytesRead < 0)
    {
        printf("\n Read Error \n");
    }
    printf("\nComplete\n");

}

unordered_map<string,pair<string,int>> generateClientInfoMap(string fileMetaData){

    // map<userid,pair<ip,port>>
    unordered_map<string,pair<string,int>> clientInfoMap;
    // [status,message,fileName, fileSize, SHA1, user1, ip1, port1...]
    vector<string> info = parseString(fileMetaData,DELIMETER);
    for(int i = 5;i < info.size();i+=3){
        //[userid] = <ip,port>
        // if offline skip it
        if(info[i+1] != "$")
            clientInfoMap[info[i]] = make_pair(info[i+1],atoi(info[i+2].c_str()));
    }

    return clientInfoMap;

}

void processChunkBitMap
(vector<string> bitmap,unordered_map<int,vector<string>>& chunkInfoMap,
string userid,string fileName ){

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

    // [bool1, bool2,...]
    for(int i = 0;i < bitmap.size();i++){
        if(!fileInfo->isChunkAvailable(i) && bitmap[i] == "1");
            chunkInfoMap[i].push_back(userid);
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

}

vector<string> downloadBitMapFromPeer
(int client_sd,string fileName){

    LOGGER::INFO("download_bitmap started...");

    vector<string> commandArgs{"download_bitmap",fileName};
    string command = getString(commandArgs,' ');

    LOGGER::DEBUG("\ncommand sent to peer server %s", command);

    // send command to peer-server
    send(client_sd,command.c_str(),command.size(),0);

    // read reponse from the peer-server
    char response[MAX_BUFFER_SIZE] = {0};
    // [status, message , bool1, bool2, ....]
    read(client_sd, response, sizeof(response));

    vector<string> resp = parseString(response,DELIMETER);
    vector<string> bitmap;
    if(resp[0] == "true"){
        vector<string> tbitmap(resp.begin() + 2,resp.end());
        bitmap = tbitmap;
    }

    logTrackerResponse(response);

    LOGGER::INFO("download_bitmap completed...");

    return bitmap;
}

bool getChunkDataFromPeer
(int client_sd,string fileName,int chunkNo,string originalChunkSHAFromTracker,
string destPath,string groupid){

    long long fileSize = getFileSize(fileName);

    long long totalChunks = ceil((float)fileSize / CHUNKSIZE);
    LOGGER::INFO("Total chunks : %lld",fileSize);

    long long dataToBeRead = 0;
    if(chunkNo == totalChunks - 1){
        dataToBeRead = fileSize % CHUNKSIZE;
    }
    else{
        dataToBeRead = CHUNKSIZE;
    }

    // read chunkData from peer-server
    char chunkData[CHUNKSIZE + 1] = {0};

    long long totalBytesRead = 0;
    long long bytesRead = 0;

    char temp[CHUNKSIZE + 1] = {0};

    long long i = 0;

    while(1){

        bytesRead = read(client_sd,temp,dataToBeRead);

        for(long long j = 0;j < bytesRead; j++){
            chunkData[i++] = temp[j];
        }

        totalBytesRead += bytesRead;

        if(totalBytesRead >= dataToBeRead){
            break;
        }

        bzero(temp,sizeof(temp));
    }

    // calculate SHA for recieved chunk
    string newSHA = calHashofchunk(chunkData,dataToBeRead,1);

    LOGGER::INFO("\nold sha %s",originalChunkSHAFromTracker.c_str());
    LOGGER::INFO("\nnew sha %s",newSHA.c_str());

    if(newSHA == originalChunkSHAFromTracker){
       
        LOGGER::INFO("SHA matched for chunk %d",chunkNo);
        int fd = open(destPath.c_str(),O_WRONLY);

        if (fd ==-1) { 
            // print which type of error have in a code 
            LOGGER::ERROR("Error Number for file %s % d\n",destPath, errno); 
            
            // print program detail "Success or failure" 
            perror("Program");    
            return false;             
        }

        size_t cs = CHUNKSIZE;
        off_t offset = chunkNo * CHUNKSIZE;
        // On success, pread() returns the number of bytes read (a return of
        // zero indicates end of file) and pwrite() returns the number of
        // bytes written.
        // ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset);
        int bytesWritten = pwrite(fd,chunkData,dataToBeRead,offset);
        LOGGER::INFO("bytes written for chunk no %d => %d",chunkNo,bytesWritten);
        close(fd);
        
        // update to tracker on tracker_sd
        string cmd = "add_leecher";
        cmd +=  " " + fileName + " " + groupid;
        send(tracker_sd,cmd.c_str(),cmd.size(),0);

        char response[MAX_BUFFER_SIZE] = {0};
        read(tracker_sd,response, sizeof(response));

        // logTrackerResponse(response);

        // update the local fileInfo bitmap
        updateChunkBitMap(fileName, chunkNo);
        
        return true;
    }
    return false;
}

void downloadChunkFromPeer(int chunkNo,vector<string> peers,
unordered_map<string,pair<string,int>> clientInfoMap,
string fileName,string destPath,string groupid){

    LOGGER::DEBUG("downloadChunkFromPeer start..");

    // now go to each user and check if user is online if yes then
    // responses from peer
    // 1. connect with peer by sending [download_chunk,fileName,chunkNo]
    // 2. if fileName shareable and chunkNo found => read the SHA for that chunk
    // 3. read the chunkData
    // 4. set data into the file using pwrite

    int client_sd;

    int totalLen = peers.size();
    LOGGER::INFO("total length : %d", totalLen);
    // starting from the end as leechers are at the end of the list
    for(int i = totalLen-1;i >= 0; i--){
        string peer = peers[i];
        client_sd = createConnectionWithPeerServer(clientInfoMap[peer].first,
                clientInfoMap[peer].second);

        if(client_sd != -1){
            
            vector<string> data{"download_chunk",fileName,to_string(chunkNo)};
            string command = getString(data,' ');
            LOGGER::INFO("command to send for download chuk %s",command.c_str());
            
            // send command to the peer-server
            // [download_file, fileName, chunkNo]
            send(client_sd, command.c_str(), command.size(), 0);

            // read reponse from peer-server
            char response[MAX_BUFFER_SIZE] = {0};
            // [status, msg, SHA1_of_chunkNo]
            read(client_sd, response, sizeof(response));

            vector<string> resp = parseString(response,DELIMETER);
            if(resp[0] == "true"){

                string chunkSHA = resp[2];

                // send ack to peer-server
                string ack = "Send chunk data!! Sha recieved";
                send(client_sd, ack.c_str(),ack.size(),0);

                if(getChunkDataFromPeer(client_sd,fileName,chunkNo,chunkSHA,destPath,groupid))
                    return ;
            }
        }
     }
}

bool performDownloadFileChunkWise
(string fileMetaDataFromTracker,vector<string> commandArgs, long long fileSize,string groupid){

    //commandArgs= [download_file, groupid, filename, destPath]
    LOGGER::DEBUG("Perform download file chunk wise started.. ");
    // map<userid,pair<ip,port>>
    unordered_map<string,pair<string,int>> clientInfoMap;
    // map<chunkNo,[userid1,userid2,...]>
    unordered_map<int,vector<string>> chunkInfoMap;
    // minheap <pair<chunkNo,clientIdList.size()>>
    priority_queue<pair<int,int>, vector<pair<int,int>>, minHeapComp > chunkMinHeap;

    // make map of clientInfoMap
    clientInfoMap = generateClientInfoMap(fileMetaDataFromTracker);

    if(clientInfoMap.empty()){
        LOGGER::ERROR("No seeder or leecher avalibale online for file download");
        return false;
    }

    LOGGER::DEBUG("\nclientinfo map generated of size %d ",clientInfoMap.size());

    int client_sd;
    string fileName = commandArgs[2];
    string fileDestPath = commandArgs[3];
    // get chunk bit map from peer-server
    for(auto client : clientInfoMap){
        client_sd = createConnectionWithPeerServer(client.second.first,client.second.second);
        if(client_sd != -1){
            vector<string> bitmap = downloadBitMapFromPeer(client_sd,fileName);    
            if(!bitmap.empty()){
                processChunkBitMap(bitmap,chunkInfoMap,client.first,fileName);
                LOGGER::INFO("bitmap found from %s peer-server",client.first.c_str());
            }else{
                LOGGER::INFO("bitmap not found from %s peer-server",client.first.c_str());
            }
        }else{
            LOGGER::ERROR("client connection not established to peer server");
        }
    }

    LOGGER::INFO("Info making min heap...");

    // make min-heap using the chunkInfoMap
    // <chunkNo,userIds.size()>
    for(auto it : chunkInfoMap){
        chunkMinHeap.push({it.first,it.second.size()});
    }

    LOGGER::INFO("Info making min heap completed");

    // now apply piece selection algorithm
    // pick rarest first-> top from the minHeap
    // make file at download_file destPath and initializa it with the required
    // fileSize and \0 as data
    LOGGER::INFO("Creating file");
    string finalPath = fileDestPath + "/" + fileName;
    FILE* fp = fopen(finalPath.c_str(),"w");
    ftruncate(fileno(fp),fileSize);
    fclose(fp);
    LOGGER::INFO("empty file created");

    // implement multi-threading here
    while(!chunkMinHeap.empty()){
        //<chunkNo, userIds.size()>
        pair<int,int> p = chunkMinHeap.top();
        chunkMinHeap.pop();
        downloadChunkFromPeer(p.first,chunkInfoMap[p.first],clientInfoMap,fileName,finalPath,groupid);
    }

    // join all the threads when formed
    LOGGER::DEBUG("Perform download file chunk wise ended.. ");

}

void serveDownloadFile
(int client_sd, string command){

    LOGGER::DEBUG("Serving download_file command");

    // [download_file, groupid, filename, destPath]
    vector<string> commandArgs = parseString(command,' ');

    string destPath = commandArgs[3];
    string fileName = commandArgs[2];
    string groupid = commandArgs[1];
    char buf[MAX_BUFFER_SIZE];
    char *res = realpath(destPath.c_str(), buf);
    if (!res){
        LOGGER::ERROR("no such file exists at this dest path");
        return;
    }

    // send command information to tracker
    // [download_file, groupid, filename, destPath]
    send(client_sd, command.c_str(), command.size(),0);

    char response[FILEINFOBUFFERSIZE] = {0};
    // read the response from tracker
    // [status,message,fileName, fileSize, SHA1, user1, ip1, port1...]
    // also take sha for each chunk of file
    read(client_sd, response, sizeof(response));

    string fileMetaDataFromTracker = response;

    vector<string> resp = parseString(fileMetaDataFromTracker,'|');

    LOGGER::INFO("\n meta data: %s ", response);

    if(resp[0] == "true") { 
        // add file to fileinfo
        char buf[MAX_BUFFER_SIZE];
        string destPath = commandArgs[3];
        char *res = realpath(destPath.c_str(), buf);
        if (!res){
            LOGGER::ERROR("no such file exists at this path");
            return;
        }

        destPath = res;
        destPath +=  "/" + commandArgs[2];
        addNewFile(commandArgs[2],resp[3],resp[4],destPath,false);
        if(performDownloadFileChunkWise
        (fileMetaDataFromTracker,commandArgs,atoll(resp[3].c_str()),groupid)){
            
            string originalFileSHA = resp[4];
            char path[destPath.size()] = {0};
            strcpy(path, destPath.c_str());
            string newFileSHA = getSHA1OfCompleteFile(path);
            if(isAllChunksRecieved(fileName) && (originalFileSHA == newFileSHA)){
                // send command to tracker for adding seeder
                string cmd = "add_seeder";
                cmd += " " + fileName + " " + groupid;
                LOGGER::SUCCESS("File downloaded successfully...");
                send(tracker_sd,cmd.c_str(),cmd.size(),0);

                // read response from tracker
                char buff[MAX_BUFFER_SIZE] = {0};
                read(tracker_sd,buff,sizeof(buff));

                logTrackerResponse(buff);
            }
            else{
                LOGGER::ERROR("File corrupted successfully...");
            }
        }else{
            LOGGER::ERROR("File cannot be downloaded...");
        }
    }
    
    logTrackerResponse(response);

    LOGGER::DEBUG("serving download file completed...");
}
