
bool isClientMemberOfGroup
(string groupid,string userid){

    LOGGER::INFO("is client member of group");

    bool isMember = false;

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    Group *group = groupInfoMap[groupid];
    LOGGER::INFO("%s", group->getGroupId().c_str());
    isMember = group->isUserMemberOfGroup(userid);
    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return isMember;

}

bool isUserPresentInPendingList
(string groupid, string userid){
    
    bool isExists = false;

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    vector<string> pendingRequest = groupInfoMap[groupid]->getListOfPendingRequest();
    for(string id : pendingRequest){
        if(id == userid){
            isExists = true;
        }
    }

    // sleep(2);

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return isExists;

}

bool isLoggedInUserOwner
(string groupid,string loggedInUserId){

    bool isOwner = false;

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    Group *group = groupInfoMap[groupid];
    if(loggedInUserId == group->getOwnerUserId()){
        isOwner = true;
    }

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return isOwner;
}

vector<string> getListOfPendingRequest
(string groupid){

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    Group *group = groupInfoMap[groupid];
    vector<string> pendingRequest = group->getListOfPendingRequest();

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return pendingRequest;

}

vector<string> getListOfAllGroups(){

    vector<string> groupIds;

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    for(auto itr : groupInfoMap){
        groupIds.push_back(itr.first);
    }

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return groupIds;

}

void addUserToPendingRequestList
(string groupid, string userid){

    // take lock
    sem_wait(&groupInfoMap_WLOCK);

    Group *group = groupInfoMap[groupid];
    group->addNewRequest(userid);
    groupInfoMap[groupid] = group;

    updateGroupInfoFile();

    // release lock
    sem_post(&groupInfoMap_WLOCK);
}

bool updatePendingRequestList
(string groupid, string userid){

    bool isFound = false;

    // take lock
    sem_wait(&groupInfoMap_WLOCK);

    // updating group

    Group *group = groupInfoMap[groupid];
    vector<string> pendingList = group->getListOfPendingRequest();

    for(int i = 0;i < pendingList.size(); i++){
        if(pendingList[i] == userid){
            pendingList.erase(pendingList.begin() + i);
            isFound = true;
            break;
        }
    }

    // add user to member list
    if(isFound){
        group->addNewMember(userid);
    }

    group->setListPendingRequests(pendingList);
    groupInfoMap[groupid] = group;

    updateGroupInfoFile();

    // release lock
    sem_post(&groupInfoMap_WLOCK);

    return isFound;
}

bool isGroupExists
(string groupid){

    bool isExists = false;

    LOGGER::DEBUG("is group exists");

     // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    if(groupInfoMap.find(groupid) != groupInfoMap.end()){
        isExists = true;
    }

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return isExists;

}

void createNewGroup
(string groupid,string userid){

    // take lock
    sem_wait(&groupInfoMap_WLOCK);

    // writing here
    Group *group = new Group(groupid,userid);
    groupInfoMap[groupid] = group;

    updateGroupInfoFile();

    // release lock
    sem_post(&groupInfoMap_WLOCK);

}

