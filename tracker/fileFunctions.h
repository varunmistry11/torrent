void addNewUserFile
(string userid, string password){

    ofstream ofile("clientInfo.txt", ios::app);

    if(!ofile.is_open()){
        return ;
    }

    // userid|pass|ip|port|true/false
    vector<string> data{userid,password,"$","$","false"};
    string info = getString(data);

    ofile << info << "\n";

    ofile.close();

}

void updateClientInfoFile(){

    // flush the updated map into file
    ofstream outfile("clientInfo.txt",ios::trunc);

    if(!outfile.is_open()){
        return ;
    }

    // format of each line: userid|password|ip|port|isloggedIn
    for(auto client : clientInfoMap){
        
        ClientInfo *clientInfo = client.second;
        vector<string> info{
            clientInfo->getUserId(),
            clientInfo->getPassword(),
            clientInfo->getIP(),
            (clientInfo->getPort() != -1 ? to_string(clientInfo->getPort()) : "$"),
            clientInfo->getIsLoggedIn() ? "true" : "false"
        };

        string data = getString(info);

        outfile << data << "\n";
    }

    outfile.close();

}

void reloadClientInfoFromFile(){

    ifstream infile("clientInfo.txt");

    if(!infile.is_open()){
        return ;
    }

    string line;
    
    while (getline(infile, line)){
        
        vector<string> info;
        string userid;
        string password;
        info = parseString(line,'|');
        if(info.size() != 5)
            break;
        userid = info[0];
        password = info[1];

        ClientInfo *client = new ClientInfo(userid,password);
        clientInfoMap[userid] = client;
    }

    infile.close();

    updateClientInfoFile();

}

void updateGroupInfoFile(){

    ofstream outfile("groupInfo.txt", ios::trunc);

    if(!outfile.is_open()){
        return ;
    }

    LOGGER::INFO("update groupinfo files");

    for(auto grp : groupInfoMap){
        
        Group *group = grp.second;
        
        // first line is groupid
        outfile << group->getGroupId() << endl;
        
        // second line is owner-user-id
        outfile << group->getOwnerUserId() << endl;

        // third line is member-userid's
        vector<string> memberList = group->getMemberUserIds();
        string members = getString(memberList);
        if(members.size() != 0)
            outfile << members << "\n";
        else
            outfile << "|" << "\n";

        // fourth line is pending-request-userid's
        vector<string> pendingRequests = group->getListOfPendingRequest();
        string pendinglist = getString(pendingRequests);
        if(pendinglist.size() != 0)
            outfile << pendinglist << "\n";
        else
            outfile << "|" << "\n";

        // fifth line is list-of-filenames
        vector<string> fileNames = group->getFileNames();
        string files = getString(fileNames);
        if(files.size() != 0)
            outfile << files << "\n";
        else
            outfile << "|" << "\n";

    }

    outfile.close();

    LOGGER::INFO("update groupinfo files completed");
}

void reloadGroupInfoFromFile(){

    ifstream infile("groupInfo.txt");

    if(!infile.is_open()){
        return ;
    }

    string line;
    
    while (getline(infile, line)){
        Group *group = new Group();
        // first line is groupid
        string groupid = line;
        group->setGroupId(groupid);

        // second line is owner-user-id
        getline(infile,line);
        string ownerid = line;
        group->setOwnerUserId(ownerid);

        // third line is member-userid's
        getline(infile, line);
        vector<string> memberList = parseString(line,'|');
        for(string userid : memberList)
            group->addNewMember(userid);
        
        // fourth line is pending-request-userid's
        getline(infile, line);
        vector<string> pendingRequests = parseString(line, '|');
        for(string userid : pendingRequests)
            group->addNewRequest(userid);

        // fifth line is list-of-filenames
        getline(infile, line);
        vector<string> fileNames = parseString(line, '|');
        for(string fileName : fileNames)
            group->addFileName(fileName);
        
        groupInfoMap[groupid] = group;

    }

    infile.close();
}

void updateFileInfoFile(){

    ofstream outfile("fileInfo.txt", ios::trunc);

    if(!outfile.is_open()){
        return ;
    }

    for(auto file : fileInfoMap){
        
        FileInfo *fileInfo = file.second;
        
        // first line is groupid|filename -> key
        outfile << file.first << endl;
        
        // second line is fileName
        outfile << fileInfo->getFileName() << endl;

        // third line is fileSize
        outfile << fileInfo->getFileSize() << endl;

        // fourth line is fileSHA
        outfile << fileInfo->getSHA_1() << endl;

        // fifth line is seederList
        vector<string> seederList = fileInfo->getSeederList();
        string seeders = getString(seederList);
        if(seeders.size() != 0)
            outfile << seeders << "\n";
        else
            outfile << "|" << "\n";

        // fifth line is leecherList
        vector<string> leecherList = fileInfo->getLeecherList();
        string leechers = getString(leecherList);
        if(leechers.size() != 0)
            outfile << leechers << "\n";
        else
            outfile << "|" << "\n";
        
    }

    outfile.close();

}

void reloadFileInfoFromFile(){

    ifstream infile("fileInfo.txt");

    if(!infile.is_open()){
        return ;
    }

    string line;
    
    while (getline(infile, line)){
        
        FileInfo *fileInfo = new FileInfo();

        // first line is groupid|filename -key of map
        string key = line;

        // second line is fileName
        getline(infile,line);
        string fileName = line;
        fileInfo->setFileName(fileName);

        // third line is fileSize
        getline(infile, line);
        long long fileSize = atoll(line.c_str());
        fileInfo->setfileSize(fileSize);
        
        // fourth line is SHA1
        getline(infile, line);
        string SHA1 = line;
        fileInfo->setSHA_1(SHA1);

        // fifth line is seederList
        getline(infile, line);
        vector<string> seederList = parseString(line, '|');
        fileInfo->addSeederList(seederList);
        
        // fifth line is leecherList
        getline(infile, line);
        vector<string> leecherList = parseString(line, '|');
        fileInfo->addLeecherList(leecherList);

        fileInfoMap[key] = fileInfo;

    }

    infile.close();



}