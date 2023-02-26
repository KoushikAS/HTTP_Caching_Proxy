#!/bin/sh
echo "Starting Testing"
cat resource/http-get-req.txt | netcat 127.0.0.1 12345  > actual/http-get-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-res.txt) <(head -n 1 actual/http-get-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http Get Request did not match witht stored value"
else
    echo "Http Get Request Passed"
fi

cat resource/http-get-req.txt | netcat 127.0.0.1 12345  > actual/http-get-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-res.txt) <(head -n 1 actual/http-get-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http cached Get Request did not match witht stored value"
else
    echo "Http cached Get Request Passed"
fi

cat resource/http-get-chunked-req.txt | netcat 127.0.0.1 12345  > actual/http-get-chunked-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-chunked-res.txt) <(head -n 1 actual/http-get-chunked-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http chunked Get Request did not match witht stored value"
else
    echo "Http chunked Get Request Passed"
fi


cat resource/http-get-chunked-req.txt | netcat 127.0.0.1 12345  > actual/http-get-chunked-res.txt

#testing the header code
diff --brief <(head -n 1 resource/http-get-chunked-res.txt) <(head -n 1 actual/http-get-chunked-res.txt) >/dev/null
comp_value=$?

if [ $comp_value -eq 1 ]
then
    echo "Http cached chunked Get Request did not match witht stored value"
else
    echo "Http cached chunked Get Request Passed"
fi

echo "Finished Testing"
