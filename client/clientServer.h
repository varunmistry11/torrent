
void* createServerThread(void* param){

    // [ip-address, port]
    string serverInfo = *(string *) param;

    vector<string> info = parseString(serverInfo,'|');

    int serverPort = atoi(info[1].c_str());
    string serverIPAddress = info[0];

    LOGGER::DEBUG("Server started on %s ip and %d port",
    serverIPAddress.c_str(),serverPort);

    int server_sd;

    // Creating socket file descriptor
    if ((server_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        LOGGER::ERROR("Tracker socket conneciton failed");
        exit(EXIT_FAILURE);
    }
    
    LOGGER::INFO("Server socket connection established");

    struct sockaddr_in serverAdd;

    // assign type of ip address ipv4 here
    serverAdd.sin_family = AF_INET;         
    // assign port 
    serverAdd.sin_port = htons(serverPort);
    // store this IP address in trackerAdd.sin_addr
    inet_pton(AF_INET, serverIPAddress.c_str(), &(serverAdd.sin_addr));


    // Forcefully attaching socket to the port trackerPort
    if (::bind(server_sd, (struct sockaddr *)&serverAdd, sizeof(serverAdd)) < 0)
    {
        LOGGER::ERROR("Server bind failed");
        exit(EXIT_FAILURE);
    } 
    LOGGER::INFO("Server binded on ip %s", serverIPAddress.c_str());

      // it can keep at max 500 connection requests at a time
    if (listen(server_sd, MAXREQUESTS) < 0){
        LOGGER::ERROR("Tracker is not able to listen");
        exit(EXIT_FAILURE);
    }
    LOGGER::INFO("Server listening on port %d", serverPort);

    socklen_t serverAddLen;
    while(1){

        LOGGER::INFO("accepting ");
        int peer_sd;
        if ((peer_sd = accept(server_sd, (struct sockaddr *)&serverAdd,
                            &serverAddLen)) < 0){
            LOGGER::ERROR("Server not able to accept more request on peer_sd %d",peer_sd);
            break;
        }

        LOGGER::INFO("Connection done on peer_sd %d", peer_sd);
        LOGGER::INFO("Connection established on ip: %s",getPeerIP(peer_sd).c_str());

        LOGGER::INFO("peer_sd %d",peer_sd);

        // read input from client and make diff thread
        // for each tasks. 
        string connection = "Connection established on ip " + serverIPAddress 
                + " port " + to_string(serverPort);
        write(STDOUT_FILENO,connection.c_str(),connection.size());

        // make new thread
        pthread_create(&threadIds[liveThreads++], NULL,
                    performPeerTasks,&peer_sd);
    }

    LOGGER::DEBUG("Server stopped");

}
