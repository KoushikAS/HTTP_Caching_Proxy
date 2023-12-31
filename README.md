# HTTP Caching Proxy

## Overview

This project involves developing an HTTP caching proxy in C++. The proxy forwards requests to the origin server on behalf of the client, caches responses, and serves cached copies of resources when appropriate. The proxy supports GET, POST, and CONNECT requests and is designed to handle HTTPS communications via the CONNECT method.

## Key Features

- **Caching Mechanism:** Caches 200-OK responses to GET requests and follows expiration/re-validation rules for serving requests from the cache.
- **Concurrency Handling:** Manages multiple concurrent requests effectively using multiple threads, with a shared and synchronized cache.
- **Logging:** Generates logs for each request in `/var/log/erss/proxy.log` with detailed information including request ID, time, IP address, and HTTP request/response lines.
- **Error Handling:** Robustly handles external failures, malformed requests, and corrupted responses, responding with appropriate HTTP error codes.

## Installation and Running

### Prerequisites
- Docker
  
### Installation
1. Clone the repository to your local machine:

```sh
git clone https://github.com/KoushikAS/Ride_Sharing_App.git
cd Ride_Sharing_App/docker-deploy
```

### Running with Docker
1. Use Docker to build and run the containers for both the backend and frontend services.

```
docker-compose up -d
```

## Testing
Ensure the proxy server is running and then type the command
```
cd tests/
bash test.sh
```

## Contributions

This project was completed as part of an academic assignment with requirments provided requirments.pdf. Contributions were made solely by Koushik Annareddy Sreenath, Ryan Mecca, adhering to the project guidelines and requirements set by the course ECE-568 Engineering Robust Server Software 

## License

This project is an academic assignment and is subject to university guidelines on academic integrity and software use.

## Acknowledgments

Thanks to Brian Rogers and the course staff for providing guidance and support throughout the project.
