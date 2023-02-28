#!/bin/sh
echo "Starting Testing"


cat resource/malformed-req.txt | netcat 127.0.0.1 12345  > actual/malformed-res.txt

#testing the header code
diff --brief <(head -n 1 resource/malformed-res.txt) <(head -n 1 actual/malformed-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "1) Handle malformed request gracefully - FAILED"
else
    echo "1) Handle malformed response gracefully - PASSED"
fi

cat resource/http-get-req-maxage-bad-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-maxage-bad-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-maxage-bad-res.txt) <(head -n 1 actual/http-get-req-maxage-bad-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "2) Handle malformed 'maxage' tag request gracefully - FAILED"
else
    echo "2) Handle malformed 'maxage' tag response gracefully - PASSED"
fi

cat resource/http-get-req-maxstale-bad-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-maxstale-bad-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-maxstale-bad-res.txt) <(head -n 1 actual/http-get-req-maxstale-bad-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "3) Handle malformed 'maxstale' tag request gracefully - FAILED"
else
    echo "3) Handle malformed 'maxstale' tag response gracefully - PASSED"
fi

cat resource/http-get-req-minfresh-bad-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-minfresh-bad-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-minfresh-bad-res.txt) <(head -n 1 actual/http-get-req-minfresh-bad-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "4) Handle malformed 'minfresh' tag request gracefully - FAILED"
else
    echo "4) Handle malformed 'minfresh' tag response gracefully - PASSED"
fi


cat resource/http-connect-req.txt | netcat 127.0.0.1 12345  > actual/http-connect-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-connect-res.txt) <(head -n 1 actual/http-connect-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "5) Http Connect Request - FAILED"
else
    echo "5) Http Connect Request - PASSED"
fi

cat resource/http-get-404-notfound-req.txt | netcat 127.0.0.1 12345  > actual/http-get-404-notfound-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-404-notfound-res.txt) <(head -n 1 actual/http-get-404-notfound-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "6) Handle 404 not found gracefully - FAILED"
else
    echo "6) Handle 404 not found gracefully - PASSED"
fi


cat resource/http-get-bad-request-req.txt | netcat 127.0.0.1 12345  > actual/http-get-bad-request-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-bad-request-res.txt) <(head -n 1 actual/http-get-bad-request-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "7) Handle 400 bad request gracefully - FAILED"
else
    echo "7) Handle 400 bad request gracefully - PASSED"
fi


cat resource/http-get-internal-server-error-req.txt | netcat 127.0.0.1 12345  > actual/http-get-internal-server-error-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-internal-server-error-res.txt) <(head -n 1 actual/http-get-internal-server-error-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "8) Handle internal server error gracefully - FAILED"
else
    echo "8) Handle internal server error gracefully - PASSED"
fi

cat resource/http-post-req.txt | netcat 127.0.0.1 12345  > actual/http-post-res.txt

#testing the header code
#diff --brief <(head -n 1 resource/http-post-res.txt) <(head -n 1 actual/http-post-res.txt) >/dev/null
#comp_value=$?

#if [ $comp_value -eq 1 ]
#then
#    echo "POST request - FAILED"
#else
#    echo "POST request - PASSED"
#fi


cat resource/http-get-chunked-req.txt | netcat 127.0.0.1 12345  > actual/http-get-chunked-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-chunked-res.txt) <(head -n 1 actual/http-get-chunked-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "9) Get chunked request - FAILED"
else
    echo "9) Get chunked request - PASSED"
fi

cat resource/http-get-nocache-info-req.txt | netcat 127.0.0.1 12345  > actual/http-get-nocache-info-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-nocache-info-res.txt) <(head -n 1 actual/http-get-nocache-info-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "10) Get request info with no cache info - FAILED"
else
    echo "10) Get request info with no cache info - PASSED"
fi

cat resource/http-get-no-cache-req.txt | netcat 127.0.0.1 12345  > actual/http-get-no-cache-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-no-cache-res.txt) <(head -n 1 actual/http-get-no-cache-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "11) Get request with 'no-cache' tag - FAILED"
else
    echo "11) Get request with 'no-cache' tag - PASSED"
fi

cat resource/http-get-no-store-req.txt | netcat 127.0.0.1 12345  > actual/http-get-no-store-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-no-store-res.txt) <(head -n 1 actual/http-get-no-store-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "12) Get request with 'no-store' tag - FAILED"
else
    echo "12) Get request with 'no-store' tag - PASSED"
fi

cat resource/http-get-private-req.txt | netcat 127.0.0.1 12345  > actual/http-get-private-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-private-res.txt) <(head -n 1 actual/http-get-private-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "13) Get request with 'private' tag - FAILED"
else
    echo "13) Get request with 'private' tag - PASSED"
fi

cat resource/http-get-must-revalidate-withoutage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-must-revalidate-withoutage-res.txt


#testing the header code
#diff --brief <(head -n 1 resource/http-get-must-revalidate-withoutage-res.txt) <(head -n 1 actual/http-get-must-revalidate-withoutage-res.txt) >/dev/null
#comp_value=$?

#if [ $comp_value -eq 1 ]
#then
#    echo "Http Get must-revalidate-withoutage did not match with the stored value"
#else
#    echo "Http Get must-revalidate-withoutage Request Passed"
#fi


cat resource/http-get-maxage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-res.txt) <(head -n 1 actual/http-get-maxage-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "14) Get request with 'maxage' tag - FAILED"
else
    echo "14) Get request with 'maxage' tag - PASSED"
fi


cat resource/http-get-maxage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-res.txt) <(head -n 1 actual/http-get-maxage-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "15) Get cached request from the proxy server before expiration time - FAILED"
else
    echo "15) Get cached request from the proxy server before expiration time - PASSED"
fi

sleep 5

cat resource/http-get-maxage-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-res.txt) <(head -n 1 actual/http-get-maxage-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "16) Get cached request from the actual server after expiration time - FAILED"
else
    echo "16) Get cached request form the actual server after expiration time  - PASSED"
fi



cat resource/http-get-maxage-must-revalidate-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-must-revalidate-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-must-revalidate-res.txt) <(head -n 1 actual/http-get-maxage-must-revalidate-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "17) Get request with 'maxage' 'must-revalidate' tag - FAILED"
else
    echo "17) Get request with 'maxage' 'must-revalidate' tag - PASSED"
fi


cat resource/http-get-maxage-must-revalidate-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-must-revalidate-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-must-revalidate-res.txt) <(head -n 1 actual/http-get-maxage-must-revalidate-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "18) Get cached request from the proxy server before expiration time - FAILED"
else
    echo "18) Get cached request from the proxy server before expiration time - PASSED"
fi

sleep 5

cat resource/http-get-maxage-must-revalidate-req.txt | netcat 127.0.0.1 12345  > actual/http-get-maxage-must-revalidate-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-maxage-must-revalidate-res.txt) <(head -n 1 actual/http-get-maxage-must-revalidate-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "19) Get cached request from the actual server after expiration time - FAILED"
else
    echo "19) Get cached request from the actual server after expiration time  - PASSED"
fi

sleep 5
# cache directive tests
cat resource/http-get-req-client-maxage-12-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-client-maxage-12-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-client-maxage-12-res.txt) <(head -n 1 actual/http-get-req-client-maxage-12-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "20) Get from server with max-age 10 - FAILED"
else
    echo "20) Get from server with max-age 10 - PASSED"
fi

cat resource/http-get-req-client-maxage-5-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-client-maxage-5-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-client-maxage-5-res.txt) <(head -n 1 actual/http-get-req-client-maxage-5-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "21) Get from cache with max-age of client used - FAILED"
else
    echo "21) Get from cache with max-age of client used - PASSED"
fi

cat resource/http-get-req-client-minfresh-2-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-client-minfresh-2-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-client-minfresh-2-res.txt) <(head -n 1 actual/http-get-req-client-minfresh-2-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "22) Get from cache with min-fresh still valid - FAILED"
else
    echo "22) Get from cache with min-fresh still valid - PASSED"
fi

sleep 5

cat resource/http-get-req-client-minfresh-7-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-client-minfresh-7-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-client-minfresh-7-res.txt) <(head -n 1 actual/http-get-req-client-minfresh-7-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "23) Get from cache with min-fresh not valid - FAILED"
else
    echo "23) Get from cache with min-fresh not valid - PASSED"
fi

sleep 5

cat resource/http-get-req-client-maxstale-5-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-client-maxstale-5-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-client-maxstale-5-res.txt) <(head -n 1 actual/http-get-req-client-maxstale-5-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "24) Get from cache expired but within max-stale - FAILED"
else
    echo "24) Get from cache expired but within max-stale - PASSED"
fi

sleep 10

cat resource/http-get-req-client-maxstale-5-req.txt | netcat 127.0.0.1 12345  > actual/http-get-req-client-maxstale-5-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-req-client-maxstale-5-res.txt) <(head -n 1 actual/http-get-req-client-maxstale-5-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "25) Get from cache expired and not within max-stale - FAILED"
else
    echo "25) Get from cache expired and not within max-stale - PASSED"
fi


echo "Finished Testing"
