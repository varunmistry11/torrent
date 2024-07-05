
void serveCreateUser
(int client_sd,string command){

    LOGGER::DEBUG("serving create user request");

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};

    // read the response from tracker
    read(client_sd, response, sizeof(response));

    logTrackerResponse(response);

    LOGGER::DEBUG("completed create_user...");
}

void serveLogin
(int client_sd, string command){

    LOGGER::DEBUG("serving login request");

    // [login, userid, password, ip, port]
    string commandData = command + " " + serverIPAddress + 
    " " + to_string(serverPort);

    vector<string> parsedData = parseString(commandData,' ');

    // send command information to tracker
    send(client_sd, commandData.c_str(), commandData.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    vector<string> res = parseString(response,'|');

    if(res[0] == "true"){
        userIdOfLoggedInUser = parsedData[1];
        isUserLoggedIn = true;
    } 

    logTrackerResponse(response);    

    LOGGER::DEBUG("serving login completed");

}

void serveCreateGroup
(int client_sd, string command){

    LOGGER::DEBUG(" serving create_group request");

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    logTrackerResponse(response);

    LOGGER::DEBUG(" serving create_group completed");

}

void serveJoinGroup
(int client_sd, string command){

    LOGGER::DEBUG("serving join_group request");

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    logTrackerResponse(response);

    LOGGER::DEBUG("serving join_group completed");
}

void serveListPendingRequest
(int client_sd, string command){

    LOGGER::DEBUG("serving list_pending_requests");

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    vector<string> res = parseString(response,'|');

    if(res[0] == "true") { 
        cout << endl << "pending request list :" << endl;
        for(int i = 2;i < res.size(); i++){
            cout << res[i] << endl;
        }
    }
    
    logTrackerResponse(response);

    LOGGER::DEBUG("serving list_pending_requests completed");

}

void serveAcceptRequest
(int client_sd, string command){
    
    LOGGER::DEBUG("serving accept_request request ");
    
    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    logTrackerResponse(response);

    LOGGER::DEBUG("serving accept_request completed");

}

void serveListAllGroups
(int client_sd, string command){

    LOGGER::DEBUG("serving list all groups request");

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    vector<string> res = parseString(response,'|');

    logTrackerResponse(response);
    cout << endl;

    if(res[0] == "true") { 
        cout << endl << "list of all groups :" << endl;
        for(int i = 2;i < res.size(); i++){
            cout << res[i] << endl;
        }
    }

    LOGGER::DEBUG("serving list all groups completed");
}

int sha1
( const char * name, unsigned char * out )
{
    FILE * pf;
    unsigned char buf[ CHUNKSIZE ];
    SHA_CTX ctxt;

    pf = fopen( name, "rb" );

    if( !pf )
        return -1;

    SHA1_Init( &ctxt );

    while(1)
    {
        size_t len;

        len = fread( buf, 1, CHUNKSIZE, pf );

        if( len <= 0 )
            break;

        SHA1_Update( &ctxt, buf, len );
    }

    fclose(pf);

    SHA1_Final( out, &ctxt );

    return 0;
}

void bin2hex
( unsigned char * src, int len, char * hex )
{
    int i, j;

    for( i = 0, j = 0; i < len; i++, j+=2 )
        sprintf( &hex[j], "%02x", src[i] );
}

string getSHA1OfCompleteFile
(char* filePath){
    
    string fileSHA1 = "";
    unsigned char digest[ SHA_DIGEST_LENGTH ];
    char str[ (SHA_DIGEST_LENGTH * 2) + 1 ];

    if( sha1( filePath, digest ) )
    {
        LOGGER::ERROR("Error in sha1 calculation!");
        return "$";
    }

    bin2hex( digest, sizeof(digest), str );

    fileSHA1 = str;

    return fileSHA1;
}

string calHashofchunk
(char *schunk, int length1, int shorthashflag)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    char buf[SHA_DIGEST_LENGTH * 2];
    SHA1((unsigned char *)schunk, length1, hash);

    // printf("\n*****hash ********");
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf((char *)&(buf[i * 2]), "%02x", hash[i]);

    // cout<<"hash : "<<buf<<endl;
    string ans;
    if (shorthashflag == 1)
    {
        for (int i = 0; i < 20; i++)
        {
            ans += buf[i];
        }
    }
    else
    {
        for (int i = 0; i < SHA_DIGEST_LENGTH * 2; i++)
        {
            ans += buf[i];
        }
    }
    return ans;
}

