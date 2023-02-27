# erss-hwk2-ka266-rm358

# Proxy Server for ECE 568 project 2 

#Authors 
Koushik Annareddy Sreenath (ka266)
Ryan Mecca (rm-358)

## Getting started
To run the application 

```
docker-compose up -d
```

Note: logs are available at logs/proxy.log

##Testing
Ensure the proxy server is running and then type the command
```
cd tests/
bash test.sh
```

##Logging
To check the logs

```
cat logs/proxy.log
```

##Citations
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
2) https://man7.org/linux/man-pages/man2/select.2.html                  
3) https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/      