bool isUserValid
(string userid, string password){

    bool isValid = false;

     // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount++;

    if(clientInfoMapReadersCount == 1){
        sem_wait(&clientInfoMap_WLOCK);
    }

    // reading here

    ClientInfo *client = clientInfoMap[userid];

    if(client->getPassword() == password){
        isValid = true;
    }

    sleep(2);
    // Unlock the semaphore
    sem_post(&clientInfoMap_RLOCK);

    // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount--;

    if (clientInfoMapReadersCount == 0) {
        sem_post(&clientInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&clientInfoMap_RLOCK);

    return isValid;

}

bool isUserExists
(string userid){

    bool isExists = false;

    // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount++;

    if(clientInfoMapReadersCount == 1){
        sem_wait(&clientInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&clientInfoMap_RLOCK);
    
    // reading here
    if(clientInfoMap.find(userid) != clientInfoMap.end()){
        isExists = true;
    }

    sleep(0.3);
    // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount--;

    if (clientInfoMapReadersCount == 0) {
        sem_post(&clientInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&clientInfoMap_RLOCK);

    return isExists;
}

void addNewUser
(string userid, string password){

    // Lock the semaphore
    sem_wait(&clientInfoMap_WLOCK);
 
    // writing here
    sleep(0.3);

    ClientInfo *client = new ClientInfo(userid,password);
    clientInfoMap[userid] = client;

    addNewUserFile(userid,password);

    // Unlock the semaphore
    sem_post(&clientInfoMap_WLOCK);

}

bool updateIpAndPort
(string userid,string ip,string port){
    
    bool isAlreadyLoggedIn = false;

    // Lock the semaphore
    sem_wait(&clientInfoMap_WLOCK);
 
    // writing here
    ClientInfo *client = clientInfoMap[userid];
    if(client->getIP() == "$"){
        client->setPort(atoi(port.c_str()));
        client->setIP(ip);
        client->setIsLoggedIn(true);
        clientInfoMap[userid] = client;

        updateClientInfoFile();

    }else{
        isAlreadyLoggedIn = true;
    }
    // Unlock the semaphore
    sem_post(&clientInfoMap_WLOCK);

    return isAlreadyLoggedIn;

}

void addNewFile
(string fileName, string groupid, string fileSize,
    string SHA1, string loggedInUserId){

    LOGGER::INFO("adding new file");

    // Lock the semaphore
    sem_wait(&fileInfoMap_WLOCK);
 
    // writing here

    FileInfo *fileInfo = new 
    FileInfo(fileName, atoll(fileSize.c_str()),SHA1,loggedInUserId);

    // generate key 
    string key = groupid + "|" + fileName;
    fileInfoMap[key] = fileInfo;

    updateFileInfoFile();

    sleep(0.1);
    // Unlock the semaphore
    sem_post(&fileInfoMap_WLOCK);

}

void addFileNameToGroup
(string fileName, string groupid){

    LOGGER::INFO("add file name to group");

     // take lock
    sem_wait(&groupInfoMap_WLOCK);

    // writing here
    Group *group = groupInfoMap[groupid];
    group->addFileName(fileName);
    groupInfoMap[groupid] = group;

    updateGroupInfoFile();

    // release lock
    sem_post(&groupInfoMap_WLOCK);

}

bool isFileNameExistInGroup
(string groupid, string fileName){

    LOGGER::INFO("is file name exist");

    bool isExists = false;

     // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    Group *group = groupInfoMap[groupid];
    isExists = group->isFileNameExist(fileName);


    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return isExists;

}

vector<string> getClientsInfo
(vector<string> userids){

    vector<string> info;

    // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount++;

    if(clientInfoMapReadersCount == 1){
        sem_wait(&clientInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&clientInfoMap_RLOCK);
    
    // reading here
    for(string userid : userids){
        ClientInfo *clientInfo = clientInfoMap[userid];
        info.push_back(clientInfo->getUserId());
        info.push_back(clientInfo->getIP());
        info.push_back(to_string(clientInfo->getPort()));
    }
    
    sleep(0.1);
    // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount--;

    if (clientInfoMapReadersCount == 0) {
        sem_post(&clientInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&clientInfoMap_RLOCK);

    return info;

}

string getFileMetaData
(string groupid, string fileName){

    string fileData = "";

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // reading here
    // generate key
    string key = groupid + DELIMETER + fileName;
    FileInfo *fileInfo = fileInfoMap[key];
    vector<string> metaData;
    metaData.push_back(fileInfo->getFileName());
    metaData.push_back(to_string(fileInfo->getFileSize()));
    metaData.push_back(fileInfo->getSHA_1());

    vector<string> seederList = getClientsInfo(fileInfo->getSeederList());
    vector<string> leecherList = getClientsInfo(fileInfo->getLeecherList());
    //appending elements of vector v2 to vector v1
    metaData.insert(metaData.end(), seederList.begin(), seederList.end());
    metaData.insert(metaData.end(), leecherList.begin(), leecherList.end());

    fileData = getString(metaData);

    sleep(0.1);
    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount--;

    if (fileInfoMapReadersCount == 0) {
        sem_post(&fileInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    return fileData;
}


bool isClientOnline(string userid){

    bool isLoggedIn = false;

     // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount++;

    if(clientInfoMapReadersCount == 1){
        sem_wait(&clientInfoMap_WLOCK);
    }

    // reading here
    ClientInfo *client = clientInfoMap[userid];
    isLoggedIn = client->getIsLoggedIn();

    // sleep(2);
    // Unlock the semaphore
    sem_post(&clientInfoMap_RLOCK);

    // Lock the semaphore
    sem_wait(&clientInfoMap_RLOCK);
    clientInfoMapReadersCount--;

    if (clientInfoMapReadersCount == 0) {
        sem_post(&clientInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&clientInfoMap_RLOCK);

    return isLoggedIn;

}

vector<string> getSeederList(string groupid, string fileName){
    
    vector<string> seeders;

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    // reading here
    // generate key
    string key = groupid + DELIMETER + fileName;
    if(fileInfoMap.find(key) != fileInfoMap.end()){
        seeders = fileInfoMap[key]->getSeederList();
    }

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount--;

    if (fileInfoMapReadersCount == 0) {
        sem_post(&fileInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    return seeders;
}

vector<string> getFileNames
(string groupid){

    vector<string> files;

    LOGGER::DEBUG("is group exists");

     // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount++;

    if(groupInfoMapReadersCount == 1){
        sem_wait(&groupInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    // reading here
    Group *group = groupInfoMap[groupid];
    vector<string> fileNames = group->getFileNames();

    for(string file : fileNames){
        vector<string> users = getSeederList(groupid,file);
        for(string user : users){
            if(isClientOnline(user)){
                files.push_back(file);
                break;
            }
        }
    }

    // Lock the semaphore
    sem_wait(&groupInfoMap_RLOCK);
    groupInfoMapReadersCount--;

    if (groupInfoMapReadersCount == 0) {
        sem_post(&groupInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&groupInfoMap_RLOCK);

    return files;
}

void removeFilesFromFileInfoMap
(string groupid,vector<string> files){

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing here
    for(string file : files){
        // generate key
        string key = groupid + "|" + file;
        fileInfoMap.erase(key);
    }

    updateFileInfoFile();

    // release lock
    sem_post(&fileInfoMap_WLOCK);

}

vector<string> removeFilesSharedByMemberFromFileInfo
(string groupid, vector<string> files, string userid){

    vector<string> fileNames;

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing here
    for(string file : files){
        // generate key
        string key = groupid + DELIMETER + file;
        FileInfo *fileInfo = fileInfoMap[key];
        vector<string> seeders = fileInfo->getSeederList();
        int pos = -1;
        for(int i = 0;i < seeders.size();i++){
            if(userid == seeders[i]){
                pos = i;
                break;
            }
        }
        if(pos != -1){
            // only one seeder of file also remove from the group
            if(seeders.size() == 1){
                fileInfoMap.erase(key);
                fileNames.push_back(file);
            }else{
                fileInfo->removeSeeder(userid,pos);
                fileInfoMap[key] = fileInfo;
            }
        }
    }

    updateFileInfoFile();

    // release lock
    sem_post(&fileInfoMap_WLOCK);

    return fileNames;
}

void performLeaveGroupForOwner
(string groupid, string userid){

    vector<string> files;

    // take lock
    sem_wait(&groupInfoMap_WLOCK);

    // writing here
    Group *group = groupInfoMap[groupid];
    vector<string> members = group->getMemberUserIds();
    files = group->getFileNames();
    // if only one member is there then delete the group itself and 
    // remove the files from the fileinfo map
    if(members.size() == 1){
        removeFilesFromFileInfoMap(groupid,files);
        groupInfoMap.erase(groupid);
    }else{
        // find the member index and remove member
        // now remove all the files that are only shared by the member
        // make next memeber owner
        group->removeMemberFromMemberIds(userid);
        string newOwnerUserId = group->getFirstMember();
        group->setOwnerUserId(newOwnerUserId);
        vector<string> fileNames = removeFilesSharedByMemberFromFileInfo(groupid,files,userid);
        group->removeFilesSharedByMember(fileNames);
        groupInfoMap[groupid] = group;
    }

    updateGroupInfoFile();

    // release lock
    sem_post(&groupInfoMap_WLOCK);

}

void performLeaveGroupForMember
(string groupid, string userid){

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing here
    Group *group = groupInfoMap[groupid];
    group->removeMemberFromMemberIds(userid);
    vector<string> files = group->getFileNames();
    vector<string> fileNames = removeFilesSharedByMemberFromFileInfo(groupid,files,userid);
    group->removeFilesSharedByMember(fileNames);
    groupInfoMap[groupid] = group;

    updateGroupInfoFile();

    // release lock
    sem_post(&fileInfoMap_WLOCK);

}

void removeFileNameFromGroup(string groupid,string fileName){
    
    LOGGER::INFO("remove filename from group");

    // take lock
    sem_wait(&groupInfoMap_WLOCK);

    Group *group = groupInfoMap[groupid];
    group->removeFile(fileName);
    groupInfoMap[groupid] = group;

    updateGroupInfoFile();

    // release lock
    sem_post(&groupInfoMap_WLOCK);

    LOGGER::INFO("remove filename from group completed");
}


bool stopSharingTheFile(string groupid, string fileName,string userid){

    bool isStopped = false;

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing here
    string key = groupid + "|" + fileName;
    if(fileInfoMap.find(key) != fileInfoMap.end()){
        LOGGER::INFO("key found");
        FileInfo *fileInfo = fileInfoMap[key];
        vector<string> seeders = fileInfo->getSeederList();
        if(seeders.size() == 1){
            LOGGER::INFO("One seeder found");
            if(seeders[0] == userid){
                LOGGER::INFO("seeder found");
                fileInfoMap.erase(key);
                removeFileNameFromGroup(groupid,fileName);
                isStopped = true;
            }
        }else{
            LOGGER::INFO("more than One seeder found");
            isStopped = fileInfo->removeSeederFromList(userid);
            fileInfoMap[key] = fileInfo;
        }
    }

    updateFileInfoFile();
    
    // release lock
    sem_post(&fileInfoMap_WLOCK);

    return isStopped;

    LOGGER::INFO("stop share function completed");

}


void addLeecherInToFileInfo(string groupid, string filename,string userid){

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing here
    string key = groupid + DELIMETER + filename;
    if(fileInfoMap.find(key) != fileInfoMap.end()){
        FileInfo *file = fileInfoMap[key];
        if(!file->isLeecherPresent(userid)){
            file->addInToLeecherList(userid);
        }
        fileInfoMap[key] = file;
    }

    // release lock
    sem_post(&fileInfoMap_WLOCK);

}

void addSeederInToFileInfo(string groupid, string filename, string userid){

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing here
    string key = groupid + DELIMETER + filename;
    if(fileInfoMap.find(key) != fileInfoMap.end()){
        FileInfo *file = fileInfoMap[key];
        if(!file->isLeecherPresent(userid)){
            file->removeLeecherFromList(userid);
        }
        file->addInToSeederList(userid);
        fileInfoMap[key] = file;
    }

    sem_post(&fileInfoMap_WLOCK);

}


bool isFileExists
(string groupid, string fileName){

    bool isExists = false;

    LOGGER::DEBUG("is file exists");

     // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount++;

    if(fileInfoMapReadersCount == 1){
        sem_wait(&fileInfoMap_WLOCK);
    }

    // Unlock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    // reading here
    // generate key
    string key = groupid + DELIMETER + fileName;
    if(fileInfoMap.find(key) != fileInfoMap.end()){
        isExists = true;
    }

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

bool addNewSeeder
(string fileName, string groupid, string SHA1,string loggedInUserId){

    bool isadded = false;

    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing
    // geneare key
    string key = groupid + DELIMETER + fileName;
    FileInfo *fileinfo = fileInfoMap[key];
    if(fileinfo->getSHA_1() == SHA1){
        fileinfo->addInToSeederList(loggedInUserId);
        isadded = true;
    }

    updateFileInfoFile();

    // release lock
    sem_post(&fileInfoMap_WLOCK);

    return isadded;

}

void updateClientInfoMap
(string loggedInUserId){

    // take lock
    sem_wait(&clientInfoMap_WLOCK);

    // writing here
    ClientInfo *client = clientInfoMap[loggedInUserId];
    client->setIP("$");
    client->setPort(-1);
    client->setIsLoggedIn(false);
    clientInfoMap[loggedInUserId] = client;

    updateClientInfoFile();

    // unlock 
    sem_post(&clientInfoMap_WLOCK);

}
