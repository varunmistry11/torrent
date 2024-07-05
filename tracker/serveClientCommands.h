
void serveCreateUser
(int client_sd,string commandInfo){

    string userid,password;

    // [create_user, userid, password]
    vector<string> args = parseString(commandInfo,' ');
    userid = args[1];
    password = args[2];

    LOGGER::DEBUG("%s serving create_user request",userid.c_str());

    vector<string> response(2);

    if(isUserExists(userid)){
        response[0] = "false";
        response[1] = "user already exists for these userid";
        LOGGER::ERROR("%s user already exist", userid.c_str());
    }else{
        addNewUser(userid,password);
        response[0] = "true";
        response[1] = "user created successfully";
        LOGGER::SUCCESS("%s user created successfully", userid.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving create_user completed", userid.c_str());

}

void serveCreateGroup
(int client_sd, string commandInfo){

    // [create_group, groupid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[1];
    string loggedInUserId = args[2];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving create_group started",grp_user.c_str());

    vector<string> response(2);

    if(isGroupExists(groupid)){
        response[0] = "false";
        response[1] = "group with same name already exists!!";
        LOGGER::ERROR("%s group with same name already exists!!",grp_user.c_str());
    }

    else{
        createNewGroup(groupid,loggedInUserId);
        response[0] = "true";
        response[1] = "group created successfully...";
        LOGGER::SUCCESS("%s group created successfully...",grp_user.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving create_group completed",grp_user.c_str());

}

void serveJoinGroup
(int client_sd, string commandInfo){

    // [join_group, groupid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[1];
    string loggedInUserId = args[2];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving join_group request",grp_user.c_str());
    
    bool isGroupPresent = isGroupExists(groupid);
    vector<string> response(2);

    if(isGroupPresent){
        // check is user already present in the pending list
        if(!isUserPresentInPendingList(groupid,loggedInUserId)){
            addUserToPendingRequestList(groupid,loggedInUserId);
            response[0] = "true";
            response[1] = "user added successfully to pending list if not already a member";
            LOGGER::SUCCESS("%s user added successfully to pending list",grp_user.c_str());
        }else{
            response[0] = "false";
            response[1] = "user already present in pending list";
            LOGGER::ERROR("%s user already present in pending list",grp_user.c_str());
        }
    }
    else{
        response[0] = "false";
        response[1] = "no group exist with " + groupid;
        LOGGER::ERROR("%s no group exist with",grp_user.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving join_group completed", grp_user.c_str());

}

void serveListPendingRequest
(int client_sd, string commandInfo){

    // [list_requests,groupid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    LOGGER::INFO("serve list_requests %s", commandInfo.c_str());

    string groupid = args[1];
    string loggedInUserId = args[2];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving ListPendingRequest",grp_user.c_str());
    
    vector<string> response(2);

    if(isGroupExists(groupid)){
        if(isLoggedInUserOwner(groupid,loggedInUserId)){
            vector<string> pendingRequest = getListOfPendingRequest(groupid);
            response[0] = "true";
            if(pendingRequest.size() == 0){
                response[1] = "no pending requests available for groupid=> " + groupid;
                LOGGER::SUCCESS("%s no pending requests available",grp_user.c_str());
            }else{
                response[1] = "Pending requests found for groupid=> " + groupid;
                string res = getString(pendingRequest);
                response.push_back(res);
                LOGGER::SUCCESS("%s pending requests found %s",grp_user.c_str());
            }
        }else{
            response[0] = "false";
            response[1] = "you are not the owner of group " + groupid;
            LOGGER::ERROR("%s logged in user is not the owner of group",grp_user.c_str());
        }
    }else{
        response[0] = "false";
        response[1] = "no group exist with groupid=> " + groupid;
        LOGGER::ERROR("%s no group exist with groupid=> %s",grp_user.c_str(),groupid);
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving listPendingRequest completed", grp_user.c_str());

}

void serveAcceptRequest
(int client_sd, string commandInfo){

    // [accept_request, groupid, userid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[1];
    string userid = args[2];
    string loggedInUserId = args[3];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s Serving accept_request for",grp_user.c_str());

    // check if group exist
    vector<string> response(2);

    if(isGroupExists(groupid)){

        if(isLoggedInUserOwner(groupid,loggedInUserId)){
            if(updatePendingRequestList(groupid,userid)){
                response[0] = "true";
                response[1] = "Request accepted and user added to group";
                LOGGER::SUCCESS("%s Request accpeted for %s user "
                    ,grp_user.c_str(),userid.c_str());
            }else{
                response[0] = "false";
                response[1] = "User id is not present in pending list";
                LOGGER::ERROR("%s User id is not present in pending list",grp_user.c_str());
            }
        }else{
            response[0] = "false";
            response[1] = "You are not the owner of group %s" + groupid;
            LOGGER::ERROR("%s Logged in user is not the owner",grp_user.c_str());
        }

    }
    else{
        response[0] = "false";
        response[1] = "No group exists with " + groupid;
        LOGGER::ERROR("%s No group exists", grp_user.c_str());
    }
    
    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s Completed accept_request", grp_user.c_str());

}

void serveListAllGroups
(int client_sd, string commandInfo){

    // [list_groups, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string loggedInUserId = args[1];

    LOGGER::DEBUG("%s serving list all groups for",loggedInUserId.c_str());

    vector<string> groups = getListOfAllGroups();
    vector<string> response(2);
    
    response[0] = "true";
    if(groups.size() == 0){
        response[1] = "no groups found " ;
        LOGGER::SUCCESS("%s no groups found ",loggedInUserId.c_str());
    }else{
        response[1] = "groups found " + loggedInUserId;
        string res = getString(groups);
        response.push_back(res);
        LOGGER::SUCCESS("%s groups found",loggedInUserId.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving list all groups completed", loggedInUserId.c_str());
}

void serveUploadFile
(int client_sd, string commandInfo){
    
    // [upload_file, fileName, groupid,fileSize,SHA1, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[2];
    string loggedInUserId = args[5];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving upload_file request",grp_user.c_str());

    vector<string> response(2);

    if(isGroupExists(groupid)){
        if(isClientMemberOfGroup(groupid,loggedInUserId)){
            if(isFileExists(groupid, args[1])){
                // if SHA is same for both the file of same name
                if(addNewSeeder(args[1], groupid, args[4],loggedInUserId)){
                    response[0] = "true";
                    response[1] = "new seeder added to the list if not present!!";
                    LOGGER::SUCCESS("%s new seeder added to the list if not present", grp_user.c_str());
                }else{
                    response[0] = "false";
                    response[1] = "file already exist with same name!!";
                    LOGGER::ERROR("%s file already exist with same name", grp_user.c_str());
                }
            }else{
                addNewFile(args[1],args[2],args[3],args[4],args[5]);
                addFileNameToGroup(args[1],groupid);
                response[0] = "true";
                response[1] = "file uploaded successfully to groupid=> " + groupid;
                LOGGER::SUCCESS("%s file uploaded successfully to group %s",
                grp_user.c_str(),groupid.c_str());
            }
        }else{
            response[0] = "false";
            response[1] = "User is not member of group=> " + groupid;
            LOGGER::ERROR("%s User is not member of group=> %s",
            grp_user.c_str(),groupid.c_str());
        }
    }
    else{
        response[0] = "false";
        response[1] = "no group exist with groupid=> " + groupid;
        LOGGER::ERROR("%s no group exist with groupid=> %s",grp_user.c_str(),groupid.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    // // if success then read the SHA1 of each chunk from client
    // if(response[0] == "success"){
    //     vector<string> chunkSHA = recieveChunkSHAFromClient(client_sd,args[3]);
    //     addAllChunkSHAToFile(groupid,args[1],chunkSHA);
    //     response[0] = "true";
    //     response[1] = "File uploaded successfully..";
    //     // send the reply to the client
    //     string reply = getString(response);
    //     send(client_sd, reply.c_str(),reply.size(),0);
    // }

    LOGGER::DEBUG("%s serving upload_file completed",grp_user.c_str());

}

/*
vector<string> getAllChunkSHA
(string groupid, string fileName){

    vector<string> allChunkSHA;

    LOGGER::DEBUG("get all chunk sha");

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
    FileInfo *fileInfo = fileInfoMap[key];
    allChunkSHA = fileInfo->getAllChunkSHA();

    // Lock the semaphore
    sem_wait(&fileInfoMap_RLOCK);
    fileInfoMapReadersCount--;

    if (fileInfoMapReadersCount == 0) {
        sem_post(&fileInfoMap_WLOCK);
    }
 
    // UNLock the semaphore
    sem_post(&fileInfoMap_RLOCK);

    return allChunkSHA;

}
*/

/*
void sendChunkWiseSHA1ToPeer
(int client_sd, string groupid, string filename){
    
    vector<string> allChunkSHA = getAllChunkSHA(groupid,filename);

    int total_chunks = allChunkSHA.size();

    for(int i = 0;i < total_chunks ;i++){
        // send each chunk sha to peer-client
        send(client_sd, allChunkSHA[i].c_str(), allChunkSHA.size(), 0);
    }

}
*/

void serveDownloadFile
(int client_sd, string commandInfo){
    
    // [download_file, groupid, filename, destPath, loggedInUserId] 
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[1];
    string loggedInUserId = args[4];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving download_file request",grp_user.c_str());

    vector<string> response(2);

    if(isGroupExists(groupid)){
        if(isClientMemberOfGroup(groupid,loggedInUserId)){
            if(isFileNameExistInGroup(groupid,args[2])){
                // [fileName,fileSize, SHA1, user1, ip1, port1,....]
                string fileMetaData = getFileMetaData(groupid,args[2]);
                response[0] = "true";
                response[1] = "fileMeta data found successfully for file=> " + args[2];
                response.push_back(fileMetaData);
                LOGGER::INFO("fileMata data: => %s",fileMetaData.c_str());
                LOGGER::SUCCESS("%s file meta data sent successfuly for file=> %s",
                        grp_user.c_str(),args[2].c_str());
            }else{
                response[0] = "false";
                response[1] = "no file exists with filename=> " + args[2];
                LOGGER::ERROR("%s no file exists with filename=> %s",
                    grp_user.c_str(),args[2].c_str());
            }
        }else{
            response[0] = "false";
            response[1] = "User is not member of group " + groupid;
            LOGGER::ERROR("%s User is not member of group %s",
            grp_user.c_str(),groupid.c_str());
        }
    }else{
        response[0] = "false";
        response[1] = "no group exist with groupid=> " + groupid;
        LOGGER::ERROR("%s no group exist with groupid=> ",grp_user.c_str(),groupid.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    // if(response[0] == "true"){
    //     sendChunkWiseSHA1ToPeer(client_sd,groupid, args[2]);
    //     response[0] = "true";
    //     response[1] = "Chunks sha sent successfully for file=> " + args[2];
    //     reply = getString(response);
    //     // send reply to client 
    //     send(client_sd, reply.c_str(), reply.size(),0);
    // }

    LOGGER::DEBUG("%s serving download_file completed",grp_user.c_str());

}

bool serveClientLogin
(int client_sd,string commandInfo){

    bool isSuccess = false;

    // [login,userid,password,ip,port]
    vector<string> args = parseString(commandInfo,' ');

    // does user exists
    string userid = args[1];
    string password = args[2];
    vector<string> response(2);

    LOGGER::DEBUG("%s serving login user ", userid.c_str());

    if(isUserExists(userid)){
        if(!isUserValid(userid,password)){
            response[0] = "false";
            response[1] = "invalid Userid or password";
            LOGGER::WARN("%s invalid userid or password", userid.c_str());
        }else{
            if(updateIpAndPort(userid,args[3],args[4])){
                response[0] = "false";
                response[1] = "already logged in from some other terminal!!";
                LOGGER::WARN("%s already loggedin in other terminal!!", userid.c_str());
            }else{
                isSuccess = true;
                response[0] = "true";
                response[1] = "user logged in successfully";
                LOGGER::SUCCESS("%s user logged in successfully", userid.c_str());
            }
        }
    }else{
        response[0] = "false";
        response[1] = "no user exist with these userid";
        LOGGER::ERROR("%s no user exist", userid.c_str());
    }
    
    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serve for login completed ", userid.c_str());

    return isSuccess;

}

void serveListFiles
(int client_sd, string commandInfo){

    // [list_files, groupid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[1];
    string loggedInUserId = args[2];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving list_files started",grp_user.c_str());

    vector<string> response(2);

    if(isGroupExists(groupid)){
        if(isClientMemberOfGroup(groupid,loggedInUserId)){
            vector<string> filenames = getFileNames(groupid);
            string files = getString(filenames);
            response[0] = "true";
            response[1] = "List of file names found ";
            response.push_back(files);
        }else{
            response[0] = "false";
            response[1] = "You are not the member of group " + groupid;
            LOGGER::ERROR("%s not a member of group %s",grp_user.c_str(), groupid.c_str());
        }
    }
    else{
        response[0] = "false";
        response[1] = "Group does not exist with name " + groupid;
        LOGGER::ERROR("%s Group does not exist with name %s",grp_user.c_str(),groupid.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving list_files completed",grp_user.c_str());

}

void serveLogout
(int client_sd,bool isClientLoggedIn,string loggedInUserId){
    
    LOGGER::DEBUG("serving logout command... for %s", loggedInUserId);
    
    vector<string> response(2);
    response[0] = "true";
    response[1] = "Logged out successfully..";

    updateClientInfoMap(loggedInUserId);

    string reply = getString(response);
    send(client_sd, reply.c_str(), reply.size(), 0);

    LOGGER::DEBUG("serving logout completed for %s", loggedInUserId);
}

void serveLeaveGroup
(int client_sd, string commandInfo){

    // [leave_group, groupid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[1];
    string loggedInUserId = args[2];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving leave_group started",grp_user.c_str());

    vector<string> response(2);
    // 1. is group exist
    // 2. is user member of group
    // 3. is owner

    if(isGroupExists(groupid)){
        if(isClientMemberOfGroup(groupid,loggedInUserId)){
            if(isLoggedInUserOwner(groupid,loggedInUserId)){
                performLeaveGroupForOwner(groupid,loggedInUserId);
            }else{
                performLeaveGroupForMember(groupid,loggedInUserId);
            }
            response[0] = "true";
            response[1] = "You are removed from the group " + groupid;
            LOGGER::SUCCESS("%s member %s removed from the group %s",grp_user.c_str(),
                loggedInUserId.c_str(), groupid.c_str());
        }else{
        response[0] = "false";
        response[1] = "You are not the member of group " + groupid;
        LOGGER::ERROR("%s not a member of group %s",grp_user.c_str(), groupid.c_str());
        }
    }
    else{
        response[0] = "false";
        response[1] = "Group does not exist with name " + groupid;
        LOGGER::ERROR("%s Group does not exist with name %s",grp_user.c_str(),groupid.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving leave_group completed",grp_user.c_str());

}

void serveStopShare(int client_sd, string commandInfo){

    // [stop_share, groupid, filename, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[1];
    string fileName = args[2];
    string loggedInUserId = args[3];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving stop_share started",grp_user.c_str());

    vector<string> response(2);
    // 1. is group exist
    // 2. is user member of group
    // 3. is file exist

    if(isGroupExists(groupid)){
        if(isClientMemberOfGroup(groupid,loggedInUserId)){
            if(isFileExists(groupid,fileName)){
                // if only one seeder then remove the file name from group 
                // remove key from fileInfoMap
                // else remove from the seeder list only 
                if(stopSharingTheFile(groupid,fileName,loggedInUserId)){
                    response[0] = "true";
                    response[1] = "file sharing stoppped for file " + fileName;
                    LOGGER::SUCCESS("%s file sharing stopped for file %s",grp_user.c_str(),fileName.c_str());
                }else{
                    response[0] = "false";
                    response[1] = "file " + fileName + " not shared by user: " + loggedInUserId;
                    LOGGER::SUCCESS("%s file %s cannot be stopped by user %s",grp_user.c_str(),
                        fileName.c_str(),loggedInUserId.c_str());
                }
            }else{
                response[0] = "false";
                response[1] = "No file shared with filename " + fileName;
                LOGGER::ERROR("%s file shared with filename ",grp_user.c_str(),fileName.c_str());
            }
        }else{
        response[0] = "false";
        response[1] = "You are not the member of group " + groupid;
        LOGGER::ERROR("%s not a member of group %s",grp_user.c_str(), groupid.c_str());
        }
    }
    else{
        response[0] = "false";
        response[1] = "Group does not exist with name " + groupid;
        LOGGER::ERROR("%s Group does not exist with name %s",grp_user.c_str(),groupid.c_str());
    }

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving stop_share completed",grp_user.c_str());

}

void serveAddLeecher(int client_sd, string commandInfo){

    // [add_leecher, fileName, groupid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[2];
    string filename = args[1];
    string loggedInUserId = args[3];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving add_leecher started",grp_user.c_str());

    vector<string> response(2);
    
    addLeecherInToFileInfo(groupid,filename,loggedInUserId);

    response[0] = "true";
    response[1] = "Leecher added successfully";

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving add_leecher completed",grp_user.c_str());

}

void serveAddSeeder(int client_sd,string commandInfo){

    // [add_leecher, fileName, groupid, loggedInUserId]
    vector<string> args = parseString(commandInfo,' ');

    string groupid = args[2];
    string filename = args[1];
    string loggedInUserId = args[3];

    string grp_user = groupid + " " + loggedInUserId;

    LOGGER::DEBUG("%s serving add_seeder started",grp_user.c_str());

    vector<string> response(2);
    
    addSeederInToFileInfo(groupid,filename,loggedInUserId);

    response[0] = "true";
    response[1] = "Seeder added successfully";

    // send response to client
    string reply = getString(response);
    send(client_sd, reply.c_str(),reply.size(),0);

    LOGGER::DEBUG("%s serving add_seeder completed",grp_user.c_str());

}