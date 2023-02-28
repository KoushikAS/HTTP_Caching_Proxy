# erss-hwk2-ka266-rm358

# Proxy Server for ECE 568 project 2 

#Authors 
Koushik Annareddy Sreenath (ka266)
Ryan Mecca (rm358)

## Getting started
To run the application 

```
docker-compose up -d
```

## Testing
Ensure the proxy server is running and then type the command
```
cd tests/
bash test.sh
```

Check the logs to see if the request is handled correctly.

Note: We have found that POST http://httpbin.org/post sometimes is down. If you see post request did not match please ensure that this server is up and try again.


## Logging
To check the logs

```
cat logs/proxy.log
```

## Citations
1) https://www.boost.org/doc/libs/1_80_0/libs/beast/example/http/server/sync/http_server_sync.cpp
2) https://man7.org/linux/man-pages/man2/select.2.html                  
3) https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/      