int saveAllChunkSHA
(int client_sd,char* filePath,string fileName){

    ifstream file1(filePath, ifstream::binary);

    /* basic sanity check */
    if (!file1)
    {
        LOGGER::ERROR("FILE DOES NOT EXITST : %s", filePath);
        return -1;
    }

    struct stat fstatus;
    stat(filePath, &fstatus);

    // Logic for deviding file1 into chunks
    long int total_size = fstatus.st_size;
    long int chunk_size = CHUNKSIZE;

    int total_chunks = total_size / chunk_size;
    int last_chunk_size = total_size % chunk_size;

    if (last_chunk_size != 0) // if file1 is not exactly divisible by chunks size
    {
        ++total_chunks; // add last chunk to count
    }
    
    else // when file1 is completely divisible by chunk size
    {
        last_chunk_size = chunk_size;
    }

    cout << "\ntotal chunks : " << total_chunks << "\n";

    // loop to getting each chunk
    vector<string> allChunkSHA;
    for (int chunk = 0; chunk < total_chunks; ++chunk)
    {
        int cur_cnk_size;
        if (chunk == total_chunks - 1)
            cur_cnk_size = last_chunk_size;
        else
            cur_cnk_size = chunk_size;

        char *chunk_data = new char[cur_cnk_size];
        file1.read(chunk_data,    /* address of buffer start */
                   cur_cnk_size); /* this many bytes is to be read */

        string sh1out = calHashofchunk(chunk_data, cur_cnk_size, 1);

        cout << "\nchunk " << (chunk) << "=> " << sh1out << "\n";
        allChunkSHA.push_back(sh1out);   
    }

    // store into fileinfo
    // take lock
    sem_wait(&fileInfoMap_WLOCK);

    // writing
    FileInfo *fileInfo = fileInfoMap[fileName];
    fileInfo->addAllChunkSHA(allChunkSHA);

    // release lock
    sem_post(&fileInfoMap_WLOCK);

}

void serveUploadFile
(int client_sd, string command){

    LOGGER::DEBUG("serving upload_file request");
    
    // [upload_file, filePath, groupid]
    vector<string> info = parseString(command,' ');
    string filePath = info[1];

    char buf[MAX_BUFFER_SIZE];

    char *res = realpath(filePath.c_str(), buf);
    if (!res){
        LOGGER::ERROR("no such file exists at this path");
        return;
    }

    filePath = res;
    string fileName = filePath.substr(filePath.find_last_of("/")+1,filePath.size());

    if(isFileExists(fileName)){
        LOGGER::WARN("File already present and made shareable");
        return ;
    }

    string SHA1 = getSHA1OfCompleteFile(res);
    
    if(SHA1 == "$"){
        LOGGER::ERROR("error in calculating SHA1");
    }

    struct stat sb { };

    if (stat(filePath.c_str(), &sb)){
        LOGGER::ERROR("error in getting file size");
        return;
    }

    string fileSize = to_string(sb.st_size);
    LOGGER::INFO("filesize %s bytes",fileSize.c_str());
    //[upload_file, fileName, groupid, fileSize, SHA1]
    string fileMetaData = info[0] + " " + fileName + " " + 
            info[2] + " " + fileSize + " " + SHA1;

    LOGGER::INFO("\nfile metadata\n %s", fileMetaData.c_str());

    // send command information to tracker
    send(client_sd, fileMetaData.c_str(), fileMetaData.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    vector<string> resp = parseString(response,'|');

    // if success returned means new file added.
    if(resp[0] == "true"){
        // add new file
        // if new seeder added or new file added
        addNewFile(fileName,fileSize,SHA1,filePath,true);
        // send chunk wise sha to tracker
        int status = saveAllChunkSHA(client_sd, res,fileName);
    }

    logTrackerResponse(response);

    LOGGER::DEBUG("serving upload_file completed");

}

void serveListAllFiles
(int client_sd, string command){

    LOGGER::DEBUG("serving list all files request");

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    vector<string> res = parseString(response,'|');

    logTrackerResponse(response);
    cout << endl;

    if(res[0] == "true") { 
        cout << endl << "list of all shareable files :" << endl;
        for(int i = 2;i < res.size(); i++){
            cout << res[i] << endl;
        }
    }

    LOGGER::DEBUG("serving list all files completed");

}

void serveLogout(int client_sd, string command){

    LOGGER::DEBUG("serving logout request started.. for %s", userIdOfLoggedInUser.c_str());

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};
    // read the response from tracker
    read(client_sd, response, sizeof(response));

    vector<string> res = parseString(response,'|');

    logTrackerResponse(response);

    if(res[0] == "true") { 
        isUserLoggedIn = false;
        userIdOfLoggedInUser = "";
    }

    LOGGER::DEBUG("serving logout completed for %s", userIdOfLoggedInUser.c_str());

}

void serveLeaveGroup(int client_sd, string command){

    LOGGER::DEBUG("serving leave_group ...");

    // send command information to tracker
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};

    // read the response from tracker
    read(client_sd, response, sizeof(response));

    logTrackerResponse(response);

    LOGGER::DEBUG("serving leave_group completed");

}

void serveStopShare(int client_sd, string command){

    LOGGER::DEBUG("serving stop_share ...");

    // send command information to tracker
    // [stop_share, groupid, filename]
    send(client_sd, command.c_str(), command.size(),0);

    char response[MAX_BUFFER_SIZE] = {0};

    // read the response from tracker
    read(client_sd, response, sizeof(response));

    vector<string> resp = parseString(response,'|');

    LOGGER::INFO("response recieved");

    if(resp[0] == "true"){
        string fileName = parseString(command,' ')[2];
        // remove the key from fileInfo map
        removeFileFromFileInfoMap(fileName);
    }

    logTrackerResponse(response);

    LOGGER::DEBUG("serving stop_share completed");

}
